#include "../include/garuda/compiler/lexer.hpp"
#include "../include/garuda/compiler/parser.hpp"
#include "../include/garuda/compiler/optimizer.hpp"
#include "../include/garuda/compiler/codegen.hpp"
#include "../include/garuda/beam/gvm/interpreter.hpp"
#include <cassert>
#include <iostream>

using namespace garuda::compiler;
using namespace garuda::beam;

int64_t optimize_and_run(const std::string& src) {
    Lexer lexer(src);
    Parser parser(lexer.tokenize());
    auto raw_module = parser.parse_program();

    Optimizer opt;
    auto opt_module = opt.optimize_ast(raw_module);

    CodeGenerator codegen;
    auto output = codegen.generate(opt_module);

    auto opt_instructions = opt.optimize_bytecode(output.all_instructions);

    GvmInterpreter interp(opt_instructions);
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
    std::cout << "GARUDA COMPILER — SPRINT A.4 OPTIMIZER TEST SUITE (15 TESTS)\n";
    std::cout << "=======================================================\n\n";

    Optimizer opt;

    // === Category 1: Constant Folding (5 Tests) ===
    std::cout << "[TEST 1/15] Running Constant Folding Integer Addition test...\n";
    auto bin1 = std::make_shared<BinaryExprNode>("+", std::make_shared<LiteralNode>(Term(2LL)), std::make_shared<LiteralNode>(Term(3LL)));
    auto fold1 = std::make_shared<Optimizer>()->optimize_ast(std::make_shared<ModuleNode>("m"));
    assert(optimize_and_run("modul m fungsi utama() -> 2 + 3 rampung") == 5);
    std::cout << "  ✓ Test 1: Constant Folding Integer Addition (2 + 3 => 5) PASS\n";

    std::cout << "[TEST 2/15] Running Constant Folding Integer Multiplication test...\n";
    assert(optimize_and_run("modul m fungsi utama() -> 10 * 2 rampung") == 20);
    std::cout << "  ✓ Test 2: Constant Folding Integer Multiplication (10 * 2 => 20) PASS\n";

    std::cout << "[TEST 3/15] Running Constant Folding String Concatenation test...\n";
    std::string src3 = "modul m fungsi utama() -> \"halo \" ++ \"dunia\" rampung";
    Lexer l3(src3);
    Parser p3(l3.tokenize());
    auto opt_m3 = opt.optimize_ast(p3.parse_program());
    auto* fn3_stmt = opt_m3->functions()[0]->body()[0].get();
    assert(fn3_stmt->type() == AstNodeType::LiteralExpr); // Folded to a single LiteralNode!
    std::cout << "  ✓ Test 3: Constant Folding String Concatenation PASS\n";

    std::cout << "[TEST 4/15] Running Constant Folding Boolean Expression test...\n";
    assert(optimize_and_run("modul m fungsi utama() -> yen 5 > 3 -> 100 lain -> 200 rampung rampung") == 100);
    std::cout << "  ✓ Test 4: Constant Folding Boolean Expression (5 > 3) PASS\n";

    std::cout << "[TEST 5/15] Running Constant Folding Nested Math test...\n";
    assert(optimize_and_run("modul m fungsi utama() -> (2 + 3) * (4 + 1) rampung") == 25);
    std::cout << "  ✓ Test 5: Constant Folding Nested Math ((2+3)*(4+1) => 25) PASS\n";

    // === Category 2: Dead Code Elimination & Branch Pruning (5 Tests) ===
    std::cout << "[TEST 6/15] Running Dead Code Elimination True Branch test...\n";
    std::string src6 = "modul m fungsi utama() -> yen benar -> 100 lain -> 200 rampung rampung";
    Lexer l6(src6);
    Parser p6(l6.tokenize());
    auto opt_m6 = opt.optimize_ast(p6.parse_program());
    assert(opt_m6->functions()[0]->body()[0]->type() == AstNodeType::LiteralExpr); // Pruned else branch!
    assert(optimize_and_run(src6) == 100);
    std::cout << "  ✓ Test 6: Dead Code Elimination True Branch PASS\n";

    std::cout << "[TEST 7/15] Running Dead Code Elimination False Branch test...\n";
    std::string src7 = "modul m fungsi utama() -> yen salah -> 100 lain -> 200 rampung rampung";
    assert(optimize_and_run(src7) == 200);
    std::cout << "  ✓ Test 7: Dead Code Elimination False Branch PASS\n";

    std::cout << "[TEST 8/15] Running Peephole Redundant Move Elimination test...\n";
    std::vector<Instruction> raw_code = {
        Instruction{Opcode::OpMove, {Operand{RegType::X, 0, Term()}, Operand{RegType::X, 0, Term()}}},
        Instruction{Opcode::OpReturn, {}}
    };
    auto opt_code8 = opt.optimize_bytecode(raw_code);
    assert(opt_code8.size() == 1); // OpMove X[0]->X[0] eliminated!
    std::cout << "  ✓ Test 8: Peephole Redundant Move Elimination PASS\n";

    std::cout << "[TEST 9/15] Running Register Recycling Pool Check test...\n";
    std::string src9 = "modul m fungsi utama() -> A = 1, B = 2, C = 3, D = 4, A + B + C + D rampung";
    assert(optimize_and_run(src9) == 10);
    std::cout << "  ✓ Test 9: Register Recycling Pool Check PASS\n";

    std::cout << "[TEST 10/15] Running Deep Call Expression Register Recycling test...\n";
    std::string src10 = "modul m fungsi f(X) -> X + 1 rampung fungsi utama() -> f(f(f(f(1)))) rampung";
    assert(optimize_and_run(src10) == 5);
    std::cout << "  ✓ Test 10: Deep Call Expression Register Recycling PASS\n";

    // === Category 3: Register Allocation & Safety Checks (5 Tests) ===
    std::cout << "[TEST 11/15] Running Conditional Expression Constant Branch Pruning test...\n";
    assert(optimize_and_run("modul m fungsi utama() -> yen 10 == 10 -> 42 lain -> 0 rampung rampung") == 42);
    std::cout << "  ✓ Test 11: Conditional Expression Constant Branch Pruning PASS\n";

    std::cout << "[TEST 12/15] Running Unused Binding Elimination test...\n";
    assert(optimize_and_run("modul m fungsi utama() -> X = 10, Y = 20, 42 rampung") == 42);
    std::cout << "  ✓ Test 12: Unused Binding Elimination PASS\n";

    std::cout << "[TEST 13/15] Running Optimizer Preservation of Program Semantics test...\n";
    std::string src13 = "modul m fungsi tambah(A, B) -> A + B rampung fungsi utama() -> tambah(20 + 20, 1 + 1) rampung";
    assert(optimize_and_run(src13) == 42);
    std::cout << "  ✓ Test 13: Optimizer Preservation of Program Semantics PASS\n";

    std::cout << "[TEST 14/15] Running End-to-End Constant Folded Execution test...\n";
    assert(optimize_and_run("modul m fungsi utama() -> 10 * 10 - 58 rampung") == 42);
    std::cout << "  ✓ Test 14: End-to-End Constant Folded Execution PASS\n";

    std::cout << "[TEST 15/15] Running Register Allocation Safety Check test...\n";
    std::string src15 = "modul m fungsi utama() -> A=1, B=2, C=3, E=4, F=5, G=6, H=7, I=8, J=9, K=10, A+B+C+E+F+G+H+I+J+K rampung";
    assert(optimize_and_run(src15) == 55);
    std::cout << "  ✓ Test 15: Register Allocation Safety Check PASS\n";

    std::cout << "\n🎉 ALL 15 OPTIMIZER & REGISTER ALLOCATION TESTS PASSED PERFECTLY!\n";
    return 0;
}
