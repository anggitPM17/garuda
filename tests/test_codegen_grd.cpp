// Garuda Language Compiler: Codegen in Garuda Test Suite (`test_codegen_grd.cpp`)
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
    garuda::beam::ScriptEngine engine1;

    std::cout << "=======================================================\n";
    std::cout << "GARUDA COMPILER — CODEGEN IN GARUDA TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test CG1: Codegen.gen_add(0, 1, 2) -> {:OpAdd, 0, 1, 2}
    std::string code1 = R"(
        modul TestCG1
        impor "src/codegen.grd"

        fungsi utama() ->
            Op = Codegen.gen_add(0, 1, 2)
            Tuple.elemen(Op, 0)
        rampung
    )";
    auto res1 = engine1.eval(code1);
    bool is_ok1 = res1 && garuda::beam::to_string(*res1) == "OpAdd";
    CHECK(is_ok1, "Test CG1: Codegen.gen_add(0, 1, 2) == :OpAdd");

    // Test CG2: Codegen.gen_return(0) -> {:OpReturn, 0}
    garuda::beam::ScriptEngine engine2;
    std::string code2 = R"(
        modul TestCG2
        impor "src/codegen.grd"

        fungsi utama() ->
            Op = Codegen.gen_return(0)
            Tuple.elemen(Op, 0)
        rampung
    )";
    auto res2 = engine2.eval(code2);
    bool is_ok2 = res2 && garuda::beam::to_string(*res2) == "OpReturn";
    CHECK(is_ok2, "Test CG2: Codegen.gen_return(0) == :OpReturn");

    std::cout << "\n🎉 ALL CODEGEN IN GARUDA TESTS PASSED PERFECTLY!\n";
    return 0;
}
