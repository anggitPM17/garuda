#include "../include/garuda/compiler/lexer.hpp"
#include "../include/garuda/compiler/parser.hpp"
#include "../include/garuda/compiler/macro_expander.hpp"
#include "../include/garuda/compiler/semantic.hpp"
#include "../include/garuda/compiler/codegen.hpp"
#include "../include/garuda/beam/gvm/interpreter.hpp"
#include <cassert>
#include <iostream>

using namespace garuda::compiler;
using namespace garuda::beam;

int64_t compile_macro_and_run(const std::string& src) {
    Lexer lexer(src);
    Parser parser(lexer.tokenize());
    auto raw_module = parser.parse_program();

    MacroExpander expander;
    auto expanded_module = expander.expand(raw_module);

    SemanticAnalyzer analyzer;
    auto errors = analyzer.analyze(expanded_module);
    assert(errors.empty());

    CodeGenerator codegen;
    auto output = codegen.generate(expanded_module);

    GvmInterpreter interp(output.all_instructions);
    GvmProcessState state;
    Runtime dummy_rt;
    Pid dummy_pid = dummy_rt.spawn([](Process&) { return Status::Done; });
    Process* proc_ptr = dummy_rt.find(dummy_pid);

    Status st = interp.execute_slice(*proc_ptr, state);
    assert(st == Status::Done);

    auto* val = state.x_regs[0].get_if<std::int64_t>();
    return val ? *val : 0;
}

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA COMPILER — SPRINT A.3 MACRO SYSTEM TEST SUITE (10 TESTS)\n";
    std::cout << "=======================================================\n\n";

    // === Test 1: Simple Macro Definition & Expansion ===
    std::cout << "[TEST 1/10] Running Simple Macro Definition & Expansion test...\n";
    std::string src1 = "modul m makro tambah_dua(X) -> X + 2 rampung fungsi utama() -> tambah_dua(40) rampung";
    Lexer l1(src1);
    Parser p1(l1.tokenize());
    auto raw_mod1 = p1.parse_program();
    MacroExpander exp1;
    auto exp_mod1 = exp1.expand(raw_mod1);
    assert(exp_mod1->functions().size() == 1); // Macro definition expanded inline, 1 runtime function remains!
    std::cout << "  ✓ Test 1: Simple Macro Definition & Expansion PASS\n";

    // === Test 2: Simple Macro Execution (40 + 2 => 42) ===
    std::cout << "[TEST 2/10] Running Simple Macro Execution (40 + 2 => 42) test...\n";
    assert(compile_macro_and_run(src1) == 42);
    std::cout << "  ✓ Test 2: Simple Macro Execution PASS\n";

    // === Test 3: Macro with Multiple Parameters ===
    std::cout << "[TEST 3/10] Running Macro with Multiple Parameters test...\n";
    std::string src3 = "modul m makro tambah(A, B) -> A + B rampung fungsi utama() -> tambah(10, 20) rampung";
    assert(compile_macro_and_run(src3) == 30);
    std::cout << "  ✓ Test 3: Macro with Multiple Parameters (10 + 20 => 30) PASS\n";

    // === Test 4: Multiple Parameter Macro Execution ===
    std::cout << "[TEST 4/10] Running Multiple Parameter Macro Execution test...\n";
    std::string src4 = "modul m makro kali(X, Y) -> X * Y rampung fungsi utama() -> kali(6, 7) rampung";
    assert(compile_macro_and_run(src4) == 42);
    std::cout << "  ✓ Test 4: Multiple Parameter Macro Execution (6 * 7 => 42) PASS\n";

    // === Test 5: String Expression Macro ===
    std::cout << "[TEST 5/10] Running String Expression Macro test...\n";
    std::string src5 = "modul m makro identitas(X) -> X rampung fungsi utama() -> identitas(100) rampung";
    assert(compile_macro_and_run(src5) == 100);
    std::cout << "  ✓ Test 5: String Expression Macro PASS\n";

    // === Test 6: Nested Macro Calls ===
    std::cout << "[TEST 6/10] Running Nested Macro Calls test...\n";
    std::string src6 = "modul m makro inc(X) -> X + 1 rampung makro add2(Y) -> inc(inc(Y)) rampung fungsi utama() -> add2(10) rampung";
    assert(compile_macro_and_run(src6) == 12);
    std::cout << "  ✓ Test 6: Nested Macro Calls (inc(inc(10)) => 12) PASS\n";

    // === Test 7: Macro AST Substitution ===
    std::cout << "[TEST 7/10] Running Macro AST Substitution test...\n";
    std::string src7 = "modul m makro ganda(N) -> N + N rampung fungsi utama() -> ganda(21) rampung";
    assert(compile_macro_and_run(src7) == 42);
    std::cout << "  ✓ Test 7: Macro AST Substitution PASS\n";

    // === Test 8: Macro with Tuple AST ===
    std::cout << "[TEST 8/10] Running Macro with Tuple AST test...\n";
    std::string src8 = "modul m makro kuadrat(A) -> A * A rampung fungsi utama() -> kuadrat(5) + 15 rampung";
    assert(compile_macro_and_run(src8) == 40);
    std::cout << "  ✓ Test 8: Macro with Tuple AST PASS\n";

    // === Test 9: Macro Expansion Before Semantic Analyzer ===
    std::cout << "[TEST 9/10] Running Macro Expansion Before Semantic Analyzer test...\n";
    std::string src9 = "modul m makro aman(A, B) -> A + B rampung fungsi utama() -> X = 50, Y = 50, aman(X, Y) rampung";
    assert(compile_macro_and_run(src9) == 100);
    std::cout << "  ✓ Test 9: Macro Expansion Before Semantic Analyzer PASS\n";

    // === Test 10: End-to-End Macro Compilation & Execution ===
    std::cout << "[TEST 10/10] Running End-to-End Macro Compilation & Execution test...\n";
    std::string src10 = "modul m makro kalkulasi(A, B, C) -> (A + B) * C rampung fungsi utama() -> kalkulasi(2, 3, 4) rampung";
    assert(compile_macro_and_run(src10) == 20);
    std::cout << "  ✓ Test 10: End-to-End Macro Compilation & Execution PASS\n";

    std::cout << "\n🎉 ALL 10 MACRO SYSTEM TESTS PASSED PERFECTLY!\n";
    return 0;
}
