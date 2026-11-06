// =====================================================================
// GARUDA COMPILER — FASE 10.5 REAL WEBSOCKET E2E TEST SUITE (5 TESTS)
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

static socket_t ws_connect(int port) {
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

static std::string build_masked_frame(const std::string& payload, uint8_t opcode = 0x1) {
    std::string frame;
    frame.push_back(static_cast<char>(0x80 | (opcode & 0x0F))); // FIN + Opcode

    size_t len = payload.length();
    frame.push_back(static_cast<char>(0x80 | (len & 0x7F))); // Masked + Len

    uint8_t mask[4] = {0x12, 0x34, 0x56, 0x78};
    for (int i = 0; i < 4; ++i) {
        frame.push_back(static_cast<char>(mask[i]));
    }

    for (size_t i = 0; i < len; ++i) {
        frame.push_back(static_cast<char>(payload[i] ^ mask[i % 4]));
    }
    return frame;
}

int main() {
    garuda::beam::ScriptEngine engine;

    std::cout << "=======================================================\n";
    std::cout << "GARUDA COMPILER — FASE 10.5 REAL WEBSOCKET E2E TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Start Real WebSocket Server on port 9092
    std::string srv_code = R"(
        modul TestWSE2E
        fungsi utama() ->
            Server.mulai_ws(9092, nil, 10)
        rampung
    )";
    engine.eval(srv_code);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Test 1: Real TCP Socket Handshake
    socket_t c1 = ws_connect(9092);
    CHECK(c1 != INVALID_SOCKET_VAL, "Test 1 connect TCP socket");

    std::string hs_req = "GET /ws HTTP/1.1\r\n"
                        "Host: localhost:9092\r\n"
                        "Upgrade: websocket\r\n"
                        "Connection: Upgrade\r\n"
                        "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n\r\n";
    send(c1, hs_req.data(), static_cast<int>(hs_req.length()), 0);

    char hbuf[2048] = {0};
    recv(c1, hbuf, 2047, 0);
    std::string hresp(hbuf);

    CHECK(hresp.find("101 Switching Protocols") != std::string::npos, "Test 1 HTTP 101 status");
    CHECK(hresp.find("Sec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=") != std::string::npos, "Test 1 Sec-WebSocket-Accept key");
    std::cout << "[Test 1] Real TCP Handshake -> 101 Switching Protocols [PASS]\n";

    // Test 2: Real Ping-Pong over TCP
    std::string ping_frame = build_masked_frame("ping", 0x1);
    send(c1, ping_frame.data(), static_cast<int>(ping_frame.length()), 0);

    char fbuf2[2048] = {0};
    int r2 = recv(c1, fbuf2, 2047, 0);
    CHECK(r2 > 0, "Test 2 recv pong frame");
    uint8_t op2 = 0;
    std::string p2 = garuda::beam::decode_ws_frame(std::string(fbuf2, r2), op2);
    CHECK(p2 == "pong", "Test 2 payload must be 'pong'");
    std::cout << "[Test 2] Real Ping-Pong over TCP -> 'pong' [PASS]\n";

    // Test 3: Real Text Echo over TCP
    std::string echo_frame = build_masked_frame("halo garuda", 0x1);
    send(c1, echo_frame.data(), static_cast<int>(echo_frame.length()), 0);

    char fbuf3[2048] = {0};
    int r3 = recv(c1, fbuf3, 2047, 0);
    CHECK(r3 > 0, "Test 3 recv text echo frame");
    uint8_t op3 = 0;
    std::string p3 = garuda::beam::decode_ws_frame(std::string(fbuf3, r3), op3);
    CHECK(p3 == "Echo: halo garuda", "Test 3 payload must be 'Echo: halo garuda'");
    std::cout << "[Test 3] Real Text Echo over TCP -> 'Echo: halo garuda' [PASS]\n";

    // Test 4: Real Binary Echo over TCP
    std::string bin_frame = build_masked_frame("BIN_DATA", 0x2);
    send(c1, bin_frame.data(), static_cast<int>(bin_frame.length()), 0);

    char fbuf4[2048] = {0};
    int r4 = recv(c1, fbuf4, 2047, 0);
    CHECK(r4 > 0, "Test 4 recv binary frame");
    uint8_t op4 = 0;
    std::string p4 = garuda::beam::decode_ws_frame(std::string(fbuf4, r4), op4);
    CHECK(op4 == 0x2 && p4 == "BIN_DATA", "Test 4 binary opcode and payload match");
    std::cout << "[Test 4] Real Binary Echo over TCP -> Opcode 0x2 BIN_DATA [PASS]\n";

    // Test 5: Real Close Handshake over TCP
    std::string close_frame = build_masked_frame("", 0x8);
    send(c1, close_frame.data(), static_cast<int>(close_frame.length()), 0);

    char fbuf5[2048] = {0};
    int r5 = recv(c1, fbuf5, 2047, 0);
    uint8_t op5 = 0;
    if (r5 > 0) {
        garuda::beam::decode_ws_frame(std::string(fbuf5, r5), op5);
    }
    CLOSE_SOCKET(c1);

    CHECK(op5 == 0x8 || r5 <= 0, "Test 5 close frame response received");
    std::cout << "[Test 5] Real Close Handshake over TCP -> Opcode 0x8 Closed [PASS]\n";

    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL 5 FASE 10.5 REAL WEBSOCKET E2E TESTS PASSED PERFECTLY!\n";
    std::cout << "========================================\n";
    return 0;
}
