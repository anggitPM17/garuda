// =====================================================================
// GARUDA COMPILER — FASE 7 MULTIPLE ASSIGNMENT TEST SUITE (5 TESTS)
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
    std::cout << "GARUDA COMPILER — FASE 7 MULTIPLE ASSIGNMENT TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test 1: Basic Multiple Assignment `A, B = 10, 20`
    std::string code1 = R"(
        modul TestMA1
        fungsi utama() ->
            A, B = 10, 20
            A + B
        rampung
    )";
    auto res1 = engine.eval(code1);
    CHECK(res1.has_value(), "Test 1 must return value");
    auto* i1 = res1->get_if<std::int64_t>();
    CHECK(i1 != nullptr && *i1 == 30, "Test 1 result must be 30");
    std::cout << "[Test 1] A, B = 10, 20: " << *i1 << " [PASS]\n";

    // Test 2: Atomic Variable Swap `A, B = B, A`
    std::string code2 = R"(
        modul TestMA2
        fungsi utama() ->
            A, B = 10, 20
            A, B = B, A
            A
        rampung
    )";
    auto res2 = engine.eval(code2);
    CHECK(res2.has_value(), "Test 2 must return value");
    auto* i2 = res2->get_if<std::int64_t>();
    CHECK(i2 != nullptr && *i2 == 20, "Test 2 result must be 20");
    std::cout << "[Test 2] Atomic Variable Swap A, B = B, A: " << *i2 << " [PASS]\n";

    // Test 3: Three Variable Assignment `A, B, C = 10, 20, 30`
    std::string code3 = R"(
        modul TestMA3
        fungsi utama() ->
            A, B, C = 10, 20, 30
            A + B + C
        rampung
    )";
    auto res3 = engine.eval(code3);
    CHECK(res3.has_value(), "Test 3 must return value");
    auto* i3 = res3->get_if<std::int64_t>();
    CHECK(i3 != nullptr && *i3 == 60, "Test 3 result must be 60");
    std::cout << "[Test 3] A, B, C = 10, 20, 30: " << *i3 << " [PASS]\n";

    // Test 4: Tuple Destructuring to Multiple Targets `A, B = {10, 20}`
    std::string code4 = R"(
        modul TestMA4
        fungsi utama() ->
            A, B = {10, 20}
            A + B
        rampung
    )";
    auto res4 = engine.eval(code4);
    CHECK(res4.has_value(), "Test 4 must return value");
    auto* i4 = res4->get_if<std::int64_t>();
    CHECK(i4 != nullptr && *i4 == 30, "Test 4 result must be 30");
    std::cout << "[Test 4] Tuple to Multiple Targets A, B = {10, 20}: " << *i4 << " [PASS]\n";

    // Test 5: Atomic Multiple Expression Evaluation `X, Y = Y + 5, X + 15`
    std::string code5 = R"(
        modul TestMA5
        fungsi utama() ->
            X, Y = 10, 20
            X, Y = Y + 5, X + 15
            X + Y
        rampung
    )";
    auto res5 = engine.eval(code5);
    CHECK(res5.has_value(), "Test 5 must return value");
    auto* i5 = res5->get_if<std::int64_t>();
    CHECK(i5 != nullptr && *i5 == 50, "Test 5 result must be 50");
    std::cout << "[Test 5] Atomic Expression Evaluation X, Y = Y + 5, X + 15: " << *i5 << " [PASS]\n";

    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL 5 FASE 7 MULTIPLE ASSIGNMENT TESTS PASSED PERFECTLY!\n";
    std::cout << "========================================\n";
    return 0;
}
