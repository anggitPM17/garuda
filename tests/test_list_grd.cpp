// =====================================================================
// GVM G3 std/list.grd UNIT TEST WITH CHECK MACRO
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

    // Test 1: List.panjang([10, 20, 30])
    std::string code1 = R"(
        modul Test1
        fungsi utama() ->
            List.panjang([10, 20, 30])
        rampung
    )";
    auto res1 = engine.eval(code1);
    CHECK(res1.has_value(), "Test 1 must return value");
    auto* i1 = res1->get_if<std::int64_t>();
    CHECK(i1 != nullptr && *i1 == 3, "Test 1 list.panjang must be 3");
    std::cout << "[Test 1] List.panjang([10, 20, 30]): " << *i1 << " [PASS]\n";

    // Test 2: List.kepala([42, 99])
    std::string code2 = R"(
        modul Test2
        fungsi utama() ->
            List.kepala([42, 99])
        rampung
    )";
    auto res2 = engine.eval(code2);
    CHECK(res2.has_value(), "Test 2 must return value");
    auto* i2 = res2->get_if<std::int64_t>();
    CHECK(i2 != nullptr && *i2 == 42, "Test 2 list.kepala must be 42");
    std::cout << "[Test 2] List.kepala([42, 99]): " << *i2 << " [PASS]\n";

    // Test 3: List.tambah([1, 2], 3)
    std::string code3 = R"(
        modul Test3
        fungsi utama() ->
            List.panjang(List.tambah([1, 2], 3))
        rampung
    )";
    auto res3 = engine.eval(code3);
    CHECK(res3.has_value(), "Test 3 must return value");
    auto* i3 = res3->get_if<std::int64_t>();
    CHECK(i3 != nullptr && *i3 == 3, "Test 3 list.tambah length must be 3");
    std::cout << "[Test 3] List.tambah([1, 2], 3) length: " << *i3 << " [PASS]\n";

    std::cout << "\n========================================\n";
    std::cout << "ALL G3 std/list.grd TESTS PASSED 100% WITH REAL CHECK MACRO!\n";
    std::cout << "========================================\n";
    return 0;
}
