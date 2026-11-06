// =====================================================================
// GARUDA COMPILER — FASE 10.8 WEBSOCKET FRAGMENTATION TEST SUITE (RED -> GREEN)
// =====================================================================

#include "../include/garuda/beam/tools/script_engine.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <cstdlib>

static socket_t ws_connect_frag(int port) {
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

static std::string build_custom_frame(bool fin, uint8_t opcode, const std::string& payload) {
    std::string frame;
    uint8_t b0 = (fin ? 0x80 : 0x00) | (opcode & 0x0F);
    frame.push_back(static_cast<char>(b0));

    size_t len = payload.length();
    if (len <= 125) {
        frame.push_back(static_cast<char>(0x80 | len));
    } else if (len <= 65535) {
        frame.push_back(static_cast<char>(0x80 | 126));
        frame.push_back(static_cast<char>((len >> 8) & 0xFF));
        frame.push_back(static_cast<char>(len & 0xFF));
    } else {
        frame.push_back(static_cast<char>(0x80 | 127));
        for (int i = 7; i >= 0; --i) {
            frame.push_back(static_cast<char>((len >> (i * 8)) & 0xFF));
        }
    }

    uint8_t mask[4] = {0x12, 0x34, 0x56, 0x78};
    for (int i = 0; i < 4; ++i) {
        frame.push_back(static_cast<char>(mask[i]));
    }

    for (size_t i = 0; i < len; ++i) {
        frame.push_back(static_cast<char>(payload[i] ^ mask[i % 4]));
    }
    return frame;
}

static std::string recv_ws_frame_client(socket_t fd, uint8_t& out_opcode) {
    char header_buf[14] = {0};
    int r = recv(fd, header_buf, 2, 0);
    if (r < 2) return "";

    out_opcode = static_cast<uint8_t>(header_buf[0]) & 0x0F;
    size_t payload_len = static_cast<uint8_t>(header_buf[1]) & 0x7F;

    if (payload_len == 126) {
        recv(fd, header_buf + 2, 2, 0);
        payload_len = (static_cast<uint8_t>(header_buf[2]) << 8) | static_cast<uint8_t>(header_buf[3]);
    } else if (payload_len == 127) {
        recv(fd, header_buf + 2, 8, 0);
        payload_len = 0;
        for (int i = 0; i < 8; ++i) {
            payload_len = (payload_len << 8) | static_cast<uint8_t>(header_buf[2 + i]);
        }
    }

    std::string payload = "";
    payload.resize(payload_len);
    size_t total_read = 0;
    while (total_read < payload_len) {
        int bytes = recv(fd, &payload[total_read], static_cast<int>(payload_len - total_read), 0);
        if (bytes <= 0) break;
        total_read += bytes;
    }
    return payload;
}

int main() {
    garuda::beam::ScriptEngine engine;
    int failures = 0;

    std::cout << "=======================================================\n";
    std::cout << "GARUDA COMPILER — FASE 10.8 WEBSOCKET FRAGMENTATION TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Start WebSocket Server on port 9101
    engine.eval("modul TFrag fungsi utama() -> Server.mulai_ws(9101, nil, 50) rampung");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    socket_t c1 = ws_connect_frag(9101);
    if (c1 == INVALID_SOCKET_VAL) {
        std::cout << "[FAIL] Connect WebSocket socket on 9101\n";
        return 1;
    }

    std::string hs = "GET /ws HTTP/1.1\r\nHost: localhost:9101\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n\r\n";
    send(c1, hs.data(), static_cast<int>(hs.length()), 0);
    char hbuf[1024] = {0};
    recv(c1, hbuf, 1023, 0);

    // Test 1: 10KB payload as 3 fragments (3KB FIN=0 + 3KB FIN=0 + 4KB FIN=1)
    std::string a3k(3072, 'A');
    std::string b3k(3072, 'B');
    std::string c4k(4096, 'C');

    send(c1, build_custom_frame(false, 0x1, a3k).data(), static_cast<int>(build_custom_frame(false, 0x1, a3k).length()), 0);
    send(c1, build_custom_frame(false, 0x0, b3k).data(), static_cast<int>(build_custom_frame(false, 0x0, b3k).length()), 0);
    send(c1, build_custom_frame(true, 0x0, c4k).data(), static_cast<int>(build_custom_frame(true, 0x0, c4k).length()), 0);

    uint8_t op1 = 0;
    std::string p1 = recv_ws_frame_client(c1, op1);
    std::string exp1 = "Echo: " + a3k + b3k + c4k;
    if (p1 == exp1) {
        std::cout << "[Test 1] 3 Fragments (3KB+3KB+4KB) FIN=0 -> Reassembled 10KB Echo [PASS]\n";
    } else {
        std::cout << "[Test 1] 3 Fragments -> [FAIL] Expected 10246 bytes, got: " << p1.length() << " bytes\n";
        failures++;
    }

    // Test 2: 20KB payload as 2 fragments (10KB FIN=0 + 10KB FIN=1)
    std::string d10k(10240, 'D');
    std::string e10k(10240, 'E');

    send(c1, build_custom_frame(false, 0x1, d10k).data(), static_cast<int>(build_custom_frame(false, 0x1, d10k).length()), 0);
    send(c1, build_custom_frame(true, 0x0, e10k).data(), static_cast<int>(build_custom_frame(true, 0x0, e10k).length()), 0);

    uint8_t op2 = 0;
    std::string p2 = recv_ws_frame_client(c1, op2);
    std::string exp2 = "Echo: " + d10k + e10k;
    if (p2 == exp2) {
        std::cout << "[Test 2] 2 Fragments (10KB+10KB) FIN=0 -> Reassembled 20KB Echo [PASS]\n";
    } else {
        std::cout << "[Test 2] 2 Fragments -> [FAIL] Expected 20486 bytes, got: " << p2.length() << " bytes\n";
        failures++;
    }

    // Test 3: Fragmented 5KB frame followed by single non-fragmented 1KB frame
    std::string f25k(2560, 'F');
    std::string g25k(2560, 'G');
    std::string h1k(1024, 'H');

    send(c1, build_custom_frame(false, 0x1, f25k).data(), static_cast<int>(build_custom_frame(false, 0x1, f25k).length()), 0);
    send(c1, build_custom_frame(true, 0x0, g25k).data(), static_cast<int>(build_custom_frame(true, 0x0, g25k).length()), 0);

    uint8_t op3_1 = 0;
    std::string p3_1 = recv_ws_frame_client(c1, op3_1);

    send(c1, build_custom_frame(true, 0x1, h1k).data(), static_cast<int>(build_custom_frame(true, 0x1, h1k).length()), 0);

    uint8_t op3_2 = 0;
    std::string p3_2 = recv_ws_frame_client(c1, op3_2);

    if (p3_1 == "Echo: " + f25k + g25k && p3_2 == "Echo: " + h1k) {
        std::cout << "[Test 3] Fragmented Frame followed by Unfragmented Frame [PASS]\n";
    } else {
        std::cout << "[Test 3] Fragmented followed by Unfragmented -> [FAIL]\n";
        failures++;
    }

    // Test 4: 5 small 1KB fragments
    std::string k1(1024, 'K');
    send(c1, build_custom_frame(false, 0x1, k1).data(), static_cast<int>(build_custom_frame(false, 0x1, k1).length()), 0);
    send(c1, build_custom_frame(false, 0x0, k1).data(), static_cast<int>(build_custom_frame(false, 0x0, k1).length()), 0);
    send(c1, build_custom_frame(false, 0x0, k1).data(), static_cast<int>(build_custom_frame(false, 0x0, k1).length()), 0);
    send(c1, build_custom_frame(false, 0x0, k1).data(), static_cast<int>(build_custom_frame(false, 0x0, k1).length()), 0);
    send(c1, build_custom_frame(true, 0x0, k1).data(), static_cast<int>(build_custom_frame(true, 0x0, k1).length()), 0);

    uint8_t op4 = 0;
    std::string p4 = recv_ws_frame_client(c1, op4);
    std::string exp4 = "Echo: " + k1 + k1 + k1 + k1 + k1;
    if (p4 == exp4) {
        std::cout << "[Test 4] 5 Small 1KB Fragments FIN=0 -> Reassembled 5KB Echo [PASS]\n";
    } else {
        std::cout << "[Test 4] 5 Small Fragments -> [FAIL] Expected 5126 bytes, got: " << p4.length() << " bytes\n";
        failures++;
    }

    // Test 5: Fragmented Binary Frame (opcode 0x2 FIN=0 + opcode 0x0 FIN=1 15KB)
    std::string b75k(7680, '\x02');
    send(c1, build_custom_frame(false, 0x2, b75k).data(), static_cast<int>(build_custom_frame(false, 0x2, b75k).length()), 0);
    send(c1, build_custom_frame(true, 0x0, b75k).data(), static_cast<int>(build_custom_frame(true, 0x0, b75k).length()), 0);

    uint8_t op5 = 0;
    std::string p5 = recv_ws_frame_client(c1, op5);
    CLOSE_SOCKET(c1);

    if (op5 == 0x2 && p5 == b75k + b75k) {
        std::cout << "[Test 5] Fragmented Binary Frame FIN=0 -> Reassembled 15KB Binary [PASS]\n";
    } else {
        std::cout << "[Test 5] Fragmented Binary Frame -> [FAIL] Expected 15360 bytes, got: " << p5.length() << " bytes\n";
        failures++;
    }

    if (failures > 0) {
        std::cout << "\n❌ " << failures << "/5 TESTS FAILED ON STUB CODE!\n";
        return 1;
    }

    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL 5 FASE 10.8 WEBSOCKET FRAGMENTATION TESTS PASSED PERFECTLY!\n";
    std::cout << "========================================\n";
    return 0;
}
