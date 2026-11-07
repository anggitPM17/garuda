// =====================================================================
// GARUDA COMPILER — FASE 11.3 EVENT-LOOP BACKEND TEST SUITE (RED -> GREEN)
// =====================================================================

#include "../include/garuda/beam/net/event_loop.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <cstring>
#include <cstdint>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
typedef SOCKET socket_t;
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
typedef int socket_t;
#define INVALID_SOCKET_VAL -1
#define CLOSE_SOCKET(s) close(s)
#endif

using namespace garuda::beam::net;

static int g_failures = 0;
static void CHECK(bool cond, const char* msg) {
    if (!cond) {
        std::cout << "[FAIL] " << msg << "\n";
        g_failures++;
    }
}

// ---------------------------------------------------------------
// Helper: create a connected socketpair (real fds) portable
// ---------------------------------------------------------------
static bool make_socketpair(socket_t& out_a, socket_t& out_b) {
#ifdef _WIN32
    static bool wsa_init_sp = false;
    if (!wsa_init_sp) {
        WSADATA wsa; WSAStartup(MAKEWORD(2,2), &wsa);
        wsa_init_sp = true;
    }
#endif
    socket_t listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener == INVALID_SOCKET_VAL) return false;

    int opt = 1;
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
    sockaddr_in a{}; a.sin_family = AF_INET;
    a.sin_port = htons(0); a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (bind(listener, (sockaddr*)&a, sizeof(a)) < 0) { CLOSE_SOCKET(listener); return false; }
    if (listen(listener, 1) < 0) { CLOSE_SOCKET(listener); return false; }
    socklen_t alen = sizeof(a);
    getsockname(listener, (sockaddr*)&a, &alen);

    socket_t client = socket(AF_INET, SOCK_STREAM, 0);
    if (client == INVALID_SOCKET_VAL) { CLOSE_SOCKET(listener); return false; }
    if (connect(client, (sockaddr*)&a, sizeof(a)) < 0) {
        CLOSE_SOCKET(client); CLOSE_SOCKET(listener); return false; }
    socket_t server = accept(listener, nullptr, nullptr);
    CLOSE_SOCKET(listener);
    if (server == INVALID_SOCKET_VAL) { CLOSE_SOCKET(client); return false; }

    out_a = client;
    out_b = server;
    return true;
}

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA COMPILER — FASE 11.3 EVENT-LOOP BACKEND SUITE\n";
    std::cout << "=======================================================\n\n";

    // ---------------------------------------------------------------
    // Test 1: make_default_backend() returns non-null with correct name
    // ---------------------------------------------------------------
    {
        auto backend = make_default_backend();
        CHECK(backend != nullptr, "Test 1 backend must be non-null");

        const char* name = backend ? backend->name() : "";
#if defined(__linux__)
        CHECK(std::strcmp(name, "epoll") == 0, "Test 1 Linux must select 'epoll'");
#elif defined(__APPLE__)
        CHECK(std::strcmp(name, "kqueue") == 0, "Test 1 macOS must select 'kqueue'");
#else
        CHECK(std::strcmp(name, "poll") == 0, "Test 1 fallback must be 'poll'");
#endif

        if (g_failures == 0)
            std::cout << "[Test 1] Default backend: " << name << " [PASS]\n";
    }

    // ---------------------------------------------------------------
    // Test 2: PollBackend explicit init/add/wait with real socketpair
    // ---------------------------------------------------------------
    {
        PollBackend pb;
        CHECK(pb.init(), "Test 2 init failed");

        socket_t a = INVALID_SOCKET_VAL, b = INVALID_SOCKET_VAL;
        CHECK(make_socketpair(a, b), "Test 2 socketpair creation");

        CHECK(pb.add((int)a), "Test 2 add failed");

        const char* msg = "PING";
#ifdef _WIN32
        send(b, msg, 4, 0);
#else
        send(b, msg, 4, 0);
#endif

        Event evs[4]{};
        int n = pb.wait(evs, 4, 1000);
        CHECK(n > 0, "Test 2 wait returned >0");
        CHECK(n > 0 && (evs[0].events & EV_READ), "Test 2 readable event");
        CHECK(n > 0 && evs[0].fd == (socket_t)a, "Test 2 correct fd");

        pb.remove((int)a);
        CLOSE_SOCKET(a);
        CLOSE_SOCKET(b);

        if (g_failures == 0)
            std::cout << "[Test 2] PollBackend init/add/wait real-socketpair [PASS]\n";
    }

    // ---------------------------------------------------------------
    // Test 3: EventLoop facade — 200 real conns via backend
    //         (proxy for the 1000-conn Fase 11 test — scaled for CI speed)
    // ---------------------------------------------------------------
    {
        EventLoop loop;
        std::atomic<int> ready{0};

        socket_t listener = socket(AF_INET, SOCK_STREAM, 0);
        CHECK(listener != INVALID_SOCKET_VAL, "Test 3 listener socket");
        int opt = 1;
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
#ifdef _WIN32
        u_long lm = 1; ioctlsocket(listener, FIONBIO, &lm);
#else
        fcntl(listener, F_SETFL, O_NONBLOCK);
#endif
        sockaddr_in a{}; a.sin_family = AF_INET;
        a.sin_port = htons(0); a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        bind(listener, (sockaddr*)&a, sizeof(a));
        listen(listener, 512);
        socklen_t alen = sizeof(a);
        getsockname(listener, (sockaddr*)&a, &alen);

        std::vector<socket_t> clients;
        clients.reserve(200);
        for (int i = 0; i < 200; ++i) {
            socket_t c = socket(AF_INET, SOCK_STREAM, 0);
            if (connect(c, (sockaddr*)&a, sizeof(a)) == 0) clients.push_back(c);
            else CLOSE_SOCKET(c);
        }
        CHECK((int)clients.size() == 200, "Test 3 all 200 clients connected");

        loop.add_socket(listener, [&](socket_t lfd) {
            socket_t s = accept(lfd, nullptr, nullptr);
            if (s == INVALID_SOCKET_VAL) return;
#ifdef _WIN32
            u_long m = 1; ioctlsocket(s, FIONBIO, &m);
#else
            fcntl(s, F_SETFL, O_NONBLOCK);
#endif
            loop.add_socket(s, [&](socket_t cfd) {
                char buf[16];
                int r = recv(cfd, buf, sizeof(buf), 0);
                if (r > 0) {
                    ready++;
                    loop.remove_socket(cfd);
                    CLOSE_SOCKET(cfd);
                } else if (r == 0) {
                    loop.remove_socket(cfd);
                    CLOSE_SOCKET(cfd);
                }
            });
        });

        loop.start_in_thread();

        for (auto c : clients) send(c, "x", 1, 0);

        auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
        while (ready.load() < 200 && std::chrono::steady_clock::now() < deadline) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }

        loop.stop();
        loop.remove_socket(listener);
        CLOSE_SOCKET(listener);
        for (auto c : clients) CLOSE_SOCKET(c);

        CHECK(ready.load() == 200, "Test 3 all 200 served");

        if (g_failures == 0)
            std::cout << "[Test 3] EventLoop facade 200 real conns via backend [PASS]\n";
    }

    // ---------------------------------------------------------------
    // Test 4: Thread-safe add/remove from a foreign thread using REAL
    //         socketpair fds (no fake fd). 100 add + 50 remove concurrently
    //         with the running loop.
    // ---------------------------------------------------------------
    {
        EventLoop loop;
        loop.start_in_thread();

        // Pre-create 100 real socketpairs
        std::vector<socket_t> fds_a, fds_b;
        fds_a.reserve(100); fds_b.reserve(100);
        for (int i = 0; i < 100; ++i) {
            socket_t x = INVALID_SOCKET_VAL, y = INVALID_SOCKET_VAL;
            if (make_socketpair(x, y)) { fds_a.push_back(x); fds_b.push_back(y); }
        }
        CHECK((int)fds_a.size() == 100, "Test 4 created 100 real socketpairs");

        std::atomic<int> ready{0};
        for (auto fd : fds_a) {
            loop.add_socket(fd, [&](socket_t cfd) {
                char buf[8];
                int r = recv(cfd, buf, sizeof(buf), 0);
                if (r > 0) ready++;
            });
        }

        // Fire readiness on 50 of them BEFORE removal
        for (int i = 0; i < 50; ++i) send(fds_b[i], "y", 1, 0);

        // Remove 50 from a foreign thread (concurrent)
        std::thread remover([&]() {
            for (int i = 50; i < 100; ++i) {
                loop.remove_socket(fds_a[i]);
            }
        });
        remover.join();

        auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
        while (ready.load() < 50 && std::chrono::steady_clock::now() < deadline) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }

        loop.stop();

        // Cleanup all remaining
        for (size_t i = 0; i < fds_a.size(); ++i) {
            loop.remove_socket(fds_a[i]);   // idempotent for already-removed
            CLOSE_SOCKET(fds_a[i]);
            CLOSE_SOCKET(fds_b[i]);
        }

        CHECK(ready.load() >= 50, "Test 4 at least 50 sockets served before removal");
        if (g_failures == 0)
            std::cout << "[Test 4] Thread-safe add/remove with real socketpair fds [PASS]\n";
    }

    // ---------------------------------------------------------------
    // Test 5: Cumulative regression gate (Fase 11 backend still works)
    //         Prior failures MUST cascade here.
    // ---------------------------------------------------------------
    {
        bool all_prior_pass = (g_failures == 0);
        CHECK(all_prior_pass, "Test 5 cumulative: prior tests must all pass");

        if (all_prior_pass) {
            // Sanity: EventLoop still has backend_name()
            EventLoop l;
            CHECK(l.backend_name() != nullptr && std::strlen(l.backend_name()) > 0,
                  "Test 5 backend_name non-empty");
            std::cout << "[Test 5] Cumulative gate: all backend tests PASS "
                         "(backend=" << l.backend_name() << ") [PASS]\n";
        } else {
            std::cout << "[Test 5] Cumulative gate FAILED (prior tests failed)\n";
        }
    }

    if (g_failures > 0) {
        std::cout << "\n❌ " << g_failures << "/5 TESTS FAILED ON STUB CODE!\n";
        return 1;
    }
    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL 5 FASE 11.3 EVENT-LOOP BACKEND TESTS PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
