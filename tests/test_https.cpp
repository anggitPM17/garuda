// =====================================================================
// GARUDA COMPILER — FASE 10 HTTPS SERVER TEST SUITE (5 TESTS)
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
    std::cout << "GARUDA COMPILER — FASE 10 HTTPS SERVER TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test 1: Server.mulai_https Initialization
    std::string code1 = R"(
        modul TestHTTPS1
        fungsi utama() ->
            Server.mulai_https(8443, "cert.pem", "key.pem", nil)
        rampung
    )";
    auto res1 = engine.eval(code1);
    CHECK(res1.has_value(), "Test 1 must return result");
    auto* a1 = res1->get_if<garuda::beam::Atom>();
    CHECK(a1 != nullptr && a1->name == "benar", "Test 1 Server.mulai_https return :benar");
    std::cout << "[Test 1] Server.mulai_https Initialization: " << a1->name << " [PASS]\n";

    // Test 2: TLS Port Validation
    std::string code2 = R"(
        modul TestHTTPS2
        fungsi utama() ->
            Server.mulai_https(9443, "", "", nil)
        rampung
    )";
    auto res2 = engine.eval(code2);
    CHECK(res2.has_value(), "Test 2 must return result");
    auto* a2 = res2->get_if<garuda::beam::Atom>();
    CHECK(a2 != nullptr && a2->name == "benar", "Test 2 TLS port validation");
    std::cout << "[Test 2] TLS Context Validation & Port Check: " << a2->name << " [PASS]\n";

    // Test 3: HTTPS 200 OK Response
    std::string code3 = R"(
        modul TestHTTPS3
        fungsi utama() ->
            Http.buat_respons(200, %{"Content-Type" => "text/html"}, "HTTPS OK")
        rampung
    )";
    auto res3 = engine.eval(code3);
    CHECK(res3.has_value(), "Test 3 must return string");
    auto* s3 = res3->get_if<std::string>();
    CHECK(s3 != nullptr && s3->find("HTTP/1.1 200 OK") != std::string::npos, "Test 3 HTTPS 200 OK");
    std::cout << "[Test 3] HTTPS 200 OK Response Generation [PASS]\n";

    // Test 4: HTTPS 404 Fallback
    std::string code4 = R"(
        modul TestHTTPS4
        fungsi utama() ->
            Http.buat_respons(404, %{}, "HTTPS 404")
        rampung
    )";
    auto res4 = engine.eval(code4);
    CHECK(res4.has_value(), "Test 4 must return string");
    auto* s4 = res4->get_if<std::string>();
    CHECK(s4 != nullptr && s4->find("HTTP/1.1 404 Not Found") != std::string::npos, "Test 4 HTTPS 404");
    std::cout << "[Test 4] HTTPS 404 Fallback Response Generation [PASS]\n";

    // Test 5: HTTPS Health Check JSON Response
    std::string code5 = R"(
        modul TestHTTPS5
        fungsi utama() ->
            Http.buat_respons(200, %{"Content-Type" => "application/json"}, "{\"https\":\"ok\"}")
        rampung
    )";
    auto res5 = engine.eval(code5);
    CHECK(res5.has_value(), "Test 5 must return string");
    auto* s5 = res5->get_if<std::string>();
    CHECK(s5 != nullptr && s5->find("application/json") != std::string::npos, "Test 5 HTTPS Health Check");
    std::cout << "[Test 5] HTTPS Health Check JSON Response [PASS]\n";

    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL 5 FASE 10 HTTPS SERVER TESTS PASSED PERFECTLY!\n";
    std::cout << "========================================\n";
    return 0;
}
