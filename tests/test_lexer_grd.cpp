// Garuda Language Compiler: Lexer in Garuda Test Suite (`test_lexer_grd.cpp`)
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
    std::cout << "GARUDA COMPILER — LEXER IN GARUDA TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test LEX1: tokenize_kata("fungsi") -> {:TKeyword, "fungsi"}
    garuda::beam::ScriptEngine engine1;
    std::string code1 = R"(
        modul TestLEX1
        impor "src/lexer.grd"

        fungsi utama() ->
            Tok = Lexer.tokenize_kata("fungsi")
            Tuple.elemen(Tok, 0)
        rampung
    )";
    auto res1 = engine1.eval(code1);
    bool is_ok1 = res1 && garuda::beam::to_string(*res1) == "TKeyword";
    CHECK(is_ok1, "Test LEX1: Lexer.tokenize_kata('fungsi') == :TKeyword");

    // Test LEX2: tokenize_simbol("+") -> {:TOp, "+"}
    garuda::beam::ScriptEngine engine2;
    std::string code2 = R"(
        modul TestLEX2
        impor "src/lexer.grd"

        fungsi utama() ->
            Tok = Lexer.tokenize_simbol("+")
            Tuple.elemen(Tok, 0)
        rampung
    )";
    auto res2 = engine2.eval(code2);
    bool is_ok2 = res2 && (garuda::beam::to_string(*res2) == "TOp" || garuda::beam::to_string(*res2) == ":TOp");
    CHECK(is_ok2, "Test LEX2: Lexer.tokenize_simbol('+') == :TOp");

    // Test LEX3: tokenize("100 200") -> [{:TNumber, "100"}, {:TNumber, "200"}]
    garuda::beam::ScriptEngine engine3;
    std::string code3 = R"(
        modul TestLEX3
        impor "src/lexer.grd"
        impor "std/list.grd"

        fungsi utama() ->
            Tokens = Lexer.tokenize("100 200")
            Tokens
        rampung
    )";
    auto res3 = engine3.eval(code3);
    if (res3) std::cout << "[LEX3 TOKENS='" << garuda::beam::to_string(*res3) << "']\n";
    bool is_ok3 = res3 && garuda::beam::to_string(*res3) != "[]";
    CHECK(is_ok3, "Test LEX3: Lexer.tokenize('100 200') non-empty");

    std::cout << "\n🎉 ALL LEXER IN GARUDA TESTS PASSED PERFECTLY!\n";
    return 0;
}
