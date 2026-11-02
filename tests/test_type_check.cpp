// Garuda Language Compiler: Type Checking Test Suite (`test_type_check.cpp`)
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
    std::cout << "GARUDA COMPILER — TYPE CHECKING TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test TC1: 1 + "a" caught by coba/tangkap -> :error_type
    std::string code1 = R"(
        modul TestTC1
        fungsi utama() ->
            coba
                1 + "a"
            tangkap E ->
                :error_type
            rampung
        rampung
    )";
    auto res1 = engine.eval(code1);
    bool is_ok1 = res1 && garuda::beam::to_string(*res1) == "error_type";
    CHECK(is_ok1, "Test TC1: 1 + 'a' caught -> :error_type");

    // Test TC2: 1 + 2 == 3
    std::string code2 = R"(
        modul TestTC2
        fungsi utama() -> 1 + 2 rampung
    )";
    auto res2 = engine.eval(code2);
    bool is_ok2 = res2 && garuda::beam::to_string(*res2) == "3";
    CHECK(is_ok2, "Test TC2: 1 + 2 == 3");

    // Test TC3: "a" ++ "b" == "ab"
    std::string code3 = R"(
        modul TestTC3
        fungsi utama() -> "a" ++ "b" rampung
    )";
    auto res3 = engine.eval(code3);
    bool is_ok3 = res3 && (garuda::beam::to_string(*res3) == "ab" || garuda::beam::to_string(*res3) == "\"ab\"");
    CHECK(is_ok3, "Test TC3: 'a' ++ 'b' == 'ab'");

    std::cout << "\n🎉 ALL TYPE CHECKING TESTS PASSED PERFECTLY!\n";
    return 0;
}
