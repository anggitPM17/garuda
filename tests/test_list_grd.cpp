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

    // Test 1: List.panjang([10, 20, 30]) == 3
    std::string code1 = R"(
        modul Test1
        fungsi utama() ->
            List.panjang([10, 20, 30])
        rampung
    )";
    auto res1 = engine.eval(code1);
    CHECK(res1.has_value(), "Test 1 must return value");
    auto* i1 = res1->get_if<std::int64_t>();
    CHECK(i1 != nullptr && *i1 == 3, "Test 1 List.panjang must be 3");
    std::cout << "[Test 1] List.panjang([10, 20, 30]): " << *i1 << " [PASS]\n";

    // Test 2: List.kepala([42, 99]) == 42
    std::string code2 = R"(
        modul Test2
        fungsi utama() ->
            List.kepala([42, 99])
        rampung
    )";
    auto res2 = engine.eval(code2);
    CHECK(res2.has_value(), "Test 2 must return value");
    auto* i2 = res2->get_if<std::int64_t>();
    CHECK(i2 != nullptr && *i2 == 42, "Test 2 List.kepala must be 42");
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
    CHECK(i3 != nullptr && *i3 == 3, "Test 3 List.tambah length must be 3");
    std::cout << "[Test 3] List.tambah([1, 2], 3) length: " << *i3 << " [PASS]\n";

    // Test 4: List.ekor([42, 99])
    std::string code4 = R"(
        modul Test4
        fungsi utama() ->
            List.kepala(List.ekor([42, 99]))
        rampung
    )";
    auto res4 = engine.eval(code4);
    CHECK(res4.has_value(), "Test 4 must return value");
    auto* i4 = res4->get_if<std::int64_t>();
    CHECK(i4 != nullptr && *i4 == 99, "Test 4 List.ekor head must be 99");
    std::cout << "[Test 4] List.kepala(List.ekor([42, 99])): " << *i4 << " [PASS]\n";

    // Test 5: List.ekor([42]) length == 0
    std::string code5 = R"(
        modul Test5
        fungsi utama() ->
            List.panjang(List.ekor([42]))
        rampung
    )";
    auto res5 = engine.eval(code5);
    CHECK(res5.has_value(), "Test 5 must return value");
    auto* i5 = res5->get_if<std::int64_t>();
    CHECK(i5 != nullptr && *i5 == 0, "Test 5 List.ekor([42]) length must be 0");
    std::cout << "[Test 5] List.panjang(List.ekor([42])): " << *i5 << " [PASS]\n";

    // Test 6: List.panjang([]) == 0
    std::string code6 = R"(
        modul Test6
        fungsi utama() ->
            List.panjang([])
        rampung
    )";
    auto res6 = engine.eval(code6);
    CHECK(res6.has_value(), "Test 6 must return value");
    auto* i6 = res6->get_if<std::int64_t>();
    CHECK(i6 != nullptr && *i6 == 0, "Test 6 List.panjang([]) must be 0");
    std::cout << "[Test 6] List.panjang([]): " << *i6 << " [PASS]\n";

    // Test 7: List.kepala([]) == nil
    std::string code7 = R"(
        modul Test7
        fungsi utama() ->
            List.kepala([])
        rampung
    )";
    auto res7 = engine.eval(code7);
    CHECK(res7.has_value(), "Test 7 must return value");
    bool is_nil = res7->is<std::monostate>() || (res7->get_if<garuda::beam::Atom>() && res7->get_if<garuda::beam::Atom>()->name == "nil");
    CHECK(is_nil, "Test 7 List.kepala([]) must return nil");
    std::cout << "[Test 7] List.kepala([]): nil [PASS]\n";

    std::cout << "\n========================================\n";
    std::cout << "ALL 7 G3 std/list.grd TESTS PASSED 100% WITH REAL CHECK MACRO!\n";
    std::cout << "========================================\n";
    return 0;
}
