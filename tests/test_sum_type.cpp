// Garuda Language Compiler: Sum Types Test Suite (`test_sum_type.cpp`)
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
    std::cout << "GARUDA COMPILER — SUM TYPES TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test ST1: Pattern matching on Tagged Tuple {:TNumber, N}
    std::string code1 = R"(
        modul TestST1
        impor "std/string.grd"

        fungsi proses_token(Tok) ->
            tinjau Tok ing
                {:TNumber, N} -> N * 2
                {:TString, S} -> String.panjang(S)
                _ -> 0
            rampung
        rampung

        fungsi utama() ->
            Tok1 = {:TNumber, 21}
            proses_token(Tok1)
        rampung
    )";
    auto res1 = engine.eval(code1);
    bool is_ok1 = res1 && garuda::beam::to_string(*res1) == "42";
    CHECK(is_ok1, "Test ST1: Sum type {:TNumber, 21} -> 42");

    // Test ST2: Pattern matching on Tagged Tuple {:TString, S}
    std::string code2 = R"(
        modul TestST2
        impor "std/string.grd"

        fungsi proses_token(Tok) ->
            tinjau Tok ing
                {:TNumber, N} -> N * 2
                {:TString, S} -> String.panjang(S)
                _ -> 0
            rampung
        rampung

        fungsi utama() ->
            Tok2 = {:TString, "Garuda"}
            proses_token(Tok2)
        rampung
    )";
    auto res2 = engine.eval(code2);
    bool is_ok2 = res2 && garuda::beam::to_string(*res2) == "6";
    CHECK(is_ok2, "Test ST2: Sum type {:TString, 'Garuda'} -> 6");

    std::cout << "\n🎉 ALL SUM TYPE TESTS PASSED PERFECTLY!\n";
    return 0;
}
