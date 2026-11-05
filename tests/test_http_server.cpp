// =====================================================================
// GARUDA COMPILER — FASE 8 NATIVE HTTP SERVER TEST SUITE (5 TESTS)
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

static std::string http_get_raw(int port, const std::string& path) {
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
    std::cout << "=======================================================\n";
    std::cout << "GARUDA COMPILER — FASE 8 NATIVE HTTP SERVER TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test 1: Start Native Server on port 9981 and GET /
    std::thread srv_thread1([]() {
        garuda::beam::ScriptEngine srv_engine;
        std::string srv_code = R"(
            modul TestSRV1
            fungsi utama() ->
                Server.mulai(9981, nil, 1)
            rampung
        )";
        srv_engine.eval(srv_code);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::string res1 = http_get_raw(9981, "/");
    if (srv_thread1.joinable()) srv_thread1.join();

    CHECK(res1.find("HTTP/1.1 200 OK") != std::string::npos, "Test 1 GET / status 200");
    CHECK(res1.find("Halo dari Garuda!") != std::string::npos, "Test 1 GET / body match");
    std::cout << "[Test 1] GET / -> 200 OK: Halo dari Garuda! [PASS]\n";

    // Test 2: GET /health -> 200 OK JSON
    std::thread srv_thread2([]() {
        garuda::beam::ScriptEngine srv_engine;
        std::string srv_code = R"(
            modul TestSRV2
            fungsi utama() ->
                Server.mulai(9982, nil, 1)
            rampung
        )";
        srv_engine.eval(srv_code);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::string res2 = http_get_raw(9982, "/health");
    if (srv_thread2.joinable()) srv_thread2.join();

    CHECK(res2.find("HTTP/1.1 200 OK") != std::string::npos, "Test 2 GET /health status 200");
    CHECK(res2.find("{\"status\":\"ok\"}") != std::string::npos, "Test 2 GET /health body match");
    std::cout << "[Test 2] GET /health -> JSON: {\"status\":\"ok\"} [PASS]\n";

    // Test 3: POST /echo -> Mirrored Body
    std::thread srv_thread3([]() {
        garuda::beam::ScriptEngine srv_engine;
        std::string srv_code = R"(
            modul TestSRV3
            fungsi utama() ->
                Server.mulai(9983, nil, 1)
            rampung
        )";
        srv_engine.eval(srv_code);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    #ifdef _WIN32
    SOCKET cfd3 = socket(AF_INET, SOCK_STREAM, 0);
    #else
    int cfd3 = socket(AF_INET, SOCK_STREAM, 0);
    #endif
    sockaddr_in saddr3;
    std::memset(&saddr3, 0, sizeof(saddr3));
    saddr3.sin_family = AF_INET;
    saddr3.sin_port = htons(9983);
    inet_pton(AF_INET, "127.0.0.1", &saddr3.sin_addr);
    connect(cfd3, (struct sockaddr*)&saddr3, sizeof(saddr3));
    std::string req3 = "POST /echo HTTP/1.1\r\nContent-Type: text/plain\r\nContent-Length: 18\r\n\r\nGARUDA_NATIVE_HTTP";
    send(cfd3, req3.data(), static_cast<int>(req3.length()), 0);
    char buf3[1024] = {0};
    recv(cfd3, buf3, 1023, 0);
    #ifdef _WIN32
    closesocket(cfd3);
    #else
    close(cfd3);
    #endif
    if (srv_thread3.joinable()) srv_thread3.join();

    std::string res3(buf3);
    CHECK(res3.find("HTTP/1.1 200 OK") != std::string::npos, "Test 3 POST /echo status 200");
    CHECK(res3.find("GARUDA_NATIVE_HTTP") != std::string::npos, "Test 3 POST /echo body match");
    std::cout << "[Test 3] POST /echo -> Mirrored: GARUDA_NATIVE_HTTP [PASS]\n";

    // Test 4: GET /missing -> 404 Not Found
    std::thread srv_thread4([]() {
        garuda::beam::ScriptEngine srv_engine;
        std::string srv_code = R"(
            modul TestSRV4
            fungsi utama() ->
                Server.mulai(9984, nil, 1)
            rampung
        )";
        srv_engine.eval(srv_code);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::string res4 = http_get_raw(9984, "/missing");
    if (srv_thread4.joinable()) srv_thread4.join();

    CHECK(res4.find("HTTP/1.1 404 Not Found") != std::string::npos, "Test 4 GET /missing status 404");
    std::cout << "[Test 4] GET /missing -> 404 Not Found [PASS]\n";

    // Test 5: Header Check
    std::thread srv_thread5([]() {
        garuda::beam::ScriptEngine srv_engine;
        std::string srv_code = R"(
            modul TestSRV5
            fungsi utama() ->
                Server.mulai(9985, nil, 1)
            rampung
        )";
        srv_engine.eval(srv_code);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::string res5 = http_get_raw(9985, "/");
    if (srv_thread5.joinable()) srv_thread5.join();

    CHECK(res5.find("Content-Type:") != std::string::npos, "Test 5 must have Content-Type");
    CHECK(res5.find("Content-Length:") != std::string::npos, "Test 5 must have Content-Length");
    std::cout << "[Test 5] HTTP Headers Validation [PASS]\n";

    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL 5 FASE 8 NATIVE HTTP SERVER TESTS PASSED PERFECTLY!\n";
    std::cout << "========================================\n";
    return 0;
}
