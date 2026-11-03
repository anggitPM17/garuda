// Garuda Language Compiler: VM Evaluator Test Suite (`test_vm_eval_grd.cpp`)
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
    std::cout << "GARUDA COMPILER — VM EVALUATOR TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test EV1: exec_add(RF, 0, 1, 2) -> reg[2] == {:VInt, 30}
    garuda::beam::ScriptEngine engine1;
    std::string code1 = R"(
        modul TestEV1
        impor "src/memory.grd"
        impor "src/eval.grd"
        impor "std/tuple.grd"

        fungsi utama() ->
            RF = Memory.reg_baru()
            RF1 = Memory.reg_tulis(RF, 0, Memory.nilai_int(10))
            RF2 = Memory.reg_tulis(RF1, 1, Memory.nilai_int(20))
            RF3 = Eval.exec_add(RF2, 0, 1, 2)
            Val = Memory.reg_baca(RF3, 2)
            Tuple.elemen(Val, 1)
        rampung
    )";
    auto res1 = engine1.eval(code1);
    bool is_ok1 = res1 && garuda::beam::to_string(*res1) == "30";
    CHECK(is_ok1, "Test EV1: Eval.exec_add(RF, 0, 1, 2) == 30");

    // Test EV2: exec_div(RF, 0, 1, 2) with div-by-zero -> reg[2] == {:VAtom, "error_div"}
    garuda::beam::ScriptEngine engine2;
    std::string code2 = R"(
        modul TestEV2
        impor "src/memory.grd"
        impor "src/eval.grd"
        impor "std/tuple.grd"

        fungsi utama() ->
            RF = Memory.reg_baru()
            RF1 = Memory.reg_tulis(RF, 0, Memory.nilai_int(10))
            RF2 = Memory.reg_tulis(RF1, 1, Memory.nilai_int(0))
            RF3 = Eval.exec_div(RF2, 0, 1, 2)
            Val = Memory.reg_baca(RF3, 2)
            Tuple.elemen(Val, 1)
        rampung
    )";
    auto res2 = engine2.eval(code2);
    bool is_ok2 = res2 && (garuda::beam::to_string(*res2) == "error_div" || garuda::beam::to_string(*res2) == ":error_div" || garuda::beam::to_string(*res2) == "\"error_div\"");
    CHECK(is_ok2, "Test EV2: Eval.exec_div(RF, 0, 1, 2) div-by-zero == 'error_div'");

    std::cout << "\n🎉 ALL VM EVALUATOR TESTS PASSED PERFECTLY!\n";
    return 0;
}
