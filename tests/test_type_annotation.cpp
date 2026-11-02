// Garuda Language Compiler: Type Annotation Test Suite (`test_type_annotation.cpp`)
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
    std::cout << "GARUDA COMPILER — TYPE ANNOTATION TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test TA1: fungsi f(X: Int) -> X
    std::string code1 = R"(
        modul TestTA1
        fungsi f(X: Int) -> X rampung
        fungsi utama() -> f(42) rampung
    )";
    auto res1 = engine.eval(code1);
    bool is_ok1 = res1 && garuda::beam::to_string(*res1) == "42";
    CHECK(is_ok1, "Test TA1: fungsi f(X: Int) -> X == 42");

    // Test TA2: fungsi f(X: String, Y: Int) -> X
    std::string code2 = R"(
        modul TestTA2
        fungsi f(X: String, Y: Int) -> X rampung
        fungsi utama() -> f("halo", 100) rampung
    )";
    auto res2 = engine.eval(code2);
    bool is_ok2 = res2 && (garuda::beam::to_string(*res2) == "halo" || garuda::beam::to_string(*res2) == "\"halo\"");
    CHECK(is_ok2, "Test TA2: fungsi f(X: String, Y: Int) -> X == 'halo'");

    // Test TA3: Return type annotation: fungsi f(X: Int) -> Int -> X
    std::string code3 = R"(
        modul TestTA3
        fungsi f(X: Int) -> Int -> X rampung
        fungsi utama() -> f(100) rampung
    )";
    auto res3 = engine.eval(code3);
    bool is_ok3 = res3 && garuda::beam::to_string(*res3) == "100";
    CHECK(is_ok3, "Test TA3: fungsi f(X: Int) -> Int -> X == 100");

    std::cout << "\n🎉 ALL TYPE ANNOTATION TESTS PASSED PERFECTLY!\n";
    return 0;
}
