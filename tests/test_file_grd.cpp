// Garuda Language Compiler: File Module Test Suite (`test_file_grd.cpp`)
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
    std::cout << "GARUDA COMPILER — FILE MODULE TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test F1: File.tulis lalu File.baca -> isi sama
    std::string code1 = R"(
        modul TestF1
        impor "std/file.grd"
        fungsi utama() ->
            File.tulis("test_f1_out.txt", "Konten Uji File Module")
            File.baca("test_f1_out.txt")
        rampung
    )";
    auto res1 = engine.eval(code1);
    bool is_ok1 = res1 && (garuda::beam::to_string(*res1) == "Konten Uji File Module" || garuda::beam::to_string(*res1) == "\"Konten Uji File Module\"");
    CHECK(is_ok1, "Test F1: File.tulis lalu File.baca -> 'Konten Uji File Module'");

    // Test F2: File.ada -> benar setelah tulis, salah untuk file tak ada
    std::string code2 = R"(
        modul TestF2
        impor "std/file.grd"
        fungsi utama() ->
            File.tulis("test_f2_out.txt", "ada")
            yen File.ada("test_f2_out.txt") ->
                yen File.ada("file_yang_pasti_tidak_ada.xyz") -> "salah_ada" lain -> "benar" rampung
            lain -> "salah_tidak_ada"
            rampung
        rampung
    )";
    auto res2 = engine.eval(code2);
    auto* val2 = res2 ? res2->get_if<std::string>() : nullptr;
    CHECK(val2 && *val2 == "benar", "Test F2: File.ada -> benar untuk file ada, salah untuk file tidak ada");

    // Test F3: Tulis file kosong -> baca string kosong
    std::string code3 = R"(
        modul TestF3
        impor "std/file.grd"
        fungsi utama() ->
            File.tulis("test_f3_out.txt", "")
            File.baca("test_f3_out.txt")
        rampung
    )";
    auto res3 = engine.eval(code3);
    bool is_ok3 = res3 && (garuda::beam::to_string(*res3) == "" || garuda::beam::to_string(*res3) == "\"\"");
    CHECK(is_ok3, "Test F3: Tulis file kosong -> File.baca == ''");

    std::cout << "\n🎉 ALL FILE MODULE TESTS PASSED PERFECTLY!\n";
    return 0;
}
