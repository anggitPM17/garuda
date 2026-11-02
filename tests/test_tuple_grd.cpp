// Garuda Language Compiler: Tuple Test Suite (`test_tuple_grd.cpp`)
#include "garuda/beam/tools/script_engine.hpp"

#include <iostream>
#include <string>

#define CHECK(cond, msg) \
    if (!(cond)) { \
        std::cerr << "[FAIL] " << msg << std::endl; \
        return 1; \
    } else { \
        std::cout << "[PASS] " << msg << std::endl; \
    }

int main() {
    garuda::beam::ScriptEngine engine;

    std::cout << "=======================================================\n";
    std::cout << "GARUDA COMPILER — TUPLE MODULE TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test TUP1: Tuple.panjang({1, 2, 3}) == 3
    std::string code1 = R"(
        modul TestTUP1
        impor "std/tuple.grd"
        fungsi utama() -> Tuple.panjang({1, 2, 3}) rampung
    )";
    auto res1 = engine.eval(code1);
    auto* v1 = res1 ? res1->get_if<std::int64_t>() : nullptr;
    CHECK(v1 && *v1 == 3, "Test TUP1: Tuple.panjang({1, 2, 3}) == 3");

    // Test TUP2: Tuple.elemen({10, 20, 30}, 0) == 10
    std::string code2 = R"(
        modul TestTUP2
        impor "std/tuple.grd"
        fungsi utama() -> Tuple.elemen({10, 20, 30}, 0) rampung
    )";
    auto res2 = engine.eval(code2);
    auto* v2 = res2 ? res2->get_if<std::int64_t>() : nullptr;
    CHECK(v2 && *v2 == 10, "Test TUP2: Tuple.elemen({10, 20, 30}, 0) == 10");

    // Test TUP3: Tuple.elemen({10, 20, 30}, 2) == 30
    std::string code3 = R"(
        modul TestTUP3
        impor "std/tuple.grd"
        fungsi utama() -> Tuple.elemen({10, 20, 30}, 2) rampung
    )";
    auto res3 = engine.eval(code3);
    auto* v3 = res3 ? res3->get_if<std::int64_t>() : nullptr;
    CHECK(v3 && *v3 == 30, "Test TUP3: Tuple.elemen({10, 20, 30}, 2) == 30");

    // Test TUP4: Tuple.elemen({10, 20, 30}, 5) == :error_index
    std::string code4 = R"(
        modul TestTUP4
        impor "std/tuple.grd"
        fungsi utama() -> Tuple.elemen({10, 20, 30}, 5) rampung
    )";
    auto res4 = engine.eval(code4);
    bool is_err4 = res4 && garuda::beam::to_string(*res4) == "error_index";
    CHECK(is_err4, "Test TUP4: Tuple.elemen({10, 20, 30}, 5) == :error_index");

    // Test TUP5: List.panjang(Tuple.ke_list({1, 2, 3})) == 3
    std::string code5 = R"(
        modul TestTUP5
        impor "std/tuple.grd"
        impor "std/list.grd"
        fungsi utama() -> List.panjang(Tuple.ke_list({1, 2, 3})) rampung
    )";
    auto res5 = engine.eval(code5);
    auto* v5 = res5 ? res5->get_if<std::int64_t>() : nullptr;
    CHECK(v5 && *v5 == 3, "Test TUP5: List.panjang(Tuple.ke_list(...)) == 3");

    // Test TUP6: Tuple.panjang(Tuple.gabung({1, 2}, {3, 4})) == 4
    std::string code6 = R"(
        modul TestTUP6
        impor "std/tuple.grd"
        fungsi utama() -> Tuple.panjang(Tuple.gabung({1, 2}, {3, 4})) rampung
    )";
    auto res6 = engine.eval(code6);
    auto* v6 = res6 ? res6->get_if<std::int64_t>() : nullptr;
    CHECK(v6 && *v6 == 4, "Test TUP6: Tuple.panjang(Tuple.gabung(...)) == 4");

    std::cout << "\n🎉 ALL TUPLE MODULE TESTS PASSED PERFECTLY!\n";
    return 0;
}
