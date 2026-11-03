// Garuda Language Compiler: Bootstrap Compiler Test Suite (`test_bootstrap_grd.cpp`)
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
    std::cout << "GARUDA COMPILER — BOOTSTRAP LEXER & PARSER TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test BS1: Bootstrap.baru("1 + 2")
    garuda::beam::ScriptEngine engine1;
    std::string code1 = R"(
        modul TestBS1
        impor "src/bootstrap.grd"

        fungsi utama() ->
            St = Bootstrap.baru("1 + 2")
            Bootstrap.get_sumber(St)
        rampung
    )";
    auto res1 = engine1.eval(code1);
    bool is_ok1 = res1 && (garuda::beam::to_string(*res1) == "1 + 2" || garuda::beam::to_string(*res1) == "\"1 + 2\"");
    CHECK(is_ok1, "Test BS1: Bootstrap.baru('1 + 2') get_sumber == '1 + 2'");

    // Test BS2: Immutable state setters
    garuda::beam::ScriptEngine engine2;
    std::string code2 = R"(
        modul TestBS2
        impor "src/bootstrap.grd"

        fungsi utama() ->
            St0 = Bootstrap.baru("1 + 2")
            St1 = Bootstrap.set_tokens(St0, [1, 2])
            St2 = Bootstrap.set_ast(St1, "AST")
            St3 = Bootstrap.set_bytecode(St2, "BYTECODE")
            St4 = Bootstrap.set_sukses(St3, benar)
            Bootstrap.is_sukses(St4)
        rampung
    )";
    auto res2 = engine2.eval(code2);
    bool is_ok2 = res2 && (garuda::beam::to_string(*res2) == "benar" || garuda::beam::to_string(*res2) == "true");
    CHECK(is_ok2, "Test BS2: Immutable setters update state and is_sukses == benar");

    // Test BS3: Bootstrap.kompilasi(St)
    garuda::beam::ScriptEngine engine3;
    std::string code3 = R"(
        modul TestBS3
        impor "src/bootstrap.grd"

        fungsi utama() ->
            St = Bootstrap.baru("1 + 2")
            StKomp = Bootstrap.kompilasi(St)
            Bootstrap.is_sukses(StKomp)
        rampung
    )";
    auto res3 = engine3.eval(code3);
    bool is_ok3 = res3 && (garuda::beam::to_string(*res3) == "benar" || garuda::beam::to_string(*res3) == "true");
    CHECK(is_ok3, "Test BS3: Bootstrap.kompilasi(St) sets is_sukses == benar");

    // Test BS4: BootstrapLexer.tokenize_bootstrap("fungsi utama -> 123")
    garuda::beam::ScriptEngine engine4;
    std::string code4 = R"(
        modul TestBS4
        impor "src/bootstrap_lexer.grd"

        fungsi utama() ->
            Toks = BootstrapLexer.tokenize_bootstrap("fungsi utama -> 123")
            List.panjang(Toks)
        rampung
    )";
    auto res4 = engine4.eval(code4);
    bool is_ok4 = res4 && garuda::beam::to_string(*res4) == "4";
    CHECK(is_ok4, "Test BS4: BootstrapLexer.tokenize_bootstrap length == 4");

    // Test BS5: BootstrapParser.parse_bootstrap(Toks)
    garuda::beam::ScriptEngine engine5;
    std::string code5 = R"(
        modul TestBS5
        impor "src/bootstrap_lexer.grd"
        impor "src/bootstrap_parser.grd"
        impor "std/tuple.grd"

        fungsi utama() ->
            Toks = BootstrapLexer.tokenize_bootstrap("1 + 2")
            Ast = BootstrapParser.parse_bootstrap(Toks)
            Tuple.elemen(Ast, 0)
        rampung
    )";
    auto res5 = engine5.eval(code5);
    bool is_ok5 = res5 && (garuda::beam::to_string(*res5) == "AProgram" || garuda::beam::to_string(*res5) == ":AProgram");
    CHECK(is_ok5, "Test BS5: BootstrapParser.parse_bootstrap jenis AST == :AProgram");

    // Test BS6: Bootstrap.kompilasi populates Tokens & AST
    garuda::beam::ScriptEngine engine6;
    std::string code6 = R"(
        modul TestBS6
        impor "src/bootstrap.grd"

        fungsi utama() ->
            St = Bootstrap.baru("1 + 2")
            StKomp = Bootstrap.kompilasi(St)
            Toks = Bootstrap.get_tokens(StKomp)
            List.panjang(Toks)
        rampung
    )";
    auto res6 = engine6.eval(code6);
    bool is_ok6 = res6 && garuda::beam::to_string(*res6) == "3";
    CHECK(is_ok6, "Test BS6: Bootstrap.kompilasi populates Tokens length == 3");

    std::cout << "\n🎉 ALL BOOTSTRAP LEXER & PARSER TESTS PASSED PERFECTLY!\n";
    return 0;
}
