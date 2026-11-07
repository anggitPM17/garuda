// =====================================================================
// GARUDA COMPILER — FASE 11 WEBSOCKET PERMESSAGE-DEFLATE TEST SUITE (RED -> GREEN)
// =====================================================================

#include "../include/garuda/beam/tools/script_engine.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <cstdlib>

static socket_t ws_connect_deflate(int port) {
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
    std::cout << "GARUDA COMPILER — FASE 11 WEBSOCKET PERMESSAGE-DEFLATE TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Start WebSocket Server on port 9097
    engine.eval("modul TWsDeflate fungsi utama() -> Server.mulai_ws_deflate(9097, nil, 100) rampung");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // Test 1: Handshake negotiation with Sec-WebSocket-Extensions: permessage-deflate
    socket_t c1 = ws_connect_deflate(9097);
    bool nego_ok = false;
    if (c1 != INVALID_SOCKET_VAL) {
        std::string hs = "GET /ws HTTP/1.1\r\nHost: localhost:9097\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\nSec-WebSocket-Extensions: permessage-deflate\r\n\r\n";
        send(c1, hs.data(), static_cast<int>(hs.length()), 0);
        char hbuf[1024] = {0};
        int r = recv(c1, hbuf, 1023, 0);
        CLOSE_SOCKET(c1);
        if (r > 0 && std::string(hbuf, r).find("Sec-WebSocket-Extensions: permessage-deflate") != std::string::npos) {
            nego_ok = true;
        }
    }

    if (nego_ok) {
        std::cout << "[Test 1] WebSocket Handshake Negotiated Sec-WebSocket-Extensions: permessage-deflate [PASS]\n";
    } else {
        std::cout << "[Test 1] WebSocket Handshake Negotiation -> [FAIL] Expected permessage-deflate extension header\n";
        failures++;
    }

    // Test 2: Send 10KB text payload with deflate extension -> compressed frame returned
    socket_t c2 = ws_connect_deflate(9097);
    bool text_deflate_ok = false;
    if (c2 != INVALID_SOCKET_VAL) {
        std::string hs = "GET /ws HTTP/1.1\r\nHost: localhost:9097\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\nSec-WebSocket-Extensions: permessage-deflate\r\n\r\n";
        send(c2, hs.data(), static_cast<int>(hs.length()), 0);
        char hbuf[1024] = {0};
        recv(c2, hbuf, 1023, 0);

        // Send uncompressed frame to server, server compresses response with permessage-deflate
        std::string text_10k = "HELLO_DEFLATE_" + std::string(10240, 'A');
        std::string frame;
        frame.push_back(static_cast<char>(0x81)); // FIN + Text
        uint16_t len = static_cast<uint16_t>(text_10k.length());
        frame.push_back(static_cast<char>(0x80 | 126)); // Masked + 16-bit len
        frame.push_back(static_cast<char>((len >> 8) & 0xFF));
        frame.push_back(static_cast<char>(len & 0xFF));
        uint8_t mask[4] = {0x11, 0x22, 0x33, 0x44};
        for (int i = 0; i < 4; ++i) frame.push_back(static_cast<char>(mask[i]));
        for (size_t i = 0; i < text_10k.length(); ++i) {
            frame.push_back(text_10k[i] ^ mask[i % 4]);
        }

        send(c2, frame.data(), static_cast<int>(frame.length()), 0);
        char rbuf[4096] = {0};
        int r = recv(c2, rbuf, 4095, 0);
        CLOSE_SOCKET(c2);
        // Server response should have RSV1 bit (0x40) set for compressed payload
        if (r > 0 && (static_cast<uint8_t>(rbuf[0]) & 0x40) != 0) {
            text_deflate_ok = true;
        }
    }

    if (text_deflate_ok) {
        std::cout << "[Test 2] 10KB Text Frame Deflate Compressed Response (RSV1=1) [PASS]\n";
    } else {
        std::cout << "[Test 2] 10KB Text Frame Deflate -> [FAIL] Expected RSV1=1 compressed frame\n";
        failures++;
    }

    // Test 3: Send 50KB JSON payload -> compressed frame size < 50% of raw payload
    socket_t c3 = ws_connect_deflate(9097);
    bool ratio_ok = false;
    if (c3 != INVALID_SOCKET_VAL) {
        std::string hs = "GET /ws HTTP/1.1\r\nHost: localhost:9097\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\nSec-WebSocket-Extensions: permessage-deflate\r\n\r\n";
        send(c3, hs.data(), static_cast<int>(hs.length()), 0);
        char hbuf[1024] = {0};
        recv(c3, hbuf, 1023, 0);

        std::string json_50k = "{\"data\":\"" + std::string(50000, 'X') + "\"}";
        std::string frame;
        frame.push_back(static_cast<char>(0x81));
        uint32_t len = static_cast<uint32_t>(json_50k.length());
        frame.push_back(static_cast<char>(0x80 | 126));
        frame.push_back(static_cast<char>((len >> 8) & 0xFF));
        frame.push_back(static_cast<char>(len & 0xFF));
        uint8_t mask[4] = {0x55, 0x66, 0x77, 0x88};
        for (int i = 0; i < 4; ++i) frame.push_back(static_cast<char>(mask[i]));
        for (size_t i = 0; i < json_50k.length(); ++i) {
            frame.push_back(json_50k[i] ^ mask[i % 4]);
        }

        send(c3, frame.data(), static_cast<int>(frame.length()), 0);
        char rbuf[10000] = {0};
        int r = recv(c3, rbuf, 9999, 0);
        CLOSE_SOCKET(c3);
        // Assert returned compressed frame size is < 25KB (< 50% of 50KB)
        if (r > 0 && r < 25000) {
            ratio_ok = true;
        }
    }

    if (ratio_ok) {
        std::cout << "[Test 3] 50KB JSON Payload Compression Ratio < 50% [PASS]\n";
    } else {
        std::cout << "[Test 3] 50KB JSON Payload Compression Ratio -> [FAIL] Compression ratio requirement not met\n";
        failures++;
    }

    // Test 4: 100KB Binary payload permessage-deflate compressed frame
    socket_t c4 = ws_connect_deflate(9097);
    bool bin_deflate_ok = false;
    if (c4 != INVALID_SOCKET_VAL) {
        std::string hs = "GET /ws HTTP/1.1\r\nHost: localhost:9097\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\nSec-WebSocket-Extensions: permessage-deflate\r\n\r\n";
        send(c4, hs.data(), static_cast<int>(hs.length()), 0);
        char hbuf[1024] = {0};
        recv(c4, hbuf, 1023, 0);

        std::string bin_100k = std::string(100000, 'Z');
        std::string frame;
        frame.push_back(static_cast<char>(0x82)); // Binary frame
        uint32_t len = static_cast<uint32_t>(bin_100k.length());
        frame.push_back(static_cast<char>(0x80 | 126));
        frame.push_back(static_cast<char>((len >> 8) & 0xFF));
        frame.push_back(static_cast<char>(len & 0xFF));
        uint8_t mask[4] = {0xAA, 0xBB, 0xCC, 0xDD};
        for (int i = 0; i < 4; ++i) frame.push_back(static_cast<char>(mask[i]));
        for (size_t i = 0; i < bin_100k.length(); ++i) {
            frame.push_back(bin_100k[i] ^ mask[i % 4]);
        }

        send(c4, frame.data(), static_cast<int>(frame.length()), 0);
        char rbuf[10000] = {0};
        int r = recv(c4, rbuf, 9999, 0);
        CLOSE_SOCKET(c4);
        if (r > 0 && (static_cast<uint8_t>(rbuf[0]) & 0x40) != 0) {
            bin_deflate_ok = true;
        }
    }

    if (bin_deflate_ok) {
        std::cout << "[Test 4] 100KB Binary Frame Deflate Compressed Response [PASS]\n";
    } else {
        std::cout << "[Test 4] 100KB Binary Frame Deflate -> [FAIL] Expected RSV1=1 compressed frame\n";
        failures++;
    }

    // Test 5: Disabled Deflate when client does NOT request permessage-deflate header
    socket_t c5 = ws_connect_deflate(9097);
    bool disabled_deflate_ok = false;
    if (c5 != INVALID_SOCKET_VAL) {
        std::string hs = "GET /ws HTTP/1.1\r\nHost: localhost:9097\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n\r\n";
        send(c5, hs.data(), static_cast<int>(hs.length()), 0);
        char hbuf[1024] = {0};
        int r_hs = recv(c5, hbuf, 1023, 0);
        if (r_hs > 0 && std::string(hbuf, r_hs).find("Sec-WebSocket-Extensions") == std::string::npos) {
            std::string msg = "HELLO_NO_DEFLATE";
            std::string frame;
            frame.push_back(static_cast<char>(0x81));
            frame.push_back(static_cast<char>(0x80 | msg.length()));
            uint8_t mask[4] = {0x01, 0x02, 0x03, 0x04};
            for (int i = 0; i < 4; ++i) frame.push_back(static_cast<char>(mask[i]));
            for (size_t i = 0; i < msg.length(); ++i) {
                frame.push_back(msg[i] ^ mask[i % 4]);
            }
            send(c5, frame.data(), static_cast<int>(frame.length()), 0);
            char rbuf[1024] = {0};
            int r_ws = recv(c5, rbuf, 1023, 0);
            CLOSE_SOCKET(c5);
            // Response should NOT have RSV1 bit (RSV1=0)
            if (r_ws > 0 && (static_cast<uint8_t>(rbuf[0]) & 0x40) == 0) {
                disabled_deflate_ok = true;
            }
        }
    }

    if (disabled_deflate_ok) {
        std::cout << "[Test 5] Disabled Deflate Fallback when Extension Header Omitted [PASS]\n";
    } else {
        std::cout << "[Test 5] Disabled Deflate Fallback -> [FAIL] Expected uncompressed frame (RSV1=0)\n";
        failures++;
    }

    if (failures > 0) {
        std::cout << "\n❌ " << failures << "/5 TESTS FAILED ON STUB CODE!\n";
        return 1;
    }

    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL 5 FASE 11 WEBSOCKET PERMESSAGE-DEFLATE TESTS PASSED PERFECTLY!\n";
    std::cout << "========================================\n";
    return 0;
}
