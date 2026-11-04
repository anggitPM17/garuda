// =====================================================================
// GARUDA COMPILER — FASE 7 STRING FORMAT SPECIFIERS TEST SUITE (5 TESTS)
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
    std::cout << "GARUDA COMPILER — FASE 7 STRING FORMAT SPECIFIERS TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test 1: Zero-Padded Integer Width `X = 42; "#{X:05d}"`
    std::string code1 = R"(
        modul TestSF1
        fungsi utama() ->
            X = 42
            "#{X:05d}"
        rampung
    )";
    auto res1 = engine.eval(code1);
    CHECK(res1.has_value(), "Test 1 must return value");
    auto* s1 = res1->get_if<std::string>();
    CHECK(s1 != nullptr && *s1 == "00042", "Test 1 result must be '00042'");
    std::cout << "[Test 1] Zero-padded int #{X:05d}: " << *s1 << " [PASS]\n";

    // Test 2: Fixed Decimal Float Precision `Y = 3.14159; "#{Y:.2f}"`
    std::string code2 = R"(
        modul TestSF2
        fungsi utama() ->
            Y = 3.14159
            "#{Y:.2f}"
        rampung
    )";
    auto res2 = engine.eval(code2);
    CHECK(res2.has_value(), "Test 2 must return value");
    auto* s2 = res2->get_if<std::string>();
    CHECK(s2 != nullptr && *s2 == "3.14", "Test 2 result must be '3.14'");
    std::cout << "[Test 2] Fixed decimal float #{Y:.2f}: " << *s2 << " [PASS]\n";

    // Test 3: Right-Aligned String Width `S = "hi"; "#{S:>5}"`
    std::string code3 = R"(
        modul TestSF3
        fungsi utama() ->
            S = "hi"
            "#{S:>5}"
        rampung
    )";
    auto res3 = engine.eval(code3);
    CHECK(res3.has_value(), "Test 3 must return value");
    auto* s3 = res3->get_if<std::string>();
    CHECK(s3 != nullptr && *s3 == "   hi", "Test 3 result must be '   hi'");
    std::cout << "[Test 3] Right-aligned string #{S:>5}: '" << *s3 << "' [PASS]\n";

    // Test 4: Hexadecimal Format `X = 255; "#{X:x}"`
    std::string code4 = R"(
        modul TestSF4
        fungsi utama() ->
            X = 255
            "#{X:x}"
        rampung
    )";
    auto res4 = engine.eval(code4);
    CHECK(res4.has_value(), "Test 4 must return value");
    auto* s4 = res4->get_if<std::string>();
    CHECK(s4 != nullptr && *s4 == "ff", "Test 4 result must be 'ff'");
    std::cout << "[Test 4] Hexadecimal #{X:x}: " << *s4 << " [PASS]\n";

    // Test 5: Left-Aligned String Width `S = "hi"; "#{S:<5}"`
    std::string code5 = R"(
        modul TestSF5
        fungsi utama() ->
            S = "hi"
            "#{S:<5}"
        rampung
    )";
    auto res5 = engine.eval(code5);
    CHECK(res5.has_value(), "Test 5 must return value");
    auto* s5 = res5->get_if<std::string>();
    CHECK(s5 != nullptr && *s5 == "hi   ", "Test 5 result must be 'hi   '");
    std::cout << "[Test 5] Left-aligned string #{S:<5}: '" << *s5 << "' [PASS]\n";

    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL 5 FASE 7 STRING FORMAT SPECIFIERS TESTS PASSED PERFECTLY!\n";
    std::cout << "========================================\n";
    return 0;
}
