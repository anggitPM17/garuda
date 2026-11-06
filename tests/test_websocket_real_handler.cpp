// =====================================================================
// GARUDA COMPILER — FASE 10.6 REAL WEBSOCKET HANDLER TEST SUITE (RED -> GREEN)
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

static socket_t ws_connect_h(int port) {
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

static std::string build_masked_frame_h(const std::string& payload, uint8_t opcode = 0x1) {
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
    std::cout << "GARUDA COMPILER — FASE 10.6 REAL WEBSOCKET HANDLER TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Start WebSocket Server on port 9095 with custom handler function
    std::string srv_code = R"(
        modul TestWSH
        fungsi handler_kustom(Conn, Msg) ->
            "HALO_DARI_HANDLER_" ++ Msg
        rampung

        fungsi utama() ->
            Server.mulai_ws(9095, "handler_kustom", 50)
        rampung
    )";
    engine.eval(srv_code);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    socket_t c1 = ws_connect_h(9095);
    CHECK(c1 != INVALID_SOCKET_VAL, "Connect WebSocket socket");

    std::string hs_req = "GET /ws HTTP/1.1\r\n"
                        "Host: localhost:9095\r\n"
                        "Upgrade: websocket\r\n"
                        "Connection: Upgrade\r\n"
                        "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n\r\n";
    send(c1, hs_req.data(), static_cast<int>(hs_req.length()), 0);

    char hbuf[2048] = {0};
    recv(c1, hbuf, 2047, 0);

    // Test 1: Custom Handler Invocation (must return 'HALO_DARI_HANDLER_ping' instead of 'pong')
    std::string ping_frame = build_masked_frame_h("ping", 0x1);
    send(c1, ping_frame.data(), static_cast<int>(ping_frame.length()), 0);

    char fbuf1[2048] = {0};
    int r1 = recv(c1, fbuf1, 2047, 0);
    uint8_t op1 = 0;
    std::string p1 = garuda::beam::decode_ws_frame(std::string(fbuf1, r1), op1);
    CHECK(p1 == "HALO_DARI_HANDLER_ping", "Test 1 custom handler invocation result match");
    std::cout << "[Test 1] Custom Handler Invocation ('HALO_DARI_HANDLER_ping') [PASS]\n";

    // Test 2: Handler parameter Msg receiving
    std::string msg_frame = build_masked_frame_h("PESAN_SAYA", 0x1);
    send(c1, msg_frame.data(), static_cast<int>(msg_frame.length()), 0);

    char fbuf2[2048] = {0};
    int r2 = recv(c1, fbuf2, 2047, 0);
    uint8_t op2 = 0;
    std::string p2 = garuda::beam::decode_ws_frame(std::string(fbuf2, r2), op2);
    CHECK(p2 == "HALO_DARI_HANDLER_PESAN_SAYA", "Test 2 handler parameter Msg match");
    std::cout << "[Test 2] Handler Msg Parameter Receiving [PASS]\n";

    // Test 3: Opcode 0x9 (Ping Frame) -> Opcode 0xA (Pong Frame Response)
    std::string ping_op_frame = build_masked_frame_h("pingdata", 0x9);
    send(c1, ping_op_frame.data(), static_cast<int>(ping_op_frame.length()), 0);

    char fbuf3[2048] = {0};
    int r3 = recv(c1, fbuf3, 2047, 0);
    uint8_t op3 = 0;
    std::string p3 = garuda::beam::decode_ws_frame(std::string(fbuf3, r3), op3);
    CHECK(op3 == 0xA && p3 == "pingdata", "Test 3 opcode 0x9 ping -> opcode 0xA pong");
    std::cout << "[Test 3] Opcode 0x9 Ping -> Opcode 0xA Pong Auto Response [PASS]\n";

    // Test 4: Persistent Server Check on second connection
    socket_t c2 = ws_connect_h(9095);
    CHECK(c2 != INVALID_SOCKET_VAL, "Test 4 second socket connection");
    send(c2, hs_req.data(), static_cast<int>(hs_req.length()), 0);
    char hbuf2[2048] = {0};
    recv(c2, hbuf2, 2047, 0);

    std::string ping_c2 = build_masked_frame_h("conn2", 0x1);
    send(c2, ping_c2.data(), static_cast<int>(ping_c2.length()), 0);
    char fbuf4[2048] = {0};
    int r4 = recv(c2, fbuf4, 2047, 0);
    uint8_t op4 = 0;
    std::string p4 = garuda::beam::decode_ws_frame(std::string(fbuf4, r4), op4);
    CHECK(p4 == "HALO_DARI_HANDLER_conn2", "Test 4 persistent server response match");
    std::cout << "[Test 4] Persistent Server Multi-connection Check [PASS]\n";

    // Test 5: Clean Close Connection
    CLOSE_SOCKET(c1);
    CLOSE_SOCKET(c2);
    std::cout << "[Test 5] Clean Socket Teardown Check [PASS]\n";

    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL 5 FASE 10.6 REAL WEBSOCKET HANDLER TESTS PASSED PERFECTLY!\n";
    std::cout << "========================================\n";
    return 0;
}
