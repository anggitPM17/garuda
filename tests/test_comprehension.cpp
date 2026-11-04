// =====================================================================
// GARUDA COMPILER — FASE 6 LIST COMPREHENSION TEST SUITE (5 TESTS)
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
    std::cout << "GARUDA COMPILER — FASE 6 LIST COMPREHENSION TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test 1: Basic List Comprehension [X * 2 untuk X ing [1, 2, 3]]
    std::string code1 = R"(
        modul TestC1
        fungsi utama() ->
            [X * 2 untuk X ing [1, 2, 3]]
        rampung
    )";
    auto res1 = engine.eval(code1);
    CHECK(res1.has_value(), "Test 1 must return value");
    CHECK(garuda::beam::to_string(*res1) == "[2,4,6]", "Test 1 result must be [2,4,6]");
    std::cout << "[Test 1] [X * 2 untuk X ing [1, 2, 3]]: " << garuda::beam::to_string(*res1) << " [PASS]\n";

    // Test 2: List Comprehension with Filter [X untuk X ing [1, 2, 3, 4] yen X > 2]
    std::string code2 = R"(
        modul TestC2
        fungsi utama() ->
            [X untuk X ing [1, 2, 3, 4] yen X > 2]
        rampung
    )";
    auto res2 = engine.eval(code2);
    CHECK(res2.has_value(), "Test 2 must return value");
    CHECK(garuda::beam::to_string(*res2) == "[3,4]", "Test 2 result must be [3,4]");
    std::cout << "[Test 2] [X untuk X ing [1, 2, 3, 4] yen X > 2]: " << garuda::beam::to_string(*res2) << " [PASS]\n";

    // Test 3: List Comprehension with Calculation [X + 10 untuk X ing [1, 2, 3]]
    std::string code3 = R"(
        modul TestC3
        fungsi utama() ->
            [X + 10 untuk X ing [1, 2, 3]]
        rampung
    )";
    auto res3 = engine.eval(code3);
    CHECK(res3.has_value(), "Test 3 must return value");
    CHECK(garuda::beam::to_string(*res3) == "[11,12,13]", "Test 3 result must be [11,12,13]");
    std::cout << "[Test 3] [X + 10 untuk X ing [1, 2, 3]]: " << garuda::beam::to_string(*res3) << " [PASS]\n";

    // Test 4: List Comprehension with Tuple Pattern [A + B untuk {A, B} ing [{1, 2}, {3, 4}]]
    std::string code4 = R"(
        modul TestC4
        fungsi utama() ->
            [A + B untuk {A, B} ing [{1, 2}, {3, 4}]]
        rampung
    )";
    auto res4 = engine.eval(code4);
    CHECK(res4.has_value(), "Test 4 must return value");
    CHECK(garuda::beam::to_string(*res4) == "[3,7]", "Test 4 result must be [3,7]");
    std::cout << "[Test 4] [A + B untuk {A, B} ing [{1, 2}, {3, 4}]]: " << garuda::beam::to_string(*res4) << " [PASS]\n";

    // Test 5: Empty List Input [X * 2 untuk X ing []]
    std::string code5 = R"(
        modul TestC5
        fungsi utama() ->
            [X * 2 untuk X ing []]
        rampung
    )";
    auto res5 = engine.eval(code5);
    CHECK(res5.has_value(), "Test 5 must return value");
    CHECK(garuda::beam::to_string(*res5) == "[]" || garuda::beam::to_string(*res5) == "[ ]", "Test 5 result must be []");
    std::cout << "[Test 5] Empty List Input [X * 2 untuk X ing []]: " << garuda::beam::to_string(*res5) << " [PASS]\n";

    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL 5 FASE 6 LIST COMPREHENSION TESTS PASSED PERFECTLY!\n";
    std::cout << "========================================\n";
    return 0;
}
