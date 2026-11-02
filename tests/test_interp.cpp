// Garuda Language Compiler: String Interpolation Test Suite (`test_interp.cpp`)
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
    std::cout << "GARUDA COMPILER — STRING INTERPOLATION TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test I1: interpolasi sederhana dengan variabel string
    std::string codeI1 = R"(
        modul TestI1
        fungsi utama() ->
            nama = "Budi"
            "Halo #{nama}"
        rampung
    )";
    auto resI1 = engine.eval(codeI1);
    auto* sI1 = resI1 ? resI1->get_if<std::string>() : nullptr;
    CHECK(sI1 && *sI1 == "Halo Budi", "Test I1: 'Halo #{nama}' == 'Halo Budi'");

    // Test I2: interpolasi dengan integer
    std::string codeI2 = R"(
        modul TestI2
        fungsi utama() ->
            skor = 100
            "Skor: #{skor}"
        rampung
    )";
    auto resI2 = engine.eval(codeI2);
    auto* sI2 = resI2 ? resI2->get_if<std::string>() : nullptr;
    CHECK(sI2 && *sI2 == "Skor: 100", "Test I2: 'Skor: #{skor}' == 'Skor: 100'");

    // Test I3: multiple interpolasi
    std::string codeI3 = R"(
        modul TestI3
        fungsi utama() ->
            nama = "Budi"
            skor = 100
            "#{nama}: #{skor}"
        rampung
    )";
    auto resI3 = engine.eval(codeI3);
    auto* sI3 = resI3 ? resI3->get_if<std::string>() : nullptr;
    CHECK(sI3 && *sI3 == "Budi: 100", "Test I3: '#{nama}: #{skor}' == 'Budi: 100'");

    // Test I4: interpolasi dengan ekspresi
    std::string codeI4 = R"(
        modul TestI4
        fungsi utama() ->
            x = 5
            "Hasil: #{x * 2}"
        rampung
    )";
    auto resI4 = engine.eval(codeI4);
    auto* sI4 = resI4 ? resI4->get_if<std::string>() : nullptr;
    CHECK(sI4 && *sI4 == "Hasil: 10", "Test I4: 'Hasil: #{x * 2}' == 'Hasil: 10'");

    // Test I5: string tanpa interpolasi tetap bekerja
    std::string codeI5 = R"(
        modul TestI5
        fungsi utama() ->
            "halo dunia"
        rampung
    )";
    auto resI5 = engine.eval(codeI5);
    auto* sI5 = resI5 ? resI5->get_if<std::string>() : nullptr;
    CHECK(sI5 && *sI5 == "halo dunia", "Test I5: 'halo dunia' == 'halo dunia'");

    std::cout << "\n🎉 ALL STRING INTERPOLATION TESTS PASSED PERFECTLY!\n";
    return 0;
}
