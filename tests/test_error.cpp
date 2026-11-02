// Garuda Language Compiler: Error Handling Test Suite (`test_error.cpp`)
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
    std::cout << "=======================================================\n";
    std::cout << "GARUDA COMPILER — ERROR HANDLING TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test E1: Division by zero caught by coba/tangkap
    std::string code1 = R"(
        modul TestE1
        fungsi utama() ->
            coba
                1 / 0
            tangkap E ->
                :error_div
            rampung
        rampung
    )";
    garuda::beam::ScriptEngine engine1;
    auto res1 = engine1.eval(code1);
    bool is_ok1 = res1 && garuda::beam::to_string(*res1) == "error_div";
    CHECK(is_ok1, "Test E1: Division by zero caught by coba/tangkap -> :error_div");

    // Test E2: Error.lempar caught by coba/tangkap
    std::string code2 = R"(
        modul TestE2
        impor "std/error.grd"
        fungsi utama() ->
            coba
                Error.lempar("pesan_error_custom")
            tangkap E ->
                Error.pesan(E)
            rampung
        rampung
    )";
    garuda::beam::ScriptEngine engine2;
    auto res2 = engine2.eval(code2);
    bool is_ok2 = res2 && (garuda::beam::to_string(*res2) == "pesan_error_custom" || garuda::beam::to_string(*res2) == "\"pesan_error_custom\"");
    CHECK(is_ok2, "Test E2: Error.lempar caught by coba/tangkap -> 'pesan_error_custom'");

    // Test E3: Successful coba block bypasses tangkap
    std::string code3 = R"(
        modul TestE3
        fungsi utama() ->
            coba
                10 + 20
            tangkap E ->
                0
            rampung
        rampung
    )";
    garuda::beam::ScriptEngine engine3;
    auto res3 = engine3.eval(code3);
    bool is_ok3 = res3 && garuda::beam::to_string(*res3) == "30";
    CHECK(is_ok3, "Test E3: Successful coba block returns 30");

    // Test E4: Error.jenis
    std::string code4 = R"(
        modul TestE4
        impor "std/error.grd"
        fungsi utama() ->
            Error.jenis(:error_custom)
        rampung
    )";
    garuda::beam::ScriptEngine engine4;
    auto res4 = engine4.eval(code4);
    bool is_ok4 = res4 && garuda::beam::to_string(*res4) == "error_custom";
    CHECK(is_ok4, "Test E4: Error.jenis(:error_custom) == :error_custom");

    std::cout << "\n🎉 ALL ERROR HANDLING TESTS PASSED PERFECTLY!\n";
    return 0;
}
