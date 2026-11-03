// Garuda Language Compiler: VM BIF Bridge Test Suite (`test_vm_bif_grd.cpp`)
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
    std::cout << "GARUDA COMPILER — VM BIF BRIDGE TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test BIF1: Bif.bif_terdaftar("String.panjang") == benar
    garuda::beam::ScriptEngine engine1;
    std::string code1 = R"(
        modul TestBIF1
        impor "src/bif.grd"

        fungsi utama() ->
            Bif.bif_terdaftar("String.panjang")
        rampung
    )";
    auto res1 = engine1.eval(code1);
    bool is_ok1 = res1 && garuda::beam::to_string(*res1) == "true";
    CHECK(is_ok1, "Test BIF1: Bif.bif_terdaftar('String.panjang') == true");

    // Test BIF2: Bif.panggil_bif("String.panjang", [nilai_str("halo")]) -> {:VInt, 4}
    garuda::beam::ScriptEngine engine2;
    std::string code2 = R"(
        modul TestBIF2
        impor "src/memory.grd"
        impor "src/bif.grd"
        impor "std/tuple.grd"

        fungsi utama() ->
            Args = [Memory.nilai_str("halo")]
            Res = Bif.panggil_bif("String.panjang", Args)
            Tuple.elemen(Res, 1)
        rampung
    )";
    auto res2 = engine2.eval(code2);
    bool is_ok2 = res2 && garuda::beam::to_string(*res2) == "4";
    CHECK(is_ok2, "Test BIF2: Bif.panggil_bif('String.panjang', ['halo']) == 4");

    std::cout << "\n🎉 ALL VM BIF BRIDGE TESTS PASSED PERFECTLY!\n";
    return 0;
}
