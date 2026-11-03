// Garuda Language Compiler: Parser in Garuda Test Suite (`test_parser_grd.cpp`)
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
    std::cout << "GARUDA COMPILER — PARSER IN GARUDA TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test PAR1: Parser.parse_number({:TNumber, "42"}) -> {:ANumber, "42"}
    garuda::beam::ScriptEngine engine1;
    std::string code1 = R"(
        modul TestPAR1
        impor "src/parser.grd"

        fungsi utama() ->
            Ast = Parser.parse_number({:TNumber, "42"})
            Tuple.elemen(Ast, 0)
        rampung
    )";
    auto res1 = engine1.eval(code1);
    bool is_ok1 = res1 && garuda::beam::to_string(*res1) == "ANumber";
    CHECK(is_ok1, "Test PAR1: Parser.parse_number({:TNumber, '42'}) == :ANumber");

    // Test PAR2: Parser.parse_binary("+", A, B) -> {:ABinary, "+", A, B}
    garuda::beam::ScriptEngine engine2;
    std::string code2 = R"(
        modul TestPAR2
        impor "src/parser.grd"

        fungsi utama() ->
            Ast = Parser.parse_binary("+", {:ANumber, 1}, {:ANumber, 2})
            Tuple.elemen(Ast, 0)
        rampung
    )";
    auto res2 = engine2.eval(code2);
    bool is_ok2 = res2 && garuda::beam::to_string(*res2) == "ABinary";
    CHECK(is_ok2, "Test PAR2: Parser.parse_binary('+') == :ABinary");

    std::cout << "\n🎉 ALL PARSER IN GARUDA TESTS PASSED PERFECTLY!\n";
    return 0;
}
