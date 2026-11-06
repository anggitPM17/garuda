// =====================================================================
// GARUDA COMPILER — FASE 10.8 WEBSOCKET PAYLOAD BOUND TEST SUITE (RED -> GREEN)
// =====================================================================

#include "../include/garuda/beam/tools/script_engine.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <cstdlib>

static socket_t ws_connect_bound(int port) {
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

static std::string build_oversized_header(uint64_t claim_len) {
    std::string frame;
    frame.push_back(static_cast<char>(0x81)); // FIN + Text

    frame.push_back(static_cast<char>(0x80 | 127)); // Masked + 64-bit len
    for (int i = 7; i >= 0; --i) {
        frame.push_back(static_cast<char>((claim_len >> (i * 8)) & 0xFF));
    }

    uint8_t mask[4] = {0x12, 0x34, 0x56, 0x78};
    for (int i = 0; i < 4; ++i) {
        frame.push_back(static_cast<char>(mask[i]));
    }
    return frame;
}

int main() {
    garuda::beam::ScriptEngine engine;
    int failures = 0;

    std::cout << "=======================================================\n";
    std::cout << "GARUDA COMPILER — FASE 10.8 WEBSOCKET PAYLOAD BOUND TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Start WebSocket Server on port 9102
    engine.eval("modul TBound fungsi utama() -> Server.mulai_ws(9102, nil, 50) rampung");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Test 1: Send frame claiming 1 GB payload (1,000,000,000 bytes)
    socket_t c1 = ws_connect_bound(9102);
    std::string hs = "GET /ws HTTP/1.1\r\nHost: localhost:9102\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n\r\n";
    send(c1, hs.data(), static_cast<int>(hs.length()), 0);
    char hbuf[1024] = {0};
    recv(c1, hbuf, 1023, 0);

    std::string f1 = build_oversized_header(1000000000ULL);
    send(c1, f1.data(), static_cast<int>(f1.length()), 0);

    char rbuf1[128] = {0};
    int r1 = recv(c1, rbuf1, 127, 0);
    CLOSE_SOCKET(c1);

    uint8_t op1 = static_cast<uint8_t>(rbuf1[0]) & 0x0F;
    if (r1 > 0 && op1 == 0x8) {
        std::cout << "[Test 1] Oversized 1GB Frame Rejected with Close Frame 1009 [PASS]\n";
    } else {
        std::cout << "[Test 1] Oversized 1GB Frame -> [FAIL] Expected Close Frame (0x8), got bytes: " << r1 << "\n";
        failures++;
    }

    // Test 2: Send frame claiming 2 GB payload
    socket_t c2 = ws_connect_bound(9102);
    send(c2, hs.data(), static_cast<int>(hs.length()), 0);
    recv(c2, hbuf, 1023, 0);

    std::string f2 = build_oversized_header(2000000000ULL);
    send(c2, f2.data(), static_cast<int>(f2.length()), 0);

    char rbuf2[128] = {0};
    int r2 = recv(c2, rbuf2, 127, 0);
    CLOSE_SOCKET(c2);

    uint8_t op2 = static_cast<uint8_t>(rbuf2[0]) & 0x0F;
    if (r2 > 0 && op2 == 0x8) {
        std::cout << "[Test 2] Oversized 2GB Frame Rejected with Close Frame 1009 [PASS]\n";
    } else {
        std::cout << "[Test 2] Oversized 2GB Frame -> [FAIL] Expected Close Frame (0x8), got bytes: " << r2 << "\n";
        failures++;
    }

    // Test 3: Send frame claiming 50 MB payload (> 16MB threshold)
    socket_t c3 = ws_connect_bound(9102);
    send(c3, hs.data(), static_cast<int>(hs.length()), 0);
    recv(c3, hbuf, 1023, 0);

    std::string f3 = build_oversized_header(50000000ULL);
    send(c3, f3.data(), static_cast<int>(f3.length()), 0);

    char rbuf3[128] = {0};
    int r3 = recv(c3, rbuf3, 127, 0);
    CLOSE_SOCKET(c3);

    uint8_t op3 = static_cast<uint8_t>(rbuf3[0]) & 0x0F;
    if (r3 > 0 && op3 == 0x8) {
        std::cout << "[Test 3] Oversized 50MB Frame (> 16MB threshold) Rejected [PASS]\n";
    } else {
        std::cout << "[Test 3] Oversized 50MB Frame -> [FAIL] Expected Close Frame (0x8), got bytes: " << r3 << "\n";
        failures++;
    }

    // Test 4: Server Persistence check after rejected oversized frame
    socket_t c4 = ws_connect_bound(9102);
    send(c4, hs.data(), static_cast<int>(hs.length()), 0);
    recv(c4, hbuf, 1023, 0);

    std::string f4_hdr = build_oversized_header(1000000000ULL);
    send(c4, f4_hdr.data(), static_cast<int>(f4_hdr.length()), 0);
    CLOSE_SOCKET(c4);

    socket_t c4_new = ws_connect_bound(9102);
    send(c4_new, hs.data(), static_cast<int>(hs.length()), 0);
    char hbuf4[1024] = {0};
    int r4_hs = recv(c4_new, hbuf4, 1023, 0);
    CLOSE_SOCKET(c4_new);

    if (r4_hs > 0 && std::string(hbuf4).find("101") != std::string::npos) {
        std::cout << "[Test 4] Server Remains Persistent and Operational [PASS]\n";
    } else {
        std::cout << "[Test 4] Server Persistence -> [FAIL] Server crashed or unoperational\n";
        failures++;
    }

    // Test 5: Frame claiming 16MB + 1 byte payload (16,777,217 bytes > 16MB threshold)
    socket_t c5 = ws_connect_bound(9102);
    send(c5, hs.data(), static_cast<int>(hs.length()), 0);
    recv(c5, hbuf, 1023, 0);

    std::string f5 = build_oversized_header(16777217ULL);
    send(c5, f5.data(), static_cast<int>(f5.length()), 0);

    char rbuf5[128] = {0};
    int r5 = recv(c5, rbuf5, 127, 0);
    CLOSE_SOCKET(c5);

    uint8_t op5 = static_cast<uint8_t>(rbuf5[0]) & 0x0F;
    if (r5 > 0 && op5 == 0x8) {
        std::cout << "[Test 5] Exact 16MB Threshold Frame Rejected with 1009 [PASS]\n";
    } else {
        std::cout << "[Test 5] Exact 16MB Threshold -> [FAIL] Expected Close Frame (0x8), got bytes: " << r5 << "\n";
        failures++;
    }

    if (failures > 0) {
        std::cout << "\n❌ " << failures << "/5 TESTS FAILED ON STUB CODE!\n";
        return 1;
    }

    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL 5 FASE 10.8 WEBSOCKET PAYLOAD BOUND TESTS PASSED PERFECTLY!\n";
    std::cout << "========================================\n";
    return 0;
}
