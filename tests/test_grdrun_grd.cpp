// Garuda Language Compiler: VM Driver & Runner Test Suite (`test_grdrun_grd.cpp`)
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
    std::cout << "GARUDA COMPILER — VM DRIVER & RUNNER TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test RUN1: Vm.eksekusi_sederhana(10, 20) -> 30
    garuda::beam::ScriptEngine engine1;
    std::string code1 = R"(
        modul TestRUN1
        impor "src/vm.grd"

        fungsi utama() ->
            Vm.eksekusi_sederhana(10, 20)
        rampung
    )";
    auto res1 = engine1.eval(code1);
    bool is_ok1 = res1 && garuda::beam::to_string(*res1) == "30";
    CHECK(is_ok1, "Test RUN1: Vm.eksekusi_sederhana(10, 20) == 30");

    // Test RUN2: Grdrun.jalankan("42") -> "42"
    garuda::beam::ScriptEngine engine2;
    std::string code2 = R"(
        modul TestRUN2
        impor "src/grdrun.grd"

        fungsi utama() ->
            Grdrun.jalankan("42")
        rampung
    )";
    auto res2 = engine2.eval(code2);
    if (res2) std::cout << "[RUN2 RES='" << garuda::beam::to_string(*res2) << "']\n";
    else std::cout << "[RUN2 RES=NULLOPT]\n";
    bool is_ok2 = res2 && (garuda::beam::to_string(*res2) == "42" || garuda::beam::to_string(*res2) == "\"42\"" || garuda::beam::to_string(*res2) == "'42'");
    CHECK(is_ok2, "Test RUN2: Grdrun.jalankan('42') == '42'");

    std::cout << "\n🎉 ALL VM DRIVER & RUNNER TESTS PASSED PERFECTLY!\n";
    return 0;
}
