// =====================================================================
// GARUDA COMPILER — FASE 10.7 HTTPS NO PLAINTEXT FALLBACK TEST SUITE (RED -> GREEN)
// =====================================================================

#include "../include/garuda/beam/tools/script_engine.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <cstdlib>

static std::string run_curl_plain(const std::string& cmd) {
    std::string full_cmd = "C:\\Windows\\System32\\curl.exe " + cmd + " 2>&1";
    #ifdef _WIN32
    FILE* pipe = _popen(full_cmd.c_str(), "r");
    #else
    FILE* pipe = popen(cmd.c_str(), "r");
    #endif

    if (!pipe) return "";
    char buffer[128];
    std::string result = "";
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    #ifdef _WIN32
    _pclose(pipe);
    #else
    pclose(pipe);
    #endif
    return result;
}

static std::string send_plain_http_request(int port) {
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

    std::string req = "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";
    send(cfd, req.data(), static_cast<int>(req.length()), 0);

    char buf[2048] = {0};
    int bytes = recv(cfd, buf, 2047, 0);
    CLOSE_SOCKET(cfd);

    return bytes > 0 ? std::string(buf, bytes) : "";
}

int main() {
    garuda::beam::ScriptEngine engine;
    int failures = 0;

    std::cout << "=======================================================\n";
    std::cout << "GARUDA COMPILER — FASE 10.7 HTTPS NO PLAINTEXT TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Start HTTPS Server on port 8445
    engine.eval("modul TNoPlain fungsi utama() -> Server.mulai_https(8445, \"\", \"\", nil, 50) rampung");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Test 1: Plaintext HTTP GET request to HTTPS port must NOT return "HTTP/1.1 200 OK"
    std::string res1 = send_plain_http_request(8445);
    if (res1.find("HTTP/1.1 200 OK") == std::string::npos && res1.find("HTTPS OK") == std::string::npos) {
        std::cout << "[Test 1] Plaintext HTTP to HTTPS Port Rejected (No 200 OK) [PASS]\n";
    } else {
        std::cout << "[Test 1] Plaintext HTTP to HTTPS Port -> [FAIL] Server responded with plaintext 200 OK fallback!\n";
        failures++;
    }

    // Test 2: Real TLS curl -k https://127.0.0.1:8445/
    std::string res2 = run_curl_plain("-k -s https://127.0.0.1:8445/");
    if (res2.find("HTTPS OK") != std::string::npos) {
        std::cout << "[Test 2] Real TLS curl -k https -> 'HTTPS OK' [PASS]\n";
    } else {
        std::cout << "[Test 2] Real TLS curl -k https -> [FAIL] Expected 'HTTPS OK', got: " << res2 << "\n";
        failures++;
    }

    // Test 3: Plaintext curl http://127.0.0.1:8445/ (WITHOUT TLS) must fail or not return HTTPS OK
    std::string res3 = run_curl_plain("-s http://127.0.0.1:8445/");
    if (res3.find("HTTPS OK") == std::string::npos && res3.find("200 OK") == std::string::npos) {
        std::cout << "[Test 3] Plaintext curl http:// to HTTPS port Rejected [PASS]\n";
    } else {
        std::cout << "[Test 3] Plaintext curl http:// -> [FAIL] Server served plaintext 200 OK!\n";
        failures++;
    }

    // Test 4: Invalid garbage bytes to HTTPS port -> server closes connection without 200 OK
    #ifdef _WIN32
    SOCKET c4 = socket(AF_INET, SOCK_STREAM, 0);
    #else
    int c4 = socket(AF_INET, SOCK_STREAM, 0);
    #endif
    sockaddr_in saddr4;
    std::memset(&saddr4, 0, sizeof(saddr4));
    saddr4.sin_family = AF_INET;
    saddr4.sin_port = htons(8445);
    inet_pton(AF_INET, "127.0.0.1", &saddr4.sin_addr);
    connect(c4, (struct sockaddr*)&saddr4, sizeof(saddr4));

    std::string garbage = "GARBAGE_BYTES_1234567890\r\n\r\n";
    send(c4, garbage.data(), static_cast<int>(garbage.length()), 0);
    char buf4[1024] = {0};
    int r4 = recv(c4, buf4, 1023, 0);
    CLOSE_SOCKET(c4);

    if (r4 <= 0 || std::string(buf4, r4).find("HTTP/1.1 200 OK") == std::string::npos) {
        std::cout << "[Test 4] Invalid Bytes Handshake Rejection [PASS]\n";
    } else {
        std::cout << "[Test 4] Invalid Bytes Handshake -> [FAIL] Server returned 200 OK!\n";
        failures++;
    }

    // Test 5: Real TLS curl -k https://127.0.0.1:8445/health -> JSON
    std::string res5 = run_curl_plain("-k -s https://127.0.0.1:8445/health");
    if (res5.find("{\"https\":\"ok\"}") != std::string::npos) {
        std::cout << "[Test 5] Real TLS /health JSON Response [PASS]\n";
    } else {
        std::cout << "[Test 5] Real TLS /health JSON -> [FAIL] Expected JSON, got: " << res5 << "\n";
        failures++;
    }

    if (failures > 0) {
        std::cout << "\n❌ " << failures << "/5 TESTS FAILED ON STUB CODE!\n";
        return 1;
    }

    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL 5 FASE 10.7 HTTPS NO PLAINTEXT TESTS PASSED PERFECTLY!\n";
    std::cout << "========================================\n";
    return 0;
}
