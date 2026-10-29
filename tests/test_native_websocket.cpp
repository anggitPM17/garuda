// =====================================================================
// FASE D.4.1 - RICHAT NATIVE WEBSOCKET TESTS
// Memverifikasi native_websocket.h & native_websocket.cpp
// =====================================================================
#include "../android/app/src/main/cpp/native_websocket.h"
#include <iostream>
#include <string>
#include <cstdlib>

using namespace richat::native;

#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

int main() {
    std::cout << "========================================\n";
    std::cout << "FASE D.4.1 - NATIVE WEBSOCKET TESTS\n";
    std::cout << "========================================\n\n";

    NativeWebSocket ws;

    // Test 1: Connect WebSocket
    std::cout << "[Test 1] Connect WebSocket...\n";
    {
        bool opened = false;
        bool connected = ws.connect("wss://richat.my.id/",
            [&]() { opened = true; },
            nullptr, nullptr, nullptr);

        CHECK(connected, "connect() harus return true");
        CHECK(opened, "on_open callback harus dipanggil");
        CHECK(ws.is_connected(), "is_connected() harus true");
    }
    std::cout << "PASS\n\n";

    // Test 2: Send & Ping/Pong Echo
    std::cout << "[Test 2] Send & Ping/Pong Echo...\n";
    {
        std::string rec_msg;
        NativeWebSocket ws2;
        ws2.connect("wss://richat.my.id/", nullptr,
            [&](const std::string& msg) { rec_msg = msg; },
            nullptr, nullptr);

        ws2.send("{\"type\":\"ping\"}");
        CHECK(rec_msg == "{\"type\":\"pong\"}", "Ping harus dibalas dengan pong");
    }
    std::cout << "PASS\n\n";

    // Test 3: Multiple Connect Rejection
    std::cout << "[Test 3] Multiple Connect Rejection...\n";
    {
        bool double_conn = ws.connect("wss://richat.my.id/", nullptr, nullptr, nullptr, nullptr);
        CHECK(!double_conn, "Koneksi ke-2 harus ditolak (return false)");
    }
    std::cout << "PASS\n\n";

    // Test 4: Close Connection
    std::cout << "[Test 4] Close Connection...\n";
    {
        bool closed = false;
        NativeWebSocket ws3;
        ws3.connect("wss://richat.my.id/", nullptr, nullptr,
            [&](int code, const std::string&) { if (code == 1000) closed = true; },
            nullptr);

        ws3.close(1000, "Normal close");
        CHECK(!ws3.is_connected(), "is_connected() harus false setelah close");
        CHECK(closed, "on_close callback harus dipanggil dengan code 1000");
    }
    std::cout << "PASS\n\n";

    // Test 5: Send after Close Rejection
    std::cout << "[Test 5] Send after Close Rejection...\n";
    {
        NativeWebSocket ws4;
        ws4.connect("wss://richat.my.id/", nullptr, nullptr, nullptr, nullptr);
        ws4.close();
        bool ok = ws4.send("Hello");
        CHECK(!ok, "send() setelah close harus return false");
    }
    std::cout << "PASS\n\n";

    std::cout << "========================================\n";
    std::cout << "FASE D.4.1 - NATIVE WEBSOCKET SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
