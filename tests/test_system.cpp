// Garuda Language Compiler: System Modules Test Suite (`test_system.cpp`)
#include "garuda/beam/tools/script_engine.hpp"

#include <fstream>
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
    std::cout << "GARUDA COMPILER — SYSTEM MODULES TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test SYS1: File.tulis & File.baca
    std::string code1 = R"(
        modul TestSYS1
        impor "std/system.grd"
        fungsi utama() ->
            File.tulis("test_sys_out.txt", "Halo dari Garuda File I/O")
            File.baca("test_sys_out.txt")
        rampung
    )";
    auto res1 = engine.eval(code1);
    bool is_ok1 = res1 && (garuda::beam::to_string(*res1) == "Halo dari Garuda File I/O" || garuda::beam::to_string(*res1) == "\"Halo dari Garuda File I/O\"");
    CHECK(is_ok1, "Test SYS1: File.tulis & File.baca == 'Halo dari Garuda File I/O'");

    // Test SYS2: File.ada
    std::string code2 = R"(
        modul TestSYS2
        fungsi utama() ->
            File.ada("test_sys_out.txt")
        rampung
    )";
    auto res2 = engine.eval(code2);
    bool is_ok2 = res2 && garuda::beam::to_string(*res2) == "true";
    CHECK(is_ok2, "Test SYS2: File.ada('test_sys_out.txt') == true");

    // Test SYS3: Os.env
    std::string code3 = R"(
        modul TestSYS3
        fungsi utama() ->
            Os.env("PATH")
        rampung
    )";
    auto res3 = engine.eval(code3);
    bool is_ok3 = res3 && !garuda::beam::to_string(*res3).empty();
    CHECK(is_ok3, "Test SYS3: Os.env('PATH') is non-empty");

    // Clean up temporary test file
    std::remove("test_sys_out.txt");

    std::cout << "\n🎉 ALL SYSTEM MODULES TESTS PASSED PERFECTLY!\n";
    return 0;
}
