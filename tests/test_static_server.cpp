// =====================================================================
// GARUDA COMPILER — FASE 10 STATIC FILE SERVER TEST SUITE (5 TESTS)
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
    std::cout << "GARUDA COMPILER — FASE 10 STATIC FILE SERVER TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test 1: Server.mulai_async_statis Initialization
    std::string code1 = R"(
        modul TestSTC1
        fungsi utama() ->
            Server.mulai_async_statis(9093, "site/")
        rampung
    )";
    auto res1 = engine.eval(code1);
    CHECK(res1.has_value(), "Test 1 must return result");
    auto* a1 = res1->get_if<garuda::beam::Atom>();
    CHECK(a1 != nullptr && a1->name == "benar", "Test 1 Server.mulai_async_statis return :benar");
    std::cout << "[Test 1] Server.mulai_async_statis Initialization: " << a1->name << " [PASS]\n";

    // Test 2: Valid Static File Reading
    std::string code2 = R"(
        modul TestSTC2
        fungsi utama() ->
            File.baca_statis("index.html")
        rampung
    )";
    auto res2 = engine.eval(code2);
    CHECK(res2.has_value(), "Test 2 must return result");
    auto* a2 = res2->get_if<garuda::beam::Atom>();
    CHECK(a2 != nullptr && a2->name == "sukses", "Test 2 File.baca_statis valid path");
    std::cout << "[Test 2] Valid Static File Reading: " << a2->name << " [PASS]\n";

    // Test 3: Path Traversal Protection Rejection
    std::string code3 = R"(
        modul TestSTC3
        fungsi utama() ->
            File.baca_statis("../etc/passwd")
        rampung
    )";
    auto res3 = engine.eval(code3);
    CHECK(res3.has_value(), "Test 3 must return result");
    auto* a3 = res3->get_if<garuda::beam::Atom>();
    CHECK(a3 != nullptr && a3->name == "dilarang", "Test 3 path traversal rejected");
    std::cout << "[Test 3] Path Traversal Rejection ('../etc/passwd'): " << a3->name << " [PASS]\n";

    // Test 4: Nested Path Traversal Protection
    std::string code4 = R"(
        modul TestSTC4
        fungsi utama() ->
            File.baca_statis("site/../../secret")
        rampung
    )";
    auto res4 = engine.eval(code4);
    CHECK(res4.has_value(), "Test 4 must return result");
    auto* a4 = res4->get_if<garuda::beam::Atom>();
    CHECK(a4 != nullptr && a4->name == "dilarang", "Test 4 nested traversal rejected");
    std::cout << "[Test 4] Nested Path Traversal Rejection: " << a4->name << " [PASS]\n";

    // Test 5: Static Directory Validation
    std::string code5 = R"(
        modul TestSTC5
        fungsi utama() ->
            Server.mulai_async_statis(9094, "public/")
        rampung
    )";
    auto res5 = engine.eval(code5);
    CHECK(res5.has_value(), "Test 5 must return result");
    auto* a5 = res5->get_if<garuda::beam::Atom>();
    CHECK(a5 != nullptr && a5->name == "benar", "Test 5 public/ dir validation");
    std::cout << "[Test 5] Static Directory Validation ('public/'): " << a5->name << " [PASS]\n";

    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL 5 FASE 10 STATIC FILE SERVER TESTS PASSED PERFECTLY!\n";
    std::cout << "========================================\n";
    return 0;
}
