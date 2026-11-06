// =====================================================================
// GARUDA COMPILER — FASE 10.5 REAL STATIC FILE SERVER E2E TEST SUITE (5 TESTS)
// =====================================================================

#include "../include/garuda/beam/tools/script_engine.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <cstdlib>

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        std::cerr << "[FAIL] " << msg << "\n"; \
        std::exit(1); \
    } \
} while(0)

static std::string real_static_get(int port, const std::string& path) {
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
        CLOSE_SOCKET(cfd);
        return "";
    }

    std::string req = "GET " + path + " HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";
    send(cfd, req.data(), static_cast<int>(req.length()), 0);

    char buf[4096] = {0};
    int bytes = recv(cfd, buf, 4095, 0);

    CLOSE_SOCKET(cfd);

    return bytes > 0 ? std::string(buf, bytes) : "";
}

int main() {
    garuda::beam::ScriptEngine engine;

    std::cout << "=======================================================\n";
    std::cout << "GARUDA COMPILER — FASE 10.5 REAL STATIC FILE SERVER E2E TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test 1: Start Real Static File Server on port 9093 serving 'site/'
    std::string code1 = R"(
        modul TestSTCE2E1
        fungsi utama() ->
            Server.mulai_async_statis(9093, "site/", 10)
        rampung
    )";
    auto res1 = engine.eval(code1);
    CHECK(res1.has_value(), "Test 1 must return result");
    auto* a1 = res1->get_if<garuda::beam::Atom>();
    CHECK(a1 != nullptr && a1->name == "benar", "Test 1 Server.mulai_async_statis return :benar");
    std::cout << "[Test 1] Real Static File Server Initialization: " << a1->name << " [PASS]\n";

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Test 2: Real Static HTML File Reading (GET /)
    std::string res2 = real_static_get(9093, "/");
    CHECK(res2.find("HTTP/1.1 200 OK") != std::string::npos, "Test 2 GET / status 200");
    CHECK(res2.find("Garuda BEAM") != std::string::npos || res2.find("<!DOCTYPE html>") != std::string::npos, "Test 2 GET / body match");
    std::cout << "[Test 2] Real Static HTML File Reading (site/index.html) [PASS]\n";

    // Test 3: Real Static MIME Detection (.svg -> image/svg+xml)
    std::string res3 = real_static_get(9093, "/assets/favicon.svg");
    CHECK(res3.find("Content-Type: image/svg+xml") != std::string::npos, "Test 3 SVG MIME match");
    std::cout << "[Test 3] Real Static MIME Detection (.svg -> image/svg+xml) [PASS]\n";

    // Test 4: Real Path Traversal Protection Rejection (/../etc/passwd -> 403 Forbidden)
    std::string res4 = real_static_get(9093, "/../etc/passwd");
    CHECK(res4.find("403 Forbidden") != std::string::npos, "Test 4 Path Traversal 403 match");
    std::cout << "[Test 4] Real Path Traversal Rejection (/../etc/passwd -> 403 Forbidden) [PASS]\n";

    // Test 5: Real File Not Found (GET /nonexistent.html -> 404 Not Found)
    std::string res5 = real_static_get(9093, "/nonexistent.html");
    CHECK(res5.find("404 Not Found") != std::string::npos, "Test 5 File Not Found 404 match");
    std::cout << "[Test 5] Real File Not Found (/nonexistent.html -> 404 Not Found) [PASS]\n";

    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL 5 FASE 10.5 REAL STATIC FILE SERVER E2E TESTS PASSED PERFECTLY!\n";
    std::cout << "========================================\n";
    return 0;
}
