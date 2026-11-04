// =====================================================================
// GARUDA COMPILER — FASE 7 PIPELINE OPERATOR TEST SUITE (5 TESTS)
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
    std::cout << "GARUDA COMPILER — FASE 7 PIPELINE OPERATOR TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test 1: Basic Pipe to Function
    std::string code1 = R"(
        modul TestP1
        fungsi inc(X) -> X + 1 rampung
        fungsi utama() ->
            5 |> inc
        rampung
    )";
    auto res1 = engine.eval(code1);
    CHECK(res1.has_value(), "Test 1 must return value");
    auto* i1 = res1->get_if<std::int64_t>();
    CHECK(i1 != nullptr && *i1 == 6, "Test 1 result must be 6");
    std::cout << "[Test 1] 5 |> inc: " << *i1 << " [PASS]\n";

    // Test 2: Chained Pipe
    std::string code2 = R"(
        modul TestP2
        fungsi inc(X) -> X + 1 rampung
        fungsi utama() ->
            5 |> inc |> inc
        rampung
    )";
    auto res2 = engine.eval(code2);
    CHECK(res2.has_value(), "Test 2 must return value");
    auto* i2 = res2->get_if<std::int64_t>();
    CHECK(i2 != nullptr && *i2 == 7, "Test 2 result must be 7");
    std::cout << "[Test 2] 5 |> inc |> inc: " << *i2 << " [PASS]\n";

    // Test 3: Placeholder `_` as First Argument
    std::string code3 = R"(
        modul TestP3
        fungsi add(A, B) -> A + B rampung
        fungsi utama() ->
            5 |> add(_, 10)
        rampung
    )";
    auto res3 = engine.eval(code3);
    CHECK(res3.has_value(), "Test 3 must return value");
    auto* i3 = res3->get_if<std::int64_t>();
    CHECK(i3 != nullptr && *i3 == 15, "Test 3 result must be 15");
    std::cout << "[Test 3] 5 |> add(_, 10): " << *i3 << " [PASS]\n";

    // Test 4: Placeholder `_` as Second Argument
    std::string code4 = R"(
        modul TestP4
        fungsi kurangi(A, B) -> A - B rampung
        fungsi utama() ->
            5 |> kurangi(100, _)
        rampung
    )";
    auto res4 = engine.eval(code4);
    CHECK(res4.has_value(), "Test 4 must return value");
    auto* i4 = res4->get_if<std::int64_t>();
    CHECK(i4 != nullptr && *i4 == 95, "Test 4 result must be 95");
    std::cout << "[Test 4] 5 |> kurangi(100, _): " << *i4 << " [PASS]\n";

    // Test 5: Multiple Arguments with Placeholder
    std::string code5 = R"(
        modul TestP5
        fungsi hitung(A, B, C) -> A - B + C rampung
        fungsi utama() ->
            5 |> hitung(100, _, 2)
        rampung
    )";
    auto res5 = engine.eval(code5);
    CHECK(res5.has_value(), "Test 5 must return value");
    auto* i5 = res5->get_if<std::int64_t>();
    CHECK(i5 != nullptr && *i5 == 97, "Test 5 result must be 97");
    std::cout << "[Test 5] 5 |> hitung(100, _, 2): " << *i5 << " [PASS]\n";

    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL 5 FASE 7 PIPELINE OPERATOR TESTS PASSED PERFECTLY!\n";
    std::cout << "========================================\n";
    return 0;
}
