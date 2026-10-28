#include "../include/garuda/beam/io/http_ws_server.hpp"
#include "../include/garuda/beam/stdlib/crypto.hpp"
#include "../include/garuda/beam/core/runtime.hpp"
#if !defined(_WIN32)
#include <arpa/inet.h>
#endif
#include <atomic>
#include <chrono>
#include <future>
#include <iostream>
#include <thread>
#include <vector>

using namespace garuda::beam;

#define GARUDA_CHECK(cond, msg) \
    if (!(cond)) { \
        std::cerr << "  ❌ ASSERTION FAILED: " << msg << " (" << #cond << ")\n"; \
        std::exit(1); \
    }

void run_test_with_timeout(const std::string& test_name, std::function<void()> test_fn, int timeout_seconds = 10) {
    auto future = std::async(std::launch::async, test_fn);
    if (future.wait_for(std::chrono::seconds(timeout_seconds)) == std::future_status::timeout) {
        std::cerr << "  ❌ " << test_name << " FAILED (TIMEOUT > " << timeout_seconds << "s)\n";
        std::exit(1); \
    }
}

// Minimal WebSocket Client Frame Encoder
std::vector<std::uint8_t> encode_ws_client_text_frame(const std::string& payload) {
    std::vector<std::uint8_t> frame;
    frame.push_back(0x81); // FIN (1) + Opcode (1 = Text)

    std::size_t len = payload.length();
    if (len <= 125) {
        frame.push_back(static_cast<std::uint8_t>(0x80 | len)); // Mask bit = 1
    } else {
        frame.push_back(0x80 | 126);
        frame.push_back(static_cast<std::uint8_t>((len >> 8) & 0xFF));
        frame.push_back(static_cast<std::uint8_t>(len & 0xFF));
    }

    // Masking Key
    std::uint8_t mask[4] = {0x12, 0x34, 0x56, 0x78};
    for (int i = 0; i < 4; ++i) frame.push_back(mask[i]);

    // Masked Payload
    for (std::size_t i = 0; i < len; ++i) {
        frame.push_back(static_cast<std::uint8_t>(payload[i]) ^ mask[i % 4]);
    }

    return frame;
}

struct WsDecodedFrame {
    std::uint8_t opcode = 0;
    std::string payload;
};

WsDecodedFrame decode_ws_frame(const std::uint8_t* buf, std::size_t len) {
    WsDecodedFrame frame;
    if (len < 2) return frame;

    frame.opcode = buf[0] & 0x0F;
    bool masked = (buf[1] & 0x80) != 0;
    std::size_t payload_len = buf[1] & 0x7F;
    std::size_t offset = 2;

    if (payload_len == 126) {
        if (len < 4) return frame;
        payload_len = (buf[2] << 8) | buf[3];
        offset = 4;
    }

    std::uint8_t mask[4] = {0, 0, 0, 0};
    if (masked) {
        if (len < offset + 4) return frame;
        for (int i = 0; i < 4; ++i) mask[i] = buf[offset++];
    }

    std::string text;
    text.reserve(payload_len);
    for (std::size_t i = 0; i < payload_len && (offset + i) < len; ++i) {
        std::uint8_t b = buf[offset + i];
        if (masked) b ^= mask[i % 4];
        text += static_cast<char>(b);
    }

    frame.payload = text;
    return frame;
}

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA WEBSOCKET END-TO-END SUITE\n";
    std::cout << "=======================================================\n\n";

    // === Test 1: Real WebSocket Server Echo Roundtrip ===
    run_test_with_timeout("Test 1: WebSocket Handshake & Server Echo Roundtrip", []() {
        Runtime rt;
        EventLoop loop(rt);
        int port = 8100;

        HttpWsServer server(loop, port);

        // Register Server WS Handler
        server.set_ws_handler([](const std::string& msg, std::function<void(const std::string&)> send_reply) {
            send_reply("Pong: " + msg);
        });

        bool started = server.start();
        GARUDA_CHECK(started, "HttpWsServer must start on port 8100");

        // Run EventLoop in background thread
        std::thread loop_thread([&loop]() {
            loop.run();
        });

        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        // Connect Client Socket
        socket_t client_fd = ::socket(AF_INET, SOCK_STREAM, 0);
        GARUDA_CHECK(client_fd != INVALID_SOCK, "Client socket creation must succeed");

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(static_cast<u_short>(port));
        inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

        int conn_rc = ::connect(client_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
        GARUDA_CHECK(conn_rc == 0, "Socket connect to 127.0.0.1:8100 must succeed");

        // 1. Send Handshake Request
        std::string req =
            "GET /ws HTTP/1.1\r\n"
            "Host: 127.0.0.1:8100\r\n"
            "Upgrade: websocket\r\n"
            "Connection: Upgrade\r\n"
            "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n\r\n";

        ::send(client_fd, req.c_str(), static_cast<int>(req.length()), 0);

        char buf[1024];
        int bytes = ::recv(client_fd, buf, sizeof(buf) - 1, 0);
        GARUDA_CHECK(bytes > 0, "Must receive HTTP 101 Handshake Response");
        buf[bytes] = '\0';

        std::string handshake_resp(buf, bytes);
        GARUDA_CHECK(handshake_resp.find("101 Switching Protocols") != std::string::npos, "Handshake must return HTTP 101");
        GARUDA_CHECK(handshake_resp.find("Upgrade: websocket") != std::string::npos, "Handshake must return Upgrade: websocket");

        // 2. Client Sends Masked Text Frame "Hello Garuda"
        std::string msg = "Hello Garuda";
        auto client_frame = encode_ws_client_text_frame(msg);
        ::send(client_fd, reinterpret_cast<const char*>(client_frame.data()), static_cast<int>(client_frame.size()), 0);

        // 3. Client Receives Server Reply Frame Over Socket
        std::uint8_t recv_buf[1024];
        int recv_bytes = ::recv(client_fd, reinterpret_cast<char*>(recv_buf), sizeof(recv_buf), 0);
        GARUDA_CHECK(recv_bytes > 0, "Must receive WebSocket frame reply from server");

        WsDecodedFrame server_reply = decode_ws_frame(recv_buf, static_cast<std::size_t>(recv_bytes));
        GARUDA_CHECK(server_reply.opcode == 0x1, "Server reply frame opcode must be 0x1 (Text Frame)");
        GARUDA_CHECK(server_reply.payload == "Pong: Hello Garuda", "Server reply payload must match 'Pong: Hello Garuda'");

        CLOSE_SOCK(client_fd);
        server.stop();
        loop.stop();

        if (loop_thread.joinable()) loop_thread.join();

        std::cout << "  ✓ Test 1: WebSocket Handshake & Server Echo Roundtrip PASS (Opcode: "
                  << static_cast<int>(server_reply.opcode) << ", Payload: " << server_reply.payload << ")\n";
    }, 5);

    std::cout << "\n🎉 ALL WEBSOCKET END-TO-END TESTS PASSED PERFECTLY!\n";
    return 0;
}
