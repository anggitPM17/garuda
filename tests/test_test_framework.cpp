// Garuda Language Compiler: Testing Framework Test Suite (`test_test_framework.cpp`)
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
    std::cout << "GARUDA COMPILER — TESTING FRAMEWORK TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test TF1: Test.sama(1+1, 2, "x") -> :ok
    std::string code1 = R"(
        modul TestTF1
        impor "std/test.grd"
        fungsi utama() ->
            Test.sama(1 + 1, 2, "TF1-sama-pass")
        rampung
    )";
    auto res1 = engine.eval(code1);
    bool is_ok1 = res1 && garuda::beam::to_string(*res1) == "ok";
    CHECK(is_ok1, "Test TF1: Test.sama(2, 2) == :ok");

    // Test TF2: Test.sama(1+1, 3, "x") -> :error
    std::string code2 = R"(
        modul TestTF2
        impor "std/test.grd"
        fungsi utama() ->
            Test.sama(1 + 1, 3, "TF2-sama-fail")
        rampung
    )";
    auto res2 = engine.eval(code2);
    bool is_err2 = res2 && garuda::beam::to_string(*res2) == "error";
    CHECK(is_err2, "Test TF2: Test.sama(2, 3) == :error");

    // Test TF3: Test.harus_benar(benar, "x") -> :ok
    std::string code3 = R"(
        modul TestTF3
        impor "std/test.grd"
        fungsi utama() ->
            Test.harus_benar(benar, "TF3-benar-pass")
        rampung
    )";
    auto res3 = engine.eval(code3);
    bool is_ok3 = res3 && garuda::beam::to_string(*res3) == "ok";
    CHECK(is_ok3, "Test TF3: Test.harus_benar(benar) == :ok");

    // Test TF4: Test.harus_salah(salah, "x") -> :ok
    std::string code4 = R"(
        modul TestTF4
        impor "std/test.grd"
        fungsi utama() ->
            Test.harus_salah(salah, "TF4-salah-pass")
        rampung
    )";
    auto res4 = engine.eval(code4);
    bool is_ok4 = res4 && garuda::beam::to_string(*res4) == "ok";
    CHECK(is_ok4, "Test TF4: Test.harus_salah(salah) == :ok");

    // Test TF5: Test.ringkasan([:ok, :ok, :ok]) -> :ok
    std::string code5 = R"(
        modul TestTF5
        impor "std/test.grd"
        fungsi utama() ->
            Test.ringkasan([:ok, :ok, :ok])
        rampung
    )";
    auto res5 = engine.eval(code5);
    bool is_ok5 = res5 && garuda::beam::to_string(*res5) == "ok";
    CHECK(is_ok5, "Test TF5: Test.ringkasan([:ok, :ok, :ok]) == :ok");

    std::cout << "\n🎉 ALL TESTING FRAMEWORK TESTS PASSED PERFECTLY!\n";
    return 0;
}
