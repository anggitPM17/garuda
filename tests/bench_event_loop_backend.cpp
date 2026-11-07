// =====================================================================
// GARUDA COMPILER — FASE 11.3 EVENT-LOOP BACKEND BENCHMARK
// =====================================================================

#include "../include/garuda/beam/net/event_loop.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <string>
#include <cstring>

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

static bool make_socketpair(socket_t& a, socket_t& b) {
    #ifdef _WIN32
    WSADATA wsa; WSAStartup(MAKEWORD(2,2), &wsa);
    #endif
    socket_t l = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1; setsockopt(l, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
    sockaddr_in ad{}; ad.sin_family = AF_INET;
    ad.sin_port = htons(0); ad.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    bind(l, (sockaddr*)&ad, sizeof(ad));
    listen(l, 1);
    socklen_t alen = sizeof(ad);
    getsockname(l, (sockaddr*)&ad, &alen);
    socket_t c = socket(AF_INET, SOCK_STREAM, 0);
    if (connect(c, (sockaddr*)&ad, sizeof(ad)) < 0) { CLOSE_SOCKET(l); CLOSE_SOCKET(c); return false; }
    socket_t s = accept(l, nullptr, nullptr);
    CLOSE_SOCKET(l);
    a = c; b = s;
    return s != INVALID_SOCKET_VAL;
}

int main(int argc, char** argv) {
    const int N_CONNS   = (argc > 1) ? std::atoi(argv[1]) : 1000;
    const int ROUNDS    = (argc > 2) ? std::atoi(argv[2]) : 3;

    auto backend = make_default_backend();
    std::cout << "backend=" << (backend ? backend->name() : "null")
              << " conns=" << N_CONNS << " rounds=" << ROUNDS << "\n";

    if (!backend) return 1;

    std::vector<socket_t> a_fds, b_fds;
    a_fds.reserve(N_CONNS); b_fds.reserve(N_CONNS);
    for (int i = 0; i < N_CONNS; ++i) {
        socket_t a = INVALID_SOCKET_VAL, b = INVALID_SOCKET_VAL;
        if (make_socketpair(a, b)) { a_fds.push_back(a); b_fds.push_back(b); }
    }
    std::cout << "created " << a_fds.size() << " socketpairs\n";

    for (auto fd : a_fds) backend->add((int)fd);

    double total_ms = 0.0;
    for (int r = 0; r < ROUNDS; ++r) {
        // Fire all
        for (auto fb : b_fds) send(fb, "z", 1, 0);

        // Drain via wait()
        int remaining = (int)a_fds.size();
        auto t0 = std::chrono::steady_clock::now();
        while (remaining > 0) {
            Event evs[512];
            int n = backend->wait(evs, 512, 100);
            if (n <= 0) break;
            for (int i = 0; i < n; ++i) {
                char buf[8];
                recv((socket_t)evs[i].fd, buf, sizeof(buf), 0);
                remaining--;
            }
        }
        auto t1 = std::chrono::steady_clock::now();
        double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        std::cout << "round " << r << ": " << ms << " ms, "
                  << (N_CONNS / (ms / 1000.0)) << " ops/s\n";
        total_ms += ms;
    }

    for (auto fd : a_fds) backend->remove((int)fd);
    for (size_t i = 0; i < a_fds.size(); ++i) { CLOSE_SOCKET(a_fds[i]); CLOSE_SOCKET(b_fds[i]); }

    std::cout << "avg: " << (total_ms / ROUNDS) << " ms per round\n";
    return 0;
}
