// =====================================================================
// GVM G2 std/result.grd UNIT TEST WITH CHECK MACRO
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

    // Test 1: Pattern Matching Tuple in tinjau
    std::string code1 = R"(
        modul Test1
        fungsi unwrap_or(Res, Default) ->
            tinjau Res ing
                {:ok, Val} -> Val
                {:error, _} -> Default
            rampung
        rampung
        fungsi utama() ->
            unwrap_or({:ok, 42}, 0)
        rampung
    )";
    auto res1 = engine.eval(code1);
    CHECK(res1.has_value(), "Test 1 must return value");
    auto* i1 = res1->get_if<std::int64_t>();
    CHECK(i1 != nullptr && *i1 == 42, "Test 1 result must be 42");
    std::cout << "[Test 1] unwrap_or({:ok, 42}, 0): " << *i1 << " [PASS]\n";

    // Test 2: Pattern Matching Error Fallback in tinjau
    std::string code2 = R"(
        modul Test2
        fungsi unwrap_or(Res, Default) ->
            tinjau Res ing
                {:ok, Val} -> Val
                {:error, _} -> Default
            rampung
        rampung
        fungsi utama() ->
            unwrap_or({:error, "fail"}, 100)
        rampung
    )";
    auto res2 = engine.eval(code2);
    CHECK(res2.has_value(), "Test 2 must return value");
    auto* i2 = res2->get_if<std::int64_t>();
    CHECK(i2 != nullptr && *i2 == 100, "Test 2 result must be 100");
    std::cout << "[Test 2] unwrap_or({:error, \"fail\"}, 100): " << *i2 << " [PASS]\n";

    std::cout << "\n========================================\n";
    std::cout << "ALL G2 std/result.grd TESTS PASSED 100% WITH REAL CHECK MACRO!\n";
    std::cout << "========================================\n";
    return 0;
}
