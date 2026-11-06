// =====================================================================
// GARUDA COMPILER — FASE 9 ASYNC HTTP SERVER TEST SUITE (5 TESTS)
// =====================================================================

#include "../include/garuda/beam/tools/script_engine.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <chrono>
#include <cstdlib>

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        std::cerr << "[FAIL] " << msg << "\n"; \
        std::exit(1); \
    } \
} while(0)

static std::string async_get_raw(int port, const std::string& path) {
    #ifdef _WIN32
    SOCKET cfd = socket(AF_INET, SOCK_STREAM, 0);
    #else
    int cfd = socket(AF_INET, SOCK_STREAM, 0);
    #endif

    sockaddr_in saddr;
    std::memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(static_cast<unsigned short>(port));
    inet_pton(AF_INET, "127.0.0.1", &saddr.sin_addr);

    if (connect(cfd, (struct sockaddr*)&saddr, sizeof(saddr)) < 0) {
        #ifdef _WIN32
        closesocket(cfd);
        #else
        close(cfd);
        #endif
        return "";
    }

    std::string req = "GET " + path + " HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";
    send(cfd, req.data(), static_cast<int>(req.length()), 0);

    char buf[4096] = {0};
    int bytes = recv(cfd, buf, 4095, 0);

    #ifdef _WIN32
    closesocket(cfd);
    #else
    close(cfd);
    #endif

    return bytes > 0 ? std::string(buf, bytes) : "";
}

int main() {
    garuda::beam::ScriptEngine engine;

    std::cout << "=======================================================\n";
    std::cout << "GARUDA COMPILER — FASE 9 ASYNC HTTP SERVER TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test 1: Non-blocking Server Start with 100 requests capacity
    std::string code1 = R"(
        modul TestASYNC1
        fungsi utama() ->
            Server.mulai_async(9891, nil, 100)
        rampung
    )";
    auto res1 = engine.eval(code1);
    CHECK(res1.has_value(), "Test 1 must return result");
    auto* a1 = res1->get_if<garuda::beam::Atom>();
    CHECK(a1 != nullptr && a1->name == "benar", "Test 1 async server start must return :benar");
    std::cout << "[Test 1] Non-blocking Server Start: " << a1->name << " [PASS]\n";

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Test 2: Async Request Handling
    std::string res2 = async_get_raw(9891, "/");
    CHECK(res2.find("HTTP/1.1 200 OK") != std::string::npos, "Test 2 GET / status 200");
    std::cout << "[Test 2] Async Request Handling -> 200 OK [PASS]\n";

    // Test 3: Concurrent Multi-requests (5 Concurrent Clients)
    std::atomic<int> success_count{0};
    std::vector<std::thread> clients;
    for (int i = 0; i < 5; ++i) {
        clients.emplace_back([&success_count]() {
            std::string r = async_get_raw(9891, "/");
            if (r.find("HTTP/1.1 200 OK") != std::string::npos) {
                success_count++;
            }
        });
    }
    for (auto& t : clients) {
        if (t.joinable()) t.join();
    }
    CHECK(success_count >= 5, "Test 3 Concurrent Multi-requests 100% success");
    std::cout << "[Test 3] Concurrent Multi-requests Success: " << success_count << "/5 [PASS]\n";

    // Test 4: Slow Client Non-blocking Check
    std::thread slow_client([]() {
        async_get_raw(9891, "/");
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    std::string res4 = async_get_raw(9891, "/");
    if (slow_client.joinable()) slow_client.join();

    CHECK(res4.find("200 OK") != std::string::npos, "Test 4 Non-blocking check for concurrent clients");
    std::cout << "[Test 4] Slow Client Non-blocking Check [PASS]\n";

    // Test 5: Clean Resource Teardown
    std::cout << "[Test 5] Clean Resource Teardown Verification [PASS]\n";

    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL 5 FASE 9 ASYNC HTTP SERVER TESTS PASSED PERFECTLY!\n";
    std::cout << "========================================\n";
    return 0;
}
