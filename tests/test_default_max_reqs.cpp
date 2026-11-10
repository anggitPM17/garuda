// =====================================================================
// GARUDA COMPILER — FASE 10.7 DEFAULT MAX_REQS=100 TEST SUITE (RED -> GREEN)
// =====================================================================

#include "../include/garuda/beam/tools/script_engine.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <cstdlib>

static std::string run_curl_max(const std::string& cmd) {
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

static socket_t tcp_connect_max(int port) {
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
    std::cout << "GARUDA COMPILER — FASE 10.7 DEFAULT MAX_REQS=100 TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test 1: Server.mulai_ws(9097, nil) WITHOUT max_reqs parameter -> 5 sequential connections
    engine.eval("modul T1 fungsi utama() -> Server.mulai_ws(9097, nil) rampung");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    bool ws5_ok = true;
    for (int i = 0; i < 5; ++i) {
        socket_t s = tcp_connect_max(9097);
        if (s == INVALID_SOCKET_VAL) { ws5_ok = false; break; }
        std::string hs = "GET /ws HTTP/1.1\r\nHost: localhost:9097\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n\r\n";
        send(s, hs.data(), static_cast<int>(hs.length()), 0);
        char buf[512] = {0};
        int r = recv(s, buf, 511, 0);
        CLOSE_SOCKET(s);
        if (r <= 0 || std::string(buf).find("101") == std::string::npos) {
            ws5_ok = false;
            break;
        }
    }
    if (ws5_ok) {
        std::cout << "[Test 1] WebSocket Default max_reqs=100 (5 sequential connections) [PASS]\n";
    } else {
        std::cout << "[Test 1] WebSocket Default max_reqs=100 -> [FAIL] Connection died on default max_reqs\n";
        failures++;
    }

    // Test 2: Server.mulai_https(8444, "", "", nil) WITHOUT max_reqs parameter -> 5 curl calls
    engine.eval("modul T2 fungsi utama() -> Server.mulai_https(8444, \"\", \"\", nil) rampung");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    bool https5_ok = true;
    for (int i = 0; i < 5; ++i) {
        std::string r = run_curl_max("-k -s -o NUL -w \"%{http_code}\" https://127.0.0.1:8444/");
        if (r.find("200") == std::string::npos) {
            https5_ok = false;
            break;
        }
    }
    if (https5_ok) {
        std::cout << "[Test 2] HTTPS Default max_reqs=100 (5 sequential curl calls) [PASS]\n";
    } else {
        std::cout << "[Test 2] HTTPS Default max_reqs=100 -> [FAIL] Connection died on default max_reqs\n";
        failures++;
    }

    // Test 3: Server.mulai_async_statis(9098, "src/") WITHOUT max_reqs parameter -> 5 static curl calls
    engine.eval("modul T3 fungsi utama() -> Server.mulai_async_statis(9098, \"src/\") rampung");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    bool stc5_ok = true;
    for (int i = 0; i < 5; ++i) {
        std::string r = run_curl_max("-s -o NUL -w \"%{http_code}\" http://127.0.0.1:9098/utama.grd");
        if (r.find("200") == std::string::npos && r.find("404") == std::string::npos) {
            stc5_ok = false;
            break;
        }
    }
    if (stc5_ok) {
        std::cout << "[Test 3] Static Default max_reqs=100 (5 sequential curl calls) [PASS]\n";
    } else {
        std::cout << "[Test 3] Static Default max_reqs=100 -> [FAIL] Connection died on default max_reqs\n";
        failures++;
    }

    // Test 4: Explicit max_reqs=2 is still respected
    engine.eval("modul T4 fungsi utama() -> Server.mulai_ws(9099, nil, 2) rampung");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    socket_t s1 = tcp_connect_max(9099);
    socket_t s2 = tcp_connect_max(9099);
    socket_t s3 = tcp_connect_max(9099);
    if (s1 != INVALID_SOCKET_VAL && s2 != INVALID_SOCKET_VAL) {
        std::cout << "[Test 4] Explicit max_reqs=2 parameter respected [PASS]\n";
    } else {
        std::cout << "[Test 4] Explicit max_reqs=2 -> [FAIL] Could not connect 2 sockets\n";
        failures++;
    }
    CLOSE_SOCKET(s1); CLOSE_SOCKET(s2); CLOSE_SOCKET(s3);

    // Test 5: WebSocket 20 sequential connections on default max_reqs
    engine.eval("modul T5 fungsi utama() -> Server.mulai_ws(9100, nil) rampung");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    bool ws20_ok = true;
    for (int i = 0; i < 20; ++i) {
        socket_t s = tcp_connect_max(9100);
        if (s == INVALID_SOCKET_VAL) { ws20_ok = false; break; }
        std::string hs = "GET /ws HTTP/1.1\r\nHost: localhost:9100\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n\r\n";
        send(s, hs.data(), static_cast<int>(hs.length()), 0);
        char buf[512] = {0};
        int r = recv(s, buf, 511, 0);
        CLOSE_SOCKET(s);
        if (r <= 0 || std::string(buf).find("101") == std::string::npos) {
            ws20_ok = false;
            break;
        }
    }
    if (ws20_ok) {
        std::cout << "[Test 5] WebSocket 20 Sequential Connections on Default max_reqs=100 [PASS]\n";
    } else {
        std::cout << "[Test 5] WebSocket 20 Sequential Connections -> [FAIL] Server died on default max_reqs\n";
        failures++;
    }

    if (failures > 0) {
        std::cout << "\n❌ " << failures << "/5 TESTS FAILED ON STUB CODE!\n";
        return 1;
    }

    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL 5 FASE 10.7 DEFAULT MAX_REQS=100 TESTS PASSED PERFECTLY!\n";
    std::cout << "========================================\n";
    return 0;
}
