// =====================================================================
// GARUDA COMPILER — FASE 6 DESTRUCTURING TEST SUITE (5 TESTS)
// REBUILD TRIGGER TIMESTAMP: 2026-11-04 13:40:00
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
    std::cout << "=======================================================\n";
    std::cout << "GARUDA COMPILER — FASE 6 DESTRUCTURING TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test 1: Tuple Destructuring `{A, B} = {10, 20}`
    {
        garuda::beam::ScriptEngine engine;
        std::string code1 = R"(
            modul TestD1
            fungsi utama() ->
                {A, B} = {10, 20}
                A + B
            rampung
        )";
        auto res1 = engine.eval(code1);
        CHECK(res1.has_value(), "Test 1 must return value");
        auto* i1 = res1->get_if<std::int64_t>();
        CHECK(i1 != nullptr && *i1 == 30, "Test 1 result must be 30");
        std::cout << "[Test 1] Tuple Destructuring {A, B} = {10, 20}: " << *i1 << " [PASS]\n";
    }

    // Test 2: List Head/Tail Destructuring `[H | T] = [100, 200, 300]`
    {
        garuda::beam::ScriptEngine engine;
        std::string code2 = R"(
            modul TestD2
            fungsi utama() ->
                [H | T] = [100, 200, 300]
                H
            rampung
        )";
        auto res2 = engine.eval(code2);
        CHECK(res2.has_value(), "Test 2 must return value");
        auto* i2 = res2->get_if<std::int64_t>();
        CHECK(i2 != nullptr && *i2 == 100, "Test 2 result must be 100");
        std::cout << "[Test 2] List Destructuring [H | T] = [100, 200, 300]: " << *i2 << " [PASS]\n";
    }

    // Test 3: Nested Tuple Destructuring `{A, {B, C}} = {1, {2, 3}}`
    {
        garuda::beam::ScriptEngine engine;
        std::string code3 = R"(
            modul TestD3
            fungsi utama() ->
                {A, {B, C}} = {1, {2, 3}}
                A + B + C
            rampung
        )";
        auto res3 = engine.eval(code3);
        CHECK(res3.has_value(), "Test 3 must return value");
        auto* i3 = res3->get_if<std::int64_t>();
        CHECK(i3 != nullptr && *i3 == 6, "Test 3 result must be 6");
        std::cout << "[Test 3] Nested Tuple Destructuring {A, {B, C}}: " << *i3 << " [PASS]\n";
    }

    // Test 4: Wildcard Destructuring `{A, _} = {50, 99}`
    {
        garuda::beam::ScriptEngine engine;
        std::string code4 = R"(
            modul TestD4
            fungsi utama() ->
                {A, _} = {50, 99}
                A
            rampung
        )";
        auto res4 = engine.eval(code4);
        CHECK(res4.has_value(), "Test 4 must return value");
        auto* i4 = res4->get_if<std::int64_t>();
        CHECK(i4 != nullptr && *i4 == 50, "Test 4 result must be 50");
        std::cout << "[Test 4] Wildcard Destructuring {A, _}: " << *i4 << " [PASS]\n";
    }

    // Test 5: Multiple Destructuring Statements
    {
        garuda::beam::ScriptEngine engine;
        std::string code5 = R"(
            modul TestD5
            fungsi utama() ->
                {X, Y} = {10, 20}
                {Z, W} = {30, 40}
                X + Y + Z + W
            rampung
        )";
        auto res5 = engine.eval(code5);
        CHECK(res5.has_value(), "Test 5 must return value");
        auto* i5 = res5->get_if<std::int64_t>();
        CHECK(i5 != nullptr && *i5 == 100, "Test 5 result must be 100");
        std::cout << "[Test 5] Multiple Destructuring Statements: " << *i5 << " [PASS]\n";
    }

    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL 5 FASE 6 DESTRUCTURING TESTS PASSED PERFECTLY!\n";
    std::cout << "========================================\n";
    return 0;
}
