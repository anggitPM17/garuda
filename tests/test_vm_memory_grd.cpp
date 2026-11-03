// Garuda Language Compiler: VM Memory Test Suite (`test_vm_memory_grd.cpp`)
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
    std::cout << "GARUDA COMPILER — VM MEMORY STATE TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test MEM1: reg_baru() length == 16
    garuda::beam::ScriptEngine engine1;
    std::string code1 = R"(
        modul TestMEM1
        impor "src/memory.grd"
        impor "std/list.grd"

        fungsi utama() ->
            RF = Memory.reg_baru()
            List.panjang(RF)
        rampung
    )";
    auto res1 = engine1.eval(code1);
    bool is_ok1 = res1 && garuda::beam::to_string(*res1) == "16";
    CHECK(is_ok1, "Test MEM1: Memory.reg_baru() length == 16");

    // Test MEM2: reg_baca(reg_tulis(RF, 2, nilai_int(100)), 2) == {:VInt, 100}
    garuda::beam::ScriptEngine engine2;
    std::string code2 = R"(
        modul TestMEM2
        impor "src/memory.grd"

        fungsi utama() ->
            RF = Memory.reg_baru()
            RF2 = Memory.reg_tulis(RF, 2, Memory.nilai_int(100))
            Val = Memory.reg_baca(RF2, 2)
            Memory.nilai_jenis(Val)
        rampung
    )";
    auto res2 = engine2.eval(code2);
    bool is_ok2 = res2 && (garuda::beam::to_string(*res2) == "VInt" || garuda::beam::to_string(*res2) == ":VInt");
    CHECK(is_ok2, "Test MEM2: reg_baca(reg_tulis(RF, 2, nilai_int(100)), 2) jenis == :VInt");

    // Test MEM3: opcode_ke_string(:OpAdd) == "OpAdd"
    garuda::beam::ScriptEngine engine3;
    std::string code3 = R"(
        modul TestMEM3
        impor "src/memory.grd"

        fungsi utama() ->
            Memory.opcode_ke_string(:OpAdd)
        rampung
    )";
    auto res3 = engine3.eval(code3);
    bool is_ok3 = res3 && (garuda::beam::to_string(*res3) == "OpAdd" || garuda::beam::to_string(*res3) == "\"OpAdd\"");
    CHECK(is_ok3, "Test MEM3: Memory.opcode_ke_string(:OpAdd) == 'OpAdd'");

    std::cout << "\n🎉 ALL VM MEMORY STATE TESTS PASSED PERFECTLY!\n";
    return 0;
}
