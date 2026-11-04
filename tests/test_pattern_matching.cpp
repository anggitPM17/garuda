// =====================================================================
// GARUDA COMPILER — FASE 6 PATTERN MATCHING TEST SUITE (8 TESTS)
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
    std::cout << "GARUDA COMPILER — FASE 6 PATTERN MATCHING TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test 1: Pattern Matching with Literal (tinjau 5 ing 5 -> 100; _ -> 200 rampung)
    std::string code1 = R"(
        modul Test1
        fungsi utama() ->
            tinjau 5 ing
                5 -> 100
                _ -> 200
            rampung
        rampung
    )";
    auto res1 = engine.eval(code1);
    CHECK(res1.has_value(), "Test 1 must return value");
    auto* i1 = res1->get_if<std::int64_t>();
    CHECK(i1 != nullptr && *i1 == 100, "Test 1 result must be 100");
    std::cout << "[Test 1] Literal Pattern Matching: " << *i1 << " [PASS]\n";

    // Test 2: Pattern Matching with Wildcard Fallback (tinjau 7 ing 5 -> 100; _ -> 200 rampung)
    std::string code2 = R"(
        modul Test2
        fungsi utama() ->
            tinjau 7 ing
                5 -> 100
                _ -> 200
            rampung
        rampung
    )";
    auto res2 = engine.eval(code2);
    CHECK(res2.has_value(), "Test 2 must return value");
    auto* i2 = res2->get_if<std::int64_t>();
    CHECK(i2 != nullptr && *i2 == 200, "Test 2 result must be 200");
    std::cout << "[Test 2] Wildcard Fallback Matching: " << *i2 << " [PASS]\n";

    // Test 3: Pattern Matching with Variable Binding
    std::string code3 = R"(
        modul Test3
        fungsi utama() ->
            tinjau 42 ing
                X -> X + 8
            rampung
        rampung
    )";
    auto res3 = engine.eval(code3);
    CHECK(res3.has_value(), "Test 3 must return value");
    auto* i3 = res3->get_if<std::int64_t>();
    CHECK(i3 != nullptr && *i3 == 50, "Test 3 result must be 50");
    std::cout << "[Test 3] Variable Binding Pattern Matching: " << *i3 << " [PASS]\n";

    // Test 4: Pattern Guard Clause (`nalika N > 0 -> ...`)
    std::string code4 = R"(
        modul Test4
        fungsi utama() ->
            tinjau 15 ing
                N nalika N > 10 -> N * 2
                N -> N
            rampung
        rampung
    )";
    auto res4 = engine.eval(code4);
    CHECK(res4.has_value(), "Test 4 must return value");
    auto* i4 = res4->get_if<std::int64_t>();
    CHECK(i4 != nullptr && *i4 == 30, "Test 4 result must be 30");
    std::cout << "[Test 4] Pattern Guard Clause (nalika): " << *i4 << " [PASS]\n";

    // Test 5: Pattern Guard Clause Fallback
    std::string code5 = R"(
        modul Test5
        fungsi utama() ->
            tinjau 5 ing
                N nalika N > 10 -> N * 2
                N -> N + 1
            rampung
        rampung
    )";
    auto res5 = engine.eval(code5);
    CHECK(res5.has_value(), "Test 5 must return value");
    auto* i5 = res5->get_if<std::int64_t>();
    CHECK(i5 != nullptr && *i5 == 6, "Test 5 result must be 6");
    std::cout << "[Test 5] Pattern Guard Fallback: " << *i5 << " [PASS]\n";

    // Test 6: Nested Tuple Pattern Matching
    std::string code6 = R"(
        modul Test6
        fungsi utama() ->
            tinjau {10, {20, 30}} ing
                {A, {B, C}} -> A + B + C
                _ -> 0
            rampung
        rampung
    )";
    auto res6 = engine.eval(code6);
    CHECK(res6.has_value(), "Test 6 must return value");
    auto* i6 = res6->get_if<std::int64_t>();
    CHECK(i6 != nullptr && *i6 == 60, "Test 6 result must be 60");
    std::cout << "[Test 6] Nested Tuple Pattern Matching: " << *i6 << " [PASS]\n";

    // Test 7: List Head/Tail Pattern Matching
    std::string code7 = R"(
        modul Test7
        fungsi utama() ->
            tinjau [100, 200, 300] ing
                [H | T] -> H
                _ -> 0
            rampung
        rampung
    )";
    auto res7 = engine.eval(code7);
    CHECK(res7.has_value(), "Test 7 must return value");
    auto* i7 = res7->get_if<std::int64_t>();
    CHECK(i7 != nullptr && *i7 == 100, "Test 7 result must be 100");
    std::cout << "[Test 7] List Head/Tail Pattern Matching: " << *i7 << " [PASS]\n";

    // Test 8: Atom & String Pattern Matching
    std::string code8 = R"(
        modul Test8
        fungsi utama() ->
            tinjau "garuda" ing
                "salah" -> 0
                "garuda" -> 99
                _ -> 1
            rampung
        rampung
    )";
    auto res8 = engine.eval(code8);
    CHECK(res8.has_value(), "Test 8 must return value");
    auto* i8 = res8->get_if<std::int64_t>();
    CHECK(i8 != nullptr && *i8 == 99, "Test 8 result must be 99");
    std::cout << "[Test 8] String Literal Pattern Matching: " << *i8 << " [PASS]\n";

    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL 8 FASE 6 PATTERN MATCHING TESTS PASSED PERFECTLY!\n";
    std::cout << "========================================\n";
    return 0;
}
