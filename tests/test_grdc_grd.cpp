// Garuda Language Compiler: Compiler Driver Test Suite (`test_grdc_grd.cpp`)
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
    std::cout << "GARUDA COMPILER — COMPILER DRIVER (grdc.grd) TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test DRV1: Grdc.kompilasi("42") -> {:OpMove, "42", 0}
    std::string code1 = R"(
        modul TestDRV1
        impor "src/grdc.grd"

        fungsi utama() ->
            Op = Grdc.kompilasi("42")
            Tuple.elemen(Op, 0)
        rampung
    )";
    auto res1 = engine.eval(code1);
    bool is_ok1 = res1 && garuda::beam::to_string(*res1) == "OpMove";
    CHECK(is_ok1, "Test DRV1: Grdc.kompilasi('42') == :OpMove");

    std::cout << "\n🎉 ALL COMPILER DRIVER TESTS PASSED PERFECTLY!\n";
    return 0;
}
