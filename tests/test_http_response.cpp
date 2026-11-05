// =====================================================================
// GARUDA COMPILER — FASE 8 HTTP RESPONSE BUILDER TEST SUITE (5 TESTS)
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
    std::cout << "GARUDA COMPILER — FASE 8 HTTP RESPONSE BUILDER TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test 1: Standard 200 OK Response
    std::string code1 = R"(
        modul TestRES1
        fungsi utama() ->
            Http.buat_respons(200, %{"Content-Type" => "text/plain"}, "Hello World")
        rampung
    )";
    auto res1 = engine.eval(code1);
    CHECK(res1.has_value(), "Test 1 must return value");
    auto* s1 = res1->get_if<std::string>();
    CHECK(s1 != nullptr, "Test 1 response must be string");
    CHECK(s1->find("HTTP/1.1 200 OK") != std::string::npos, "Test 1 must contain 200 OK");
    CHECK(s1->find("Content-Length: 11") != std::string::npos, "Test 1 must contain Content-Length: 11");
    CHECK(s1->find("Hello World") != std::string::npos, "Test 1 must contain body");
    std::cout << "[Test 1] 200 OK Response: " << *s1 << " [PASS]\n";

    // Test 2: 404 Not Found Response
    std::string code2 = R"(
        modul TestRES2
        fungsi utama() ->
            Http.buat_respons(404, %{}, "Not Found")
        rampung
    )";
    auto res2 = engine.eval(code2);
    CHECK(res2.has_value(), "Test 2 must return value");
    auto* s2 = res2->get_if<std::string>();
    CHECK(s2 != nullptr && s2->find("HTTP/1.1 404 Not Found") != std::string::npos, "Test 2 must be 404");
    std::cout << "[Test 2] 404 Not Found Response [PASS]\n";

    // Test 3: 500 Internal Server Error Response with JSON
    std::string code3 = R"(
        modul TestRES3
        fungsi utama() ->
            Http.buat_respons(500, %{"Content-Type" => "application/json"}, "{\"error\":\"fail\"}")
        rampung
    )";
    auto res3 = engine.eval(code3);
    CHECK(res3.has_value(), "Test 3 must return value");
    auto* s3 = res3->get_if<std::string>();
    CHECK(s3 != nullptr && s3->find("Content-Type: application/json") != std::string::npos, "Test 3 JSON header");
    std::cout << "[Test 3] 500 JSON Response [PASS]\n";

    // Test 4: Multiple Custom Headers
    std::string code4 = R"(
        modul TestRES4
        fungsi utama() ->
            Http.buat_respons(200, %{"Server" => "Garuda", "X-Custom" => "123"}, "OK")
        rampung
    )";
    auto res4 = engine.eval(code4);
    CHECK(res4.has_value(), "Test 4 must return value");
    auto* s4 = res4->get_if<std::string>();
    CHECK(s4 != nullptr && s4->find("Server: Garuda") != std::string::npos, "Test 4 Server header");
    CHECK(s4 != nullptr && s4->find("X-Custom: 123") != std::string::npos, "Test 4 X-Custom header");
    std::cout << "[Test 4] Custom Headers [PASS]\n";

    // Test 5: Empty Body Response 204 No Content
    std::string code5 = R"(
        modul TestRES5
        fungsi utama() ->
            Http.buat_respons(204, %{}, "")
        rampung
    )";
    auto res5 = engine.eval(code5);
    CHECK(res5.has_value(), "Test 5 must return value");
    auto* s5 = res5->get_if<std::string>();
    CHECK(s5 != nullptr && s5->find("HTTP/1.1 204 No Content") != std::string::npos, "Test 5 204 No Content");
    CHECK(s5 != nullptr && s5->find("Content-Length: 0") != std::string::npos, "Test 5 Content-Length: 0");
    std::cout << "[Test 5] 204 No Content Empty Body [PASS]\n";

    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL 5 FASE 8 HTTP RESPONSE BUILDER TESTS PASSED PERFECTLY!\n";
    std::cout << "========================================\n";
    return 0;
}
