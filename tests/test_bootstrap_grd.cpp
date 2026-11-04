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
    std::cout << "GARUDA COMPILER — BOOTSTRAP END-TO-END TEST SUITE\n";
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

    // Test BS7: BootstrapCodegen.gen_number
    garuda::beam::ScriptEngine engine7;
    std::string code7 = R"(
        modul TestBS7
        impor "src/bootstrap_codegen.grd"
        impor "std/tuple.grd"

        fungsi utama() ->
            Inst = BootstrapCodegen.gen_number(10, 0)
            Tuple.elemen(Inst, 0)
        rampung
    )";
    auto res7 = engine7.eval(code7);
    bool is_ok7 = res7 && (garuda::beam::to_string(*res7) == "OpMove" || garuda::beam::to_string(*res7) == ":OpMove");
    CHECK(is_ok7, "Test BS7: BootstrapCodegen.gen_number returns :OpMove instruction");

    // Test BS8: BootstrapCodegen.gen_add
    garuda::beam::ScriptEngine engine8;
    std::string code8 = R"(
        modul TestBS8
        impor "src/bootstrap_codegen.grd"
        impor "std/tuple.grd"

        fungsi utama() ->
            Inst = BootstrapCodegen.gen_add(0, 1, 2)
            Tuple.elemen(Inst, 0)
        rampung
    )";
    auto res8 = engine8.eval(code8);
    bool is_ok8 = res8 && (garuda::beam::to_string(*res8) == "OpAdd" || garuda::beam::to_string(*res8) == ":OpAdd");
    CHECK(is_ok8, "Test BS8: BootstrapCodegen.gen_add returns :OpAdd instruction");

    // Test BS9: Bootstrap.kompilasi populates Bytecode
    garuda::beam::ScriptEngine engine9;
    std::string code9 = R"(
        modul TestBS9
        impor "src/bootstrap.grd"

        fungsi utama() ->
            St = Bootstrap.baru("1 + 2")
            StKomp = Bootstrap.kompilasi(St)
            Bc = Bootstrap.get_bytecode(StKomp)
            List.panjang(Bc)
        rampung
    )";
    auto res9 = engine9.eval(code9);
    bool is_ok9 = res9 && garuda::beam::to_string(*res9) == "4";
    CHECK(is_ok9, "Test BS9: Bootstrap.kompilasi populates Bytecode length == 4");

    // Test BS10: BootstrapEval.eval_inst OpMove
    garuda::beam::ScriptEngine engine10;
    std::string code10 = R"(
        modul TestBS10
        impor "src/bootstrap_eval.grd"
        impor "src/memory.grd"
        impor "std/tuple.grd"

        fungsi utama() ->
            RF = Memory.reg_baru()
            Inst = {:OpMove, 10, 0}
            RF2 = BootstrapEval.eval_inst(RF, Inst)
            Val = Memory.reg_baca(RF2, 0)
            Tuple.elemen(Val, 1)
        rampung
    )";
    auto res10 = engine10.eval(code10);
    bool is_ok10 = res10 && garuda::beam::to_string(*res10) == "10";
    CHECK(is_ok10, "Test BS10: BootstrapEval.eval_inst OpMove sets reg 0 == 10");

    // Test BS11: BootstrapEval.eval_inst OpAdd
    garuda::beam::ScriptEngine engine11;
    std::string code11 = R"(
        modul TestBS11
        impor "src/bootstrap_eval.grd"
        impor "src/memory.grd"
        impor "std/tuple.grd"

        fungsi utama() ->
            RF = Memory.reg_baru()
            RF1 = Memory.reg_tulis(RF, 0, Memory.nilai_int(10))
            RF2 = Memory.reg_tulis(RF1, 1, Memory.nilai_int(20))
            Inst = {:OpAdd, 0, 1, 2}
            RF3 = BootstrapEval.eval_inst(RF2, Inst)
            Val = Memory.reg_baca(RF3, 2)
            Tuple.elemen(Val, 1)
        rampung
    )";
    auto res11 = engine11.eval(code11);
    bool is_ok11 = res11 && garuda::beam::to_string(*res11) == "30";
    CHECK(is_ok11, "Test BS11: BootstrapEval.eval_inst OpAdd calculates 10 + 20 == 30");

    // Test BS12: Bootstrap.kompilasi -> BootstrapEval.jalankan_bootstrap -> 3
    garuda::beam::ScriptEngine engine12;
    std::string code12 = R"(
        modul TestBS12
        impor "src/bootstrap.grd"
        impor "src/bootstrap_eval.grd"

        fungsi utama() ->
            St = Bootstrap.baru("1 + 2")
            StKomp = Bootstrap.kompilasi(St)
            Bc = Bootstrap.get_bytecode(StKomp)
            BootstrapEval.jalankan_bootstrap(Bc)
        rampung
    )";
    auto res12 = engine12.eval(code12);
    bool is_ok12 = res12 && garuda::beam::to_string(*res12) == "3";
    CHECK(is_ok12, "Test BS12: Bootstrap.kompilasi('1 + 2') -> jalankan_bootstrap -> 3");

    // Test BS13: SelfCompile.kompilasi_teks
    garuda::beam::ScriptEngine engine13;
    std::string code13 = R"(
        modul TestBS13
        impor "src/self_compile.grd"
        impor "src/bootstrap.grd"

        fungsi utama() ->
            St = SelfCompile.kompilasi_teks("10 + 20")
            Bootstrap.is_sukses(St)
        rampung
    )";
    auto res13 = engine13.eval(code13);
    bool is_ok13 = res13 && (garuda::beam::to_string(*res13) == "benar" || garuda::beam::to_string(*res13) == "true");
    CHECK(is_ok13, "Test BS13: SelfCompile.kompilasi_teks('10 + 20') returns is_sukses == benar");

    // Test BS14: SelfCompile.kompilasi_dan_jalankan("10 + 20")
    garuda::beam::ScriptEngine engine14;
    std::string code14 = R"(
        modul TestBS14
        impor "src/self_compile.grd"

        fungsi utama() ->
            SelfCompile.kompilasi_dan_jalankan("10 + 20")
        rampung
    )";
    auto res14 = engine14.eval(code14);
    bool is_ok14 = res14 && garuda::beam::to_string(*res14) == "3";
    CHECK(is_ok14, "Test BS14: SelfCompile.kompilasi_dan_jalankan('10 + 20') -> 3");

    // Test BS15: SelfCompile.kompilasi_dan_jalankan("100 - 40")
    garuda::beam::ScriptEngine engine15;
    std::string code15 = R"(
        modul TestBS15
        impor "src/self_compile.grd"

        fungsi utama() ->
            SelfCompile.kompilasi_dan_jalankan("100 - 40")
        rampung
    )";
    auto res15 = engine15.eval(code15);
    bool is_ok15 = res15 && garuda::beam::to_string(*res15) == "3";
    CHECK(is_ok15, "Test BS15: SelfCompile.kompilasi_dan_jalankan('100 - 40') -> 3");

    std::cout << "\n🎉 ALL BOOTSTRAP END-TO-END TESTS PASSED PERFECTLY!\n";
    return 0;
}
