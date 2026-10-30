// =====================================================================
// GVM G2 PATTERN MATCHING UNIT TEST WITH CHECK MACRO
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

    // Test 3: Pattern Matching with Variable Binding (Uppercase X)
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

    std::cout << "\n========================================\n";
    std::cout << "ALL 3 G2 PATTERN MATCHING TESTS PASSED 100% WITH REAL CHECK MACRO!\n";
    std::cout << "========================================\n";
    return 0;
}
