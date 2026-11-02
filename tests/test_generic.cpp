// Garuda Language Compiler: Generic Types Parser Test Suite (`test_generic.cpp`)
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
    std::cout << "GARUDA COMPILER — GENERIC TYPES PARSER TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test GT1: fungsi f(X: List<Int>) -> X
    std::string code1 = R"(
        modul TestGT1
        impor "std/list.grd"
        fungsi f(X: List<Int>) -> X rampung
        fungsi utama() -> List.panjang(f([1, 2, 3])) rampung
    )";
    auto res1 = engine.eval(code1);
    bool is_ok1 = res1 && garuda::beam::to_string(*res1) == "3";
    CHECK(is_ok1, "Test GT1: fungsi f(X: List<Int>) -> List.panjang([1,2,3]) == 3");

    // Test GT2: fungsi f(M: Map<String, Int>) -> Map<String, Int> -> M
    std::string code2 = R"(
        modul TestGT2
        impor "std/map.grd"
        fungsi f(M: Map<String, Int>) -> Map<String, Int> -> M rampung
        fungsi utama() -> Map.ukuran(f(Map.taruh(Map.baru(), "a", 1))) rampung
    )";
    auto res2 = engine.eval(code2);
    bool is_ok2 = res2 && garuda::beam::to_string(*res2) == "1";
    CHECK(is_ok2, "Test GT2: fungsi f(M: Map<String, Int>) -> Map<String, Int> -> Map.ukuran == 1");

    std::cout << "\n🎉 ALL GENERIC TYPES PARSER TESTS PASSED PERFECTLY!\n";
    return 0;
}
