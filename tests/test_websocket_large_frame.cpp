// =====================================================================
// GARUDA COMPILER — FASE 10.7 WEBSOCKET LARGE FRAME & FRAGMENTATION TEST SUITE (RED -> GREEN)
// =====================================================================

#include "../include/garuda/beam/tools/script_engine.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <cstdlib>

static socket_t ws_connect_large(int port) {
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

static std::string build_masked_frame_large(const std::string& payload, uint8_t opcode = 0x1) {
    std::string frame;
    frame.push_back(static_cast<char>(0x80 | (opcode & 0x0F))); // FIN + Opcode

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

static std::string recv_ws_frame_complete_client(socket_t fd, uint8_t& out_opcode) {
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
    std::cout << "GARUDA COMPILER — FASE 10.7 WEBSOCKET LARGE FRAME TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Start WebSocket Server on port 9098 with capacity 50
    std::string code_srv = R"(
        modul TestWSLarge
        fungsi utama() ->
            Server.mulai_ws(9098, nil, 50)
        rampung
    )";
    engine.eval(code_srv);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    socket_t c1 = ws_connect_large(9098);
    if (c1 == INVALID_SOCKET_VAL) {
        std::cout << "[FAIL] Could not connect to WebSocket port 9098\n";
        return 1;
    }

    std::string hs_req = "GET /ws HTTP/1.1\r\nHost: localhost:9098\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n\r\n";
    send(c1, hs_req.data(), static_cast<int>(hs_req.length()), 0);
    char hbuf[1024] = {0};
    recv(c1, hbuf, 1023, 0);

    // Test 1: Send 10KB Text Frame
    std::string data_10k(10240, 'A');
    std::string f1 = build_masked_frame_large(data_10k, 0x1);
    send(c1, f1.data(), static_cast<int>(f1.length()), 0);

    uint8_t op1 = 0;
    std::string p1 = recv_ws_frame_complete_client(c1, op1);
    if (p1.length() == 10240 + 6 && p1.rfind("Echo: ", 0) == 0) {
        std::cout << "[Test 1] Send 10KB Text Frame -> Received 10KB Echo [PASS]\n";
    } else {
        std::cout << "[Test 1] Send 10KB Text Frame -> [FAIL] Expected 10246 bytes, got: " << p1.length() << " bytes\n";
        failures++;
    }

    // Test 2: Send 100KB Text Frame
    std::string data_100k(102400, 'B');
    std::string f2 = build_masked_frame_large(data_100k, 0x1);
    send(c1, f2.data(), static_cast<int>(f2.length()), 0);

    uint8_t op2 = 0;
    std::string p2 = recv_ws_frame_complete_client(c1, op2);
    if (p2.length() == 102400 + 6 && p2.rfind("Echo: ", 0) == 0) {
        std::cout << "[Test 2] Send 100KB Text Frame -> Received 100KB Echo [PASS]\n";
    } else {
        std::cout << "[Test 2] Send 100KB Text Frame -> [FAIL] Expected 102406 bytes, got: " << p2.length() << " bytes\n";
        failures++;
    }

    // Test 3: Send 1KB then 10KB sequentially
    std::string data_1k(1024, 'C');
    std::string f3_1 = build_masked_frame_large(data_1k, 0x1);
    send(c1, f3_1.data(), static_cast<int>(f3_1.length()), 0);

    uint8_t op3_1 = 0;
    std::string p3_1 = recv_ws_frame_complete_client(c1, op3_1);

    std::string f3_2 = build_masked_frame_large(data_10k, 0x1);
    send(c1, f3_2.data(), static_cast<int>(f3_2.length()), 0);

    uint8_t op3_2 = 0;
    std::string p3_2 = recv_ws_frame_complete_client(c1, op3_2);

    if (p3_1.length() == 1024 + 6 && p3_2.length() == 10240 + 6) {
        std::cout << "[Test 3] Sequential 1KB then 10KB Frames -> Correct Lengths [PASS]\n";
    } else {
        std::cout << "[Test 3] Sequential 1KB then 10KB Frames -> [FAIL] Got lengths " << p3_1.length() << " and " << p3_2.length() << "\n";
        failures++;
    }

    // Test 4: Send 20KB Binary Frame
    std::string data_20k_bin(20480, '\x02');
    std::string f4 = build_masked_frame_large(data_20k_bin, 0x2);
    send(c1, f4.data(), static_cast<int>(f4.length()), 0);

    uint8_t op4 = 0;
    std::string p4 = recv_ws_frame_complete_client(c1, op4);
    if (p4.length() == 20480 && op4 == 0x2) {
        std::cout << "[Test 4] Send 20KB Binary Frame -> Received 20KB Binary Echo [PASS]\n";
    } else {
        std::cout << "[Test 4] Send 20KB Binary Frame -> [FAIL] Expected 20480 bytes, got: " << p4.length() << " bytes\n";
        failures++;
    }

    // Test 5: Server Persistence after Large Frames
    std::string ping_f = build_masked_frame_large("ping", 0x1);
    send(c1, ping_f.data(), static_cast<int>(ping_f.length()), 0);

    uint8_t op5 = 0;
    std::string p5 = recv_ws_frame_complete_client(c1, op5);
    CLOSE_SOCKET(c1);

    if (p5 == "pong") {
        std::cout << "[Test 5] Server Persistence after Large Frames -> 'pong' [PASS]\n";
    } else {
        std::cout << "[Test 5] Server Persistence after Large Frames -> [FAIL] Expected 'pong', got: " << p5 << "\n";
        failures++;
    }

    if (failures > 0) {
        std::cout << "\n❌ " << failures << "/5 TESTS FAILED ON STUB CODE!\n";
        return 1;
    }

    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL 5 FASE 10.7 WEBSOCKET LARGE FRAME TESTS PASSED PERFECTLY!\n";
    std::cout << "========================================\n";
    return 0;
}
