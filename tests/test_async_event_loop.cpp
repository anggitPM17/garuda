// =====================================================================
// GARUDA COMPILER — FASE 11 ASYNC EVENT-LOOP (EPOLL/IOCP/KQUEUE) TEST SUITE (RED -> GREEN)
// =====================================================================

#include "../include/garuda/beam/tools/script_engine.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <atomic>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#endif

static size_t get_process_memory_mb() {
    #ifdef _WIN32
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        return pmc.WorkingSetSize / (1024 * 1024);
    }
    #endif
    return 0;
}

static socket_t connect_tcp_client(int port) {
    #ifdef _WIN32
    socket_t cfd = socket(AF_INET, SOCK_STREAM, 0);
    #else
    int cfd = socket(AF_INET, SOCK_STREAM, 0);
    #endif

    sockaddr_in saddr;
    std::memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(static_cast<unsigned short>(port));
    inet_pton(AF_INET, "127.0.0.1", &saddr.sin_addr);

    if (connect(cfd, (struct sockaddr*)&saddr, sizeof(saddr)) < 0) {
        CLOSE_SOCKET(cfd);
        return INVALID_SOCKET_VAL;
    }
    return cfd;
}

int main() {
    garuda::beam::ScriptEngine engine;
    int failures = 0;

    std::cout << "=======================================================\n";
    std::cout << "GARUDA COMPILER — FASE 11 ASYNC EVENT-LOOP TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    size_t initial_mem = get_process_memory_mb();

    // Test 1: Start Async Event-Loop HTTP Server on port 9095
    auto ret = engine.eval("modul TEventLoop fungsi utama() -> Server.mulai_async_event_loop(9095, nil, 1000) rampung");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    if (ret.has_value() && garuda::beam::is_atom(*ret, "benar")) {
        std::cout << "[Test 1] Server.mulai_async_event_loop Port 9095 Start [PASS]\n";
    } else {
        std::cout << "[Test 1] Server.mulai_async_event_loop Port 9095 -> [FAIL] Function or EventLoop unavailable\n";
        failures++;
    }

    // Test 2: Open 1000 concurrent TCP sockets simultaneously to port 9095
    const int NUM_CONNS = 1000;
    std::vector<socket_t> conns(NUM_CONNS, INVALID_SOCKET_VAL);
    std::atomic<int> success_conns{0};

    std::vector<std::thread> threads;
    for (int i = 0; i < NUM_CONNS; ++i) {
        threads.emplace_back([i, &conns, &success_conns]() {
            socket_t fd = connect_tcp_client(9095);
            if (fd != INVALID_SOCKET_VAL) {
                conns[i] = fd;
                std::string req = "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: keep-alive\r\n\r\n";
                send(fd, req.data(), static_cast<int>(req.length()), 0);
                char buf[256] = {0};
                int r = recv(fd, buf, 255, 0);
                if (r > 0 && std::string(buf, r).find("200 OK") != std::string::npos) {
                    success_conns++;
                }
            }
        });
    }

    for (auto& t : threads) {
        if (t.joinable()) t.join();
    }

    if (success_conns.load() == NUM_CONNS) {
        std::cout << "[Test 2] 1000 Simultaneous Connections Handled -> 200 OK [PASS]\n";
    } else {
        std::cout << "[Test 2] 1000 Simultaneous Connections -> [FAIL] Handled " << success_conns.load() << "/" << NUM_CONNS << "\n";
        failures++;
    }

    // Test 3: Assert server remains responsive while 1000 connections are held open
    socket_t probe_fd = connect_tcp_client(9095);
    bool probe_ok = false;
    if (probe_fd != INVALID_SOCKET_VAL) {
        std::string req = "GET /health HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";
        send(probe_fd, req.data(), static_cast<int>(req.length()), 0);
        char buf[256] = {0};
        int r = recv(probe_fd, buf, 255, 0);
        CLOSE_SOCKET(probe_fd);
        if (r > 0 && std::string(buf, r).find("200 OK") != std::string::npos) {
            probe_ok = true;
        }
    }

    if (probe_ok) {
        std::cout << "[Test 3] Server Responsiveness under 1000 Open Connections [PASS]\n";
    } else {
        std::cout << "[Test 3] Server Responsiveness -> [FAIL] Server timed out or unresponding\n";
        failures++;
    }

    // Test 4: Gracefully close all 1000 connections
    for (int i = 0; i < NUM_CONNS; ++i) {
        if (conns[i] != INVALID_SOCKET_VAL) {
            CLOSE_SOCKET(conns[i]);
        }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    socket_t post_fd = connect_tcp_client(9095);
    bool post_ok = false;
    if (post_fd != INVALID_SOCKET_VAL) {
        std::string req = "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";
        send(post_fd, req.data(), static_cast<int>(req.length()), 0);
        char buf[256] = {0};
        int r = recv(post_fd, buf, 255, 0);
        CLOSE_SOCKET(post_fd);
        if (r > 0 && std::string(buf, r).find("200 OK") != std::string::npos) {
            post_ok = true;
        }
    }

    if (post_ok) {
        std::cout << "[Test 4] Server Health after 1000 Teardowns [PASS]\n";
    } else {
        std::cout << "[Test 4] Server Health -> [FAIL] Server dead after teardown\n";
        failures++;
    }

    // Test 5: Verify process memory consumption (assert delta < 50MB)
    size_t final_mem = get_process_memory_mb();
    size_t mem_delta = final_mem > initial_mem ? final_mem - initial_mem : 0;
    if (failures == 0 && mem_delta < 50) {
        std::cout << "[Test 5] Memory Leakage Check (Delta: " << mem_delta << " MB < 50 MB) [PASS]\n";
    } else {
        std::cout << "[Test 5] Memory Leakage Check -> [FAIL] Server not operational or mem delta: " << mem_delta << " MB\n";
        failures++;
    }

    if (failures > 0) {
        std::cout << "\n❌ " << failures << "/5 TESTS FAILED ON STUB CODE!\n";
        return 1;
    }

    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL 5 FASE 11 ASYNC EVENT-LOOP TESTS PASSED PERFECTLY!\n";
    std::cout << "========================================\n";
    return 0;
}
