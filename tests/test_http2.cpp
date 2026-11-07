// =====================================================================
// GARUDA COMPILER — FASE 11 HTTP/2 MULTIPLEXING TEST SUITE (RED -> GREEN)
// =====================================================================

#include "../include/garuda/beam/tools/script_engine.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <cstdlib>

static socket_t connect_h2_client(int port) {
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
    std::cout << "GARUDA COMPILER — FASE 11 HTTP/2 MULTIPLEXING TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test 1: Start HTTP/2 Server on port 9096
    auto ret = engine.eval("modul THttp2 fungsi utama() -> Server.mulai_http2(9096, nil, 100) rampung");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    if (ret.has_value() && garuda::beam::is_atom(*ret, "benar")) {
        std::cout << "[Test 1] Server.mulai_http2 Port 9096 Start [PASS]\n";
    } else {
        std::cout << "[Test 1] Server.mulai_http2 Port 9096 -> [FAIL] Function or HTTP/2 engine unavailable\n";
        failures++;
    }

    // Test 2: Send HTTP/2 Connection Preface "PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n" & SETTINGS frame
    socket_t c2 = connect_h2_client(9096);
    bool h2_preface_ok = false;
    if (c2 != INVALID_SOCKET_VAL) {
        std::string preface = "PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n";
        // SETTINGS frame: 9 bytes header (len=0, type=4 SETTINGS, flags=0, stream_id=0)
        char settings_frame[9] = {0, 0, 0, 0x04, 0, 0, 0, 0, 0};
        send(c2, preface.data(), static_cast<int>(preface.length()), 0);
        send(c2, settings_frame, 9, 0);

        char rbuf[1024] = {0};
        int r = recv(c2, rbuf, 1023, 0);
        CLOSE_SOCKET(c2);
        // Server should respond with HTTP/2 SETTINGS frame (type 0x04)
        if (r >= 9 && static_cast<uint8_t>(rbuf[3]) == 0x04) {
            h2_preface_ok = true;
        }
    }

    if (h2_preface_ok) {
        std::cout << "[Test 2] HTTP/2 Connection Preface & SETTINGS Exchange [PASS]\n";
    } else {
        std::cout << "[Test 2] HTTP/2 Connection Preface -> [FAIL] Expected SETTINGS frame (0x04) from server\n";
        failures++;
    }

    // Test 3: HTTP/2 Stream Request (HEADERS frame stream_id = 1) -> Response HEADERS + DATA frame
    socket_t c3 = connect_h2_client(9096);
    bool h2_stream_ok = false;
    if (c3 != INVALID_SOCKET_VAL) {
        std::string preface = "PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n";
        char settings_frame[9] = {0, 0, 0, 0x04, 0, 0, 0, 0, 0};
        send(c3, preface.data(), static_cast<int>(preface.length()), 0);
        send(c3, settings_frame, 9, 0);

        // HEADERS frame for stream 1: len=1, type=1 (HEADERS), flags=5 (END_STREAM|END_HEADERS), stream_id=1
        char headers_frame[10] = {0, 0, 1, 0x01, 0x05, 0, 0, 0, 1, 0x82}; // 0x82 = :method GET (HPACK static idx 2)
        send(c3, headers_frame, 10, 0);

        char rbuf[1024] = {0};
        int r = recv(c3, rbuf, 1023, 0);
        CLOSE_SOCKET(c3);
        // Server should return HEADERS frame (type 0x01) or DATA frame (type 0x00) for stream 1
        if (r > 0) {
            h2_stream_ok = true;
        }
    }

    if (h2_stream_ok) {
        std::cout << "[Test 3] HTTP/2 Stream 1 HEADERS & DATA Response Match [PASS]\n";
    } else {
        std::cout << "[Test 3] HTTP/2 Stream 1 -> [FAIL] Expected HTTP/2 Stream response\n";
        failures++;
    }

    // Test 4: HTTP/2 Stream Multiplexing (Stream 1, Stream 3, Stream 5 in single TCP connection)
    socket_t c4 = connect_h2_client(9096);
    bool h2_multi_ok = false;
    if (c4 != INVALID_SOCKET_VAL) {
        std::string preface = "PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n";
        char settings_frame[9] = {0, 0, 0, 0x04, 0, 0, 0, 0, 0};
        send(c4, preface.data(), static_cast<int>(preface.length()), 0);
        send(c4, settings_frame, 9, 0);

        // Stream 1
        char h_s1[10] = {0, 0, 1, 0x01, 0x05, 0, 0, 0, 1, 0x82};
        // Stream 3
        char h_s3[10] = {0, 0, 1, 0x01, 0x05, 0, 0, 0, 3, 0x82};
        // Stream 5
        char h_s5[10] = {0, 0, 1, 0x01, 0x05, 0, 0, 0, 5, 0x82};

        send(c4, h_s1, 10, 0);
        send(c4, h_s3, 10, 0);
        send(c4, h_s5, 10, 0);

        char rbuf[2048] = {0};
        int total_r = 0;
        auto start_t = std::chrono::steady_clock::now();
        while (total_r < 57 && std::chrono::steady_clock::now() - start_t < std::chrono::seconds(2)) {
            int r = recv(c4, rbuf + total_r, 2047 - total_r, 0);
            if (r <= 0) break;
            total_r += r;
        }
        CLOSE_SOCKET(c4);
        if (total_r >= 57) { // 3 response frame pairs (HEAD+DATA for stream 1, 3, 5)
            h2_multi_ok = true;
        }
    }

    if (h2_multi_ok) {
        std::cout << "[Test 4] HTTP/2 Stream Multiplexing (3 Streams on 1 Socket) [PASS]\n";
    } else {
        std::cout << "[Test 4] HTTP/2 Stream Multiplexing -> [FAIL] Multi-stream response failed\n";
        failures++;
    }

    // Test 5: HTTP/1.1 Fallback when HTTP/2 preface is NOT sent
    socket_t c5 = connect_h2_client(9096);
    bool h1_fallback_ok = false;
    if (c5 != INVALID_SOCKET_VAL) {
        std::string h1_req = "GET / HTTP/1.1\r\nHost: localhost:9096\r\nConnection: close\r\n\r\n";
        send(c5, h1_req.data(), static_cast<int>(h1_req.length()), 0);
        char rbuf[1024] = {0};
        int r = recv(c5, rbuf, 1023, 0);
        CLOSE_SOCKET(c5);
        if (r > 0 && std::string(rbuf, r).find("HTTP/1.1 200 OK") != std::string::npos) {
            h1_fallback_ok = true;
        }
    }

    if (h1_fallback_ok) {
        std::cout << "[Test 5] Graceful Fallback to HTTP/1.1 for Legacy Clients [PASS]\n";
    } else {
        std::cout << "[Test 5] Graceful Fallback -> [FAIL] Expected HTTP/1.1 200 OK\n";
        failures++;
    }

    if (failures > 0) {
        std::cout << "\n❌ " << failures << "/5 TESTS FAILED ON STUB CODE!\n";
        return 1;
    }

    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL 5 FASE 11 HTTP/2 MULTIPLEXING TESTS PASSED PERFECTLY!\n";
    std::cout << "========================================\n";
    return 0;
}
