// Garuda Language Compiler: Matematika Test Suite (`test_matematika.cpp`)
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
    std::cout << "GARUDA COMPILER — MATEMATIKA TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test M1: pangkat(2, 3) == 8
    std::string codeM1 = R"(
        modul TestM1
        impor "std/matematika.grd"
        fungsi utama() ->
            Matematika.pangkat(2, 3)
        rampung
    )";
    auto resM1 = engine.eval(codeM1);
    auto* valM1 = resM1 ? resM1->get_if<std::int64_t>() : nullptr;
    CHECK(valM1 && *valM1 == 8, "Test M1: Matematika.pangkat(2, 3) == 8");

    // Test M2: pangkat(5, 0) == 1
    std::string codeM2 = R"(
        modul TestM2
        impor "std/matematika.grd"
        fungsi utama() ->
            Matematika.pangkat(5, 0)
        rampung
    )";
    auto resM2 = engine.eval(codeM2);
    auto* valM2 = resM2 ? resM2->get_if<std::int64_t>() : nullptr;
    CHECK(valM2 && *valM2 == 1, "Test M2: Matematika.pangkat(5, 0) == 1");

    // Test M3: abs(-5) == 5 dan abs(5) == 5
    std::string codeM3 = R"(
        modul TestM3
        impor "std/matematika.grd"
        fungsi utama() ->
            Matematika.abs(-5) + Matematika.abs(5)
        rampung
    )";
    auto resM3 = engine.eval(codeM3);
    auto* valM3 = resM3 ? resM3->get_if<std::int64_t>() : nullptr;
    CHECK(valM3 && *valM3 == 10, "Test M3: Matematika.abs(-5) + abs(5) == 10");

    // Test M4: min(3, 7) == 3
    std::string codeM4 = R"(
        modul TestM4
        impor "std/matematika.grd"
        fungsi utama() ->
            Matematika.min(3, 7)
        rampung
    )";
    auto resM4 = engine.eval(codeM4);
    auto* valM4 = resM4 ? resM4->get_if<std::int64_t>() : nullptr;
    CHECK(valM4 && *valM4 == 3, "Test M4: Matematika.min(3, 7) == 3");

    // Test M5: max(3, 7) == 7
    std::string codeM5 = R"(
        modul TestM5
        impor "std/matematika.grd"
        fungsi utama() ->
            Matematika.max(3, 7)
        rampung
    )";
    auto resM5 = engine.eval(codeM5);
    auto* valM5 = resM5 ? resM5->get_if<std::int64_t>() : nullptr;
    CHECK(valM5 && *valM5 == 7, "Test M5: Matematika.max(3, 7) == 7");

    // Test M6: akar(9) == 3 dan akar(16) == 4
    std::string codeM6 = R"(
        modul TestM6
        impor "std/matematika.grd"
        fungsi utama() ->
            Matematika.akar(9) + Matematika.akar(16)
        rampung
    )";
    auto resM6 = engine.eval(codeM6);
    auto* valM6 = resM6 ? resM6->get_if<std::int64_t>() : nullptr;
    CHECK(valM6 && *valM6 == 7, "Test M6: Matematika.akar(9) + akar(16) == 7");

    // Test M7: akar(10) == 3 (floor integer sqrt)
    std::string codeM7 = R"(
        modul TestM7
        impor "std/matematika.grd"
        fungsi utama() ->
            Matematika.akar(10)
        rampung
    )";
    auto resM7 = engine.eval(codeM7);
    auto* valM7 = resM7 ? resM7->get_if<std::int64_t>() : nullptr;
    CHECK(valM7 && *valM7 == 3, "Test M7: Matematika.akar(10) == 3 (floor)");

    // Test M8: pembulatan(5) == 5
    std::string codeM8 = R"(
        modul TestM8
        impor "std/matematika.grd"
        fungsi utama() ->
            Matematika.pembulatan(5)
        rampung
    )";
    auto resM8 = engine.eval(codeM8);
    auto* valM8 = resM8 ? resM8->get_if<std::int64_t>() : nullptr;
    CHECK(valM8 && *valM8 == 5, "Test M8: Matematika.pembulatan(5) == 5");

    std::cout << "\n🎉 ALL MATEMATIKA TESTS PASSED PERFECTLY!\n";
    return 0;
}
