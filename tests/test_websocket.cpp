// =====================================================================
// GARUDA COMPILER — FASE 10 WEBSOCKET SERVER TEST SUITE (5 TESTS)
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

int main() {
    garuda::beam::ScriptEngine engine;

    std::cout << "=======================================================\n";
    std::cout << "GARUDA COMPILER — FASE 10 WEBSOCKET SERVER TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test 1: RFC 6455 Sec-WebSocket-Accept Test Vector
    // Key: "dGhlIHNhbXBsZSBub25jZQ==" -> Accept: "s3pPLMBiTxaQ9kYGzzhZRbK+xOo="
    std::string code1 = R"(
        modul TestWS1
        fungsi utama() ->
            Ws.upgrade("dGhlIHNhbXBsZSBub25jZQ==")
        rampung
    )";
    auto res1 = engine.eval(code1);
    CHECK(res1.has_value(), "Test 1 must return accept key");
    auto* s1 = res1->get_if<std::string>();
    CHECK(s1 != nullptr && *s1 == "s3pPLMBiTxaQ9kYGzzhZRbK+xOo=", "Test 1 RFC 6455 accept key match");
    std::cout << "[Test 1] RFC 6455 Sec-WebSocket-Accept: " << *s1 << " [PASS]\n";

    // Test 2: Masked Frame Unmasking (Client -> Server "Hello")
    // Mask key: 0x37, 0xfa, 0x21, 0x3d. Masked payload for "Hello": 0x7f, 0x9f, 0x4d, 0x51, 0x58
    std::string masked_hello;
    masked_hello.push_back(static_cast<char>(0x81)); // FIN + Text
    masked_hello.push_back(static_cast<char>(0x85)); // Masked + Len 5
    masked_hello.push_back(static_cast<char>(0x37)); // Mask 0
    masked_hello.push_back(static_cast<char>(0xfa)); // Mask 1
    masked_hello.push_back(static_cast<char>(0x21)); // Mask 2
    masked_hello.push_back(static_cast<char>(0x3d)); // Mask 3
    masked_hello.push_back(static_cast<char>(0x7f));
    masked_hello.push_back(static_cast<char>(0x9f));
    masked_hello.push_back(static_cast<char>(0x4d));
    masked_hello.push_back(static_cast<char>(0x51));
    masked_hello.push_back(static_cast<char>(0x58));

    std::string code2 = R"(
        modul TestWS2
        fungsi utama(RawFrame) ->
            Ws.baca(RawFrame)
        rampung
    )";
    auto res2 = engine.eval(code2, {garuda::beam::Term(masked_hello)});
    CHECK(res2.has_value(), "Test 2 must return unmasked string");
    auto* s2 = res2->get_if<std::string>();
    CHECK(s2 != nullptr && *s2 == "Hello", "Test 2 unmasked string must be 'Hello'");
    std::cout << "[Test 2] WebSocket Masked Frame Unmasking: " << *s2 << " [PASS]\n";

    // Test 3: Unmasked Text Frame Encoding (Server -> Client)
    std::string code3 = R"(
        modul TestWS3
        fungsi utama() ->
            Ws.kirim(999, "PING_WS")
        rampung
    )";
    auto res3 = engine.eval(code3);
    CHECK(res3.has_value(), "Test 3 must return result");
    auto* a3 = res3->get_if<garuda::beam::Atom>();
    CHECK(a3 != nullptr, "Test 3 Ws.kirim return atom");
    std::cout << "[Test 3] WebSocket Frame Encoding: " << a3->name << " [PASS]\n";

    // Test 4: WebSocket Close Frame Handling
    std::string code4 = R"(
        modul TestWS4
        fungsi utama() ->
            Ws.tutup(999)
        rampung
    )";
    auto res4 = engine.eval(code4);
    CHECK(res4.has_value(), "Test 4 must return result");
    auto* a4 = res4->get_if<garuda::beam::Atom>();
    CHECK(a4 != nullptr && a4->name == "benar", "Test 4 Ws.tutup return :benar");
    std::cout << "[Test 4] WebSocket Close Frame Handling: " << a4->name << " [PASS]\n";

    // Test 5: Server.mulai_ws Initialization
    std::string code5 = R"(
        modul TestWS5
        fungsi utama() ->
            Server.mulai_ws(9092, nil)
        rampung
    )";
    auto res5 = engine.eval(code5);
    CHECK(res5.has_value(), "Test 5 must return result");
    auto* a5 = res5->get_if<garuda::beam::Atom>();
    CHECK(a5 != nullptr && a5->name == "benar", "Test 5 Server.mulai_ws return :benar");
    std::cout << "[Test 5] Server.mulai_ws Initialization: " << a5->name << " [PASS]\n";

    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL 5 FASE 10 WEBSOCKET SERVER TESTS PASSED PERFECTLY!\n";
    std::cout << "========================================\n";
    return 0;
}
