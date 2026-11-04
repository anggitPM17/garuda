// =====================================================================
// GARUDA COMPILER — FASE 7 MAP COMPREHENSION TEST SUITE (5 TESTS)
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
    std::cout << "GARUDA COMPILER — FASE 7 MAP COMPREHENSION TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test 1: Basic Map Comprehension %{K => V * 2 untuk {K, V} ing [{:a, 1}, {:b, 2}]}
    std::string code1 = R"(
        modul TestMC1
        fungsi utama() ->
            M = %{K => V * 2 untuk {K, V} ing [{:a, 1}, {:b, 2}]}
            Map.ambil(M, :b)
        rampung
    )";
    auto res1 = engine.eval(code1);
    CHECK(res1.has_value(), "Test 1 must return value");
    auto* i1 = res1->get_if<std::int64_t>();
    CHECK(i1 != nullptr && *i1 == 4, "Test 1 result must be 4");
    std::cout << "[Test 1] Map Comprehension %{K => V * 2}: " << *i1 << " [PASS]\n";

    // Test 2: Filtered Map Comprehension %{K => V untuk {K, V} ing [{:a, 10}, {:b, 20}] yen V > 10}
    std::string code2 = R"(
        modul TestMC2
        fungsi utama() ->
            M = %{K => V untuk {K, V} ing [{:a, 10}, {:b, 20}] yen V > 10}
            Map.ukuran(M)
        rampung
    )";
    auto res2 = engine.eval(code2);
    CHECK(res2.has_value(), "Test 2 must return value");
    auto* i2 = res2->get_if<std::int64_t>();
    CHECK(i2 != nullptr && *i2 == 1, "Test 2 result must be 1");
    std::cout << "[Test 2] Filtered Map Comprehension size: " << *i2 << " [PASS]\n";

    // Test 3: Map Comprehension with Value Addition %{K => V + 1 untuk {K, V} ing [{:x, 1}, {:y, 2}]}
    std::string code3 = R"(
        modul TestMC3
        fungsi utama() ->
            M = %{K => V + 1 untuk {K, V} ing [{:x, 1}, {:y, 2}]}
            Map.ambil(M, :x)
        rampung
    )";
    auto res3 = engine.eval(code3);
    CHECK(res3.has_value(), "Test 3 must return value");
    auto* i3 = res3->get_if<std::int64_t>();
    CHECK(i3 != nullptr && *i3 == 2, "Test 3 result must be 2");
    std::cout << "[Test 3] Map Comprehension %{K => V + 1}: " << *i3 << " [PASS]\n";

    // Test 4: Empty Input Map Comprehension %{K => V untuk {K, V} ing []}
    std::string code4 = R"(
        modul TestMC4
        fungsi utama() ->
            M = %{K => V untuk {K, V} ing []}
            Map.ukuran(M)
        rampung
    )";
    auto res4 = engine.eval(code4);
    CHECK(res4.has_value(), "Test 4 must return value");
    auto* i4 = res4->get_if<std::int64_t>();
    CHECK(i4 != nullptr && *i4 == 0, "Test 4 result must be 0");
    std::cout << "[Test 4] Empty Input Map Comprehension size: " << *i4 << " [PASS]\n";

    // Test 5: Map Literal Syntax %{:a => 10, :b => 20}
    std::string code5 = R"(
        modul TestMC5
        fungsi utama() ->
            M = %{:a => 10, :b => 20}
            Map.ambil(M, :b)
        rampung
    )";
    auto res5 = engine.eval(code5);
    CHECK(res5.has_value(), "Test 5 must return value");
    auto* i5 = res5->get_if<std::int64_t>();
    CHECK(i5 != nullptr && *i5 == 20, "Test 5 result must be 20");
    std::cout << "[Test 5] Map Literal %{:a => 10, :b => 20}: " << *i5 << " [PASS]\n";

    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL 5 FASE 7 MAP COMPREHENSION TESTS PASSED PERFECTLY!\n";
    std::cout << "========================================\n";
    return 0;
}
