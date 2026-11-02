// Garuda Language Compiler: Guard Clause (`nalika`) Test Suite (`test_guard.cpp`)
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
    std::cout << "GARUDA COMPILER — GUARD CLAUSE (nalika) TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test G1: guard benar, fungsi dieksekusi (bagi(10, 2) == 5)
    std::string codeG1 = R"(
        modul TestG1
        fungsi bagi(A, B) nalika B != 0 -> A / B rampung
        fungsi utama() -> bagi(10, 2) rampung
    )";
    auto resG1 = engine.eval(codeG1);
    auto* valG1 = resG1 ? resG1->get_if<std::int64_t>() : nullptr;
    CHECK(valG1 && *valG1 == 5, "Test G1: bagi(10, 2) nalika B != 0 == 5");

    // Test G2: guard salah, fungsi mengembalikan nil (bagi(10, 0) == nil)
    std::string codeG2 = R"(
        modul TestG2
        fungsi bagi(A, B) nalika B != 0 -> A / B rampung
        fungsi utama() -> bagi(10, 0) rampung
    )";
    auto resG2 = engine.eval(codeG2);
    bool is_nilG2 = resG2 && garuda::beam::to_string(*resG2) == "nil";
    CHECK(is_nilG2, "Test G2: bagi(10, 0) nalika B != 0 == nil");

    // Test G3: guard dengan kondisi komparasi list
    std::string codeG3 = R"(
        modul TestG3
        impor "std/list.grd"
        fungsi ambil_positif(L) nalika List.panjang(L) > 0 -> List.kepala(L) rampung
        fungsi utama() -> ambil_positif([1, 2, 3]) rampung
    )";
    auto resG3 = engine.eval(codeG3);
    auto* valG3 = resG3 ? resG3->get_if<std::int64_t>() : nullptr;
    CHECK(valG3 && *valG3 == 1, "Test G3: ambil_positif([1,2,3]) nalika len > 0 == 1");

    // Test G4: guard dengan kondisi parameter >= 0
    std::string codeG4 = R"(
        modul TestG4
        fungsi akar(X) nalika X >= 0 -> X rampung
        fungsi utama() -> akar(9) rampung
    )";
    auto resG4 = engine.eval(codeG4);
    auto* valG4 = resG4 ? resG4->get_if<std::int64_t>() : nullptr;
    CHECK(valG4 && *valG4 == 9, "Test G4: akar(9) nalika X >= 0 == 9");

    // Test G5: fungsi tanpa guard tetap bekerja secara normal
    std::string codeG5 = R"(
        modul TestG5
        fungsi tambah(A, B) -> A + B rampung
        fungsi utama() -> tambah(2, 3) rampung
    )";
    auto resG5 = engine.eval(codeG5);
    auto* valG5 = resG5 ? resG5->get_if<std::int64_t>() : nullptr;
    CHECK(valG5 && *valG5 == 5, "Test G5: tambah(2, 3) tanpa guard == 5");

    std::cout << "\n🎉 ALL GUARD CLAUSE (nalika) TESTS PASSED PERFECTLY!\n";
    return 0;
}
