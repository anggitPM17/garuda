// =====================================================================
// GARUDA COMPILER — FASE 8 HTTP REQUEST PARSER TEST SUITE (5 TESTS)
// REBUILD TRIGGER TIMESTAMP: 2026-11-05 08:15:00
// =====================================================================

#include "../include/garuda/beam/tools/script_engine.hpp"
#include <iostream>
#include <string>
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
    std::cout << "GARUDA COMPILER — FASE 8 HTTP REQUEST PARSER TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test 1: Standard GET Request Parsing
    std::string code1 = R"(
        modul TestRP1
        fungsi utama() ->
            Req = Http.parse_request("GET /api/users HTTP/1.1\r\nHost: localhost\r\n\r\n")
            Map.ambil(Req, "metode")
        rampung
    )";
    auto res1 = engine.eval(code1);
    CHECK(res1.has_value(), "Test 1 must return value");
    auto* s1 = res1->get_if<std::string>();
    CHECK(s1 != nullptr && *s1 == "GET", "Test 1 method must be 'GET'");
    std::cout << "[Test 1] GET /api/users method: " << *s1 << " [PASS]\n";

    // Test 2: POST Request with Body
    std::string code2 = R"(
        modul TestRP2
        fungsi utama() ->
            Req = Http.parse_request("POST /echo HTTP/1.1\r\nContent-Type: text/plain\r\n\r\nHello World")
            Map.ambil(Req, "body")
        rampung
    )";
    auto res2 = engine.eval(code2);
    CHECK(res2.has_value(), "Test 2 must return value");
    auto* s2 = res2->get_if<std::string>();
    CHECK(s2 != nullptr && *s2 == "Hello World", "Test 2 body must be 'Hello World'");
    std::cout << "[Test 2] POST body: " << *s2 << " [PASS]\n";

    // Test 3: Multi-header Extraction
    std::string code3 = R"(
        modul TestRP3
        fungsi utama() ->
            Req = Http.parse_request("POST /echo HTTP/1.1\r\nHost: localhost\r\nContent-Type: text/plain\r\n\r\nHello World")
            H = Map.ambil(Req, "headers")
            Map.ambil(H, "Content-Type")
        rampung
    )";
    auto res3 = engine.eval(code3);
    CHECK(res3.has_value(), "Test 3 must return value");
    auto* s3 = res3->get_if<std::string>();
    CHECK(s3 != nullptr && *s3 == "text/plain", "Test 3 Content-Type header must be 'text/plain'");
    std::cout << "[Test 3] Header Content-Type: " << *s3 << " [PASS]\n";

    // Test 4: Default Fallback Path
    std::string code4 = R"(
        modul TestRP4
        fungsi utama() ->
            Req = Http.parse_request("GET / HTTP/1.1\r\n\r\n")
            Map.ambil(Req, "path")
        rampung
    )";
    auto res4 = engine.eval(code4);
    CHECK(res4.has_value(), "Test 4 must return value");
    auto* s4 = res4->get_if<std::string>();
    CHECK(s4 != nullptr && *s4 == "/", "Test 4 path must be '/'");
    std::cout << "[Test 4] Path /: " << *s4 << " [PASS]\n";

    // Test 5: Malformed Request Handling
    std::string code5 = R"(
        modul TestRP5
        fungsi utama() ->
            Req = Http.parse_request("")
            Map.ambil(Req, "path")
        rampung
    )";
    auto res5 = engine.eval(code5);
    CHECK(res5.has_value(), "Test 5 must return value");
    auto* s5 = res5->get_if<std::string>();
    CHECK(s5 != nullptr && *s5 == "/", "Test 5 path must fallback to '/'");
    std::cout << "[Test 5] Malformed request fallback path: " << *s5 << " [PASS]\n";

    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL 5 FASE 8 HTTP REQUEST PARSER TESTS PASSED PERFECTLY!\n";
    std::cout << "========================================\n";
    return 0;
}
