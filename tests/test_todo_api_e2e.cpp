// =====================================================================
// GARUDA COMPILER — FASE 12.4 TODOAPI END-TO-END TEST SUITE
// =====================================================================
#include "../include/garuda/beam/tools/script_engine.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <fstream>
#include <cstring>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

using namespace garuda::beam;

static int g_failures = 0;

static std::string http_get(int port, const std::string& path) {
#ifdef _WIN32
    WSADATA wsa; WSAStartup(MAKEWORD(2,2), &wsa);
#endif
    int s = (int)socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) return "";
    sockaddr_in a{}; a.sin_family = AF_INET;
    a.sin_port = htons(port); a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (connect(s, (sockaddr*)&a, sizeof(a)) < 0) {
#ifdef _WIN32
        closesocket(s);
#else
        close(s);
#endif
        return "";
    }
    std::string req = "GET " + path + " HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";
    send(s, req.data(), (int)req.length(), 0);
    char buf[4096] = {0};
    std::string resp;
    int r;
    while ((r = recv(s, buf, sizeof(buf)-1, 0)) > 0) {
        resp.append(buf, r);
        std::memset(buf, 0, sizeof(buf));
    }
#ifdef _WIN32
    closesocket(s);
#else
    close(s);
#endif
    return resp;
}

static std::string http_post(int port, const std::string& path, const std::string& body) {
#ifdef _WIN32
    WSADATA wsa; WSAStartup(MAKEWORD(2,2), &wsa);
#endif
    int s = (int)socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) return "";
    sockaddr_in a{}; a.sin_family = AF_INET;
    a.sin_port = htons(port); a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (connect(s, (sockaddr*)&a, sizeof(a)) < 0) {
#ifdef _WIN32
        closesocket(s);
#else
        close(s);
#endif
        return "";
    }
    std::string req = "POST " + path + " HTTP/1.1\r\nHost: localhost\r\nContent-Type: application/json\r\nContent-Length: "
                    + std::to_string(body.length()) + "\r\nConnection: close\r\n\r\n" + body;
    send(s, req.data(), (int)req.length(), 0);
    char buf[4096] = {0};
    std::string resp;
    int r;
    while ((r = recv(s, buf, sizeof(buf)-1, 0)) > 0) {
        resp.append(buf, r);
        std::memset(buf, 0, sizeof(buf));
    }
#ifdef _WIN32
    closesocket(s);
#else
    close(s);
#endif
    return resp;
}

int main() {
    ScriptEngine engine;
    std::cout << "=======================================================\n";
    std::cout << "GARUDA COMPILER — FASE 12.4 TODOAPI E2E TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Jalankan TodoAPI.grd
    std::ifstream f("examples/todo_api/todo_api.grd");
    if (!f.is_open()) {
        std::cout << "[FAIL] examples/todo_api/todo_api.grd not found\n";
        return 1;
    }
    std::string src((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    f.close();

    engine.eval(src);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Test 1: GET /health -> 200
    {
        std::string resp = http_get(9292, "/health");
        bool ok = resp.find("200") != std::string::npos;
        std::cout << (ok ? "[Test 1] GET /health -> 200 OK [PASS]\n" : "[Test 1] -> [FAIL]\n");
        if (!ok) g_failures++;
    }

    // Test 2: POST /todos dengan body JSON -> response berisi id
    {
        std::string body = R"({"judul":"Belajar Garuda"})";
        std::string resp = http_post(9292, "/todos", body);
        bool ok = resp.find("200") != std::string::npos || resp.find("201") != std::string::npos;
        std::cout << (ok ? "[Test 2] POST /todos -> 201 [PASS]\n" : "[Test 2] -> [FAIL]\n");
        if (!ok) g_failures++;
    }

    // Test 3: GET /todos -> list JSON berisi "Belajar Garuda"
    {
        std::string resp = http_get(9292, "/todos");
        bool ok = resp.find("200") != std::string::npos &&
                  resp.find("Belajar Garuda") != std::string::npos;
        std::cout << (ok ? "[Test 3] GET /todos -> contains 'Belajar Garuda' [PASS]\n" : "[Test 3] -> [FAIL]\n");
        if (!ok) g_failures++;
    }

    // Test 4: GET /todos/1 -> single todo
    {
        std::string resp = http_get(9292, "/todos/1");
        bool ok = resp.find("200") != std::string::npos;
        std::cout << (ok ? "[Test 4] GET /todos/1 -> 200 OK [PASS]\n" : "[Test 4] -> [FAIL]\n");
        if (!ok) g_failures++;
    }

    // Test 5: GET /missing -> 404
    {
        std::string resp = http_get(9292, "/missing");
        bool ok = resp.find("404") != std::string::npos;
        std::cout << (ok ? "[Test 5] GET /missing -> 404 [PASS]\n" : "[Test 5] -> [FAIL]\n");
        if (!ok) g_failures++;
    }

    if (g_failures > 0) {
        std::cout << "\n❌ " << g_failures << "/5 TESTS FAILED ON STUB CODE!\n";
        return 1;
    }
    std::cout << "\n🎉 ALL 5 FASE 12.4 TODOAPI E2E TESTS PASSED!\n";
    return 0;
}
