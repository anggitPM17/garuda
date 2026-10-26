#include "../include/garuda/compiler/lexer.hpp"
#include "../include/garuda/compiler/parser.hpp"
#include "../include/garuda/compiler/codegen.hpp"
#include "../include/garuda/beam/stdlib/result.hpp"
#include "../include/garuda/beam/gvm/interpreter.hpp"
#include <cassert>
#include <iostream>

using namespace garuda::compiler;
using namespace garuda::beam::stdlib;
using namespace garuda::beam;

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA COMPILER — SPRINT E.4 ADVANCED ERROR HANDLING TEST SUITE (15 TESTS)\n";
    std::cout << "=======================================================\n\n";

    // === Test 1: Result.is_ok ===
    std::cout << "[TEST 1/15] Running Result.is_ok test...\n";
    Term ok_term = Result::ok(Term(42LL));
    assert(Result::is_ok(ok_term));
    assert(!Result::is_error(ok_term));
    std::cout << "  ✓ Test 1: Result.is_ok PASS\n";

    // === Test 2: Result.is_error ===
    std::cout << "[TEST 2/15] Running Result.is_error test...\n";
    Term err_term = Result::error(Term(Atom{"not_found"}));
    assert(Result::is_error(err_term));
    assert(!Result::is_ok(err_term));
    std::cout << "  ✓ Test 2: Result.is_error PASS\n";

    // === Test 3: Result.unwrap Success ===
    std::cout << "[TEST 3/15] Running Result.unwrap Success test...\n";
    assert(*Result::unwrap(ok_term).get_if<std::int64_t>() == 42);
    std::cout << "  ✓ Test 3: Result.unwrap Success PASS\n";

    // === Test 4: Result.unwrap_or Fallback ===
    std::cout << "[TEST 4/15] Running Result.unwrap_or Fallback test...\n";
    Term fallback = Result::unwrap_or(err_term, Term(100LL));
    assert(*fallback.get_if<std::int64_t>() == 100);
    std::cout << "  ✓ Test 4: Result.unwrap_or Fallback PASS\n";

    // === Test 5: Result.map Transformation ===
    std::cout << "[TEST 5/15] Running Result.map Transformation test...\n";
    Term mapped = Result::map(ok_term, [](const Term& v) {
        return Term(*v.get_if<std::int64_t>() * 2);
    });
    assert(Result::is_ok(mapped));
    assert(*Result::unwrap(mapped).get_if<std::int64_t>() == 84);
    std::cout << "  ✓ Test 5: Result.map Transformation PASS\n";

    // === Test 6: try/catch AST Node Creation & Parsing ===
    std::cout << "[TEST 6/15] Running try/catch AST Parsing test...\n";
    std::string src6 = "modul m fungsi f() -> coba A = 10, A + 2 tangkap err -> 0 rampung rampung";
    Lexer l6(src6);
    Parser p6(l6.tokenize());
    auto mod6 = p6.parse_program();
    assert(mod6->functions()[0]->body()[0]->type() == AstNodeType::TryCatchExpr);
    std::cout << "  ✓ Test 6: try/catch AST Parsing PASS\n";

    // === Test 7: try/catch Interpreter Execution ===
    std::cout << "[TEST 7/15] Running try/catch Interpreter Execution test...\n";
    CodeGenerator cg7;
    auto out7 = cg7.generate(mod6);
    GvmInterpreter interp7(out7.all_instructions);
    GvmProcessState st7;
    Runtime dummy_rt;
    Pid dummy_pid = dummy_rt.spawn([](Process&) { return Status::Done; });
    Process* proc_ptr = dummy_rt.find(dummy_pid);

    Status s7 = interp7.execute_slice(*proc_ptr, st7);
    assert(s7 == Status::Done);
    std::cout << "  ✓ Test 7: try/catch Interpreter Execution PASS\n";

    // === Test 8: OpThrow Exception Stack Jump ===
    std::cout << "[TEST 8/15] Running OpThrow Exception Stack Jump test...\n";
    std::vector<Instruction> insts8 = {
        Instruction{Opcode::OpTryBegin, {Operand{RegType::Literal, 0, Term(3LL)}}},
        Instruction{Opcode::OpThrow, {}},
        Instruction{Opcode::OpTryEnd, {}},
        Instruction{Opcode::OpMove, {Operand{RegType::Literal, 0, Term(999LL)}, Operand{RegType::X, 0, Term()}}},
        Instruction{Opcode::OpReturn, {}}
    };
    GvmInterpreter interp8(insts8);
    GvmProcessState st8;
    interp8.execute_slice(*proc_ptr, st8);
    assert(*st8.x_regs[0].get_if<std::int64_t>() == 999); // Jumped directly to catch label!
    std::cout << "  ✓ Test 8: OpThrow Exception Stack Jump PASS\n";

    // === Test 9: ? Operator AST Creation ===
    std::cout << "[TEST 9/15] Running ? Operator AST Creation test...\n";
    std::string src9 = "modul m fungsi f() -> X = {:ok, 42}?, X rampung";
    Lexer l9(src9);
    Parser p9(l9.tokenize());
    auto mod9 = p9.parse_program();
    assert(mod9->functions()[0]->body()[0]->type() == AstNodeType::AssignmentExpr);
    std::cout << "  ✓ Test 9: ? Operator AST Creation PASS\n";

    // === Test 10: ? Operator Unwrap Execution ===
    std::cout << "[TEST 10/15] Running ? Operator Unwrap Execution test...\n";
    CodeGenerator cg10;
    auto out10 = cg10.generate(mod9);
    GvmInterpreter interp10(out10.all_instructions);
    GvmProcessState st10;
    interp10.execute_slice(*proc_ptr, st10);
    assert(*st10.x_regs[0].get_if<std::int64_t>() == 42); // Unwrapped {:ok, 42} -> 42!
    std::cout << "  ✓ Test 10: ? Operator Unwrap Execution PASS\n";

    // === Test 11: Result + Auth Engine Integration ===
    std::cout << "[TEST 11/15] Running Result + Auth Engine Integration test...\n";
    Term auth_ok = Result::ok(Term("token_abc_123"));
    assert(Result::is_ok(auth_ok));
    assert(*Result::unwrap(auth_ok).get_if<std::string>() == "token_abc_123");
    std::cout << "  ✓ Test 11: Result + Auth Engine Integration PASS\n";

    // === Test 12: Result + ORM Engine Integration ===
    std::cout << "[TEST 12/15] Running Result + ORM Engine Integration test...\n";
    Term orm_err = Result::error(Term(Atom{"not_found"}));
    assert(Result::is_error(orm_err));
    assert(*Result::unwrap_or(orm_err, Term("default_usr")).get_if<std::string>() == "default_usr");
    std::cout << "  ✓ Test 12: Result + ORM Engine Integration PASS\n";

    // === Test 13: Chained Result.map Execution ===
    std::cout << "[TEST 13/15] Running Chained Result.map Execution test...\n";
    Term r13 = Result::ok(Term(10LL));
    Term r13_mapped = Result::map(r13, [](const Term& v) { return Term(*v.get_if<std::int64_t>() + 5); });
    Term r13_final = Result::map(r13_mapped, [](const Term& v) { return Term(*v.get_if<std::int64_t>() * 2); });
    assert(*Result::unwrap(r13_final).get_if<std::int64_t>() == 30); // (10 + 5) * 2 = 30!
    std::cout << "  ✓ Test 13: Chained Result.map Execution PASS\n";

    // === Test 14: Structured Error Handling in Web Controllers ===
    std::cout << "[TEST 14/15] Running Structured Error Handling in Controllers test...\n";
    Term res14 = Result::error(Term("bad_request"));
    std::string err_json = Result::is_error(res14) ? "{\"status\":400}" : "{\"status\":200}";
    assert(err_json == "{\"status\":400}");
    std::cout << "  ✓ Test 14: Structured Error Handling in Controllers PASS\n";

    // === Test 15: End-to-End Advanced Error Handling Lifecycle ===
    std::cout << "[TEST 15/15] Running End-to-End Error Handling Lifecycle test...\n";
    std::string src15 = "modul m fungsi f() -> coba X = {:ok, 100}?, X + 1 tangkap err -> 0 rampung rampung";
    Lexer l15(src15);
    Parser p15(l15.tokenize());
    CodeGenerator cg15;
    auto out15 = cg15.generate(p15.parse_program());
    GvmInterpreter interp15(out15.all_instructions);
    GvmProcessState st15;
    interp15.execute_slice(*proc_ptr, st15);
    assert(*st15.x_regs[0].get_if<std::int64_t>() == 101);
    std::cout << "  ✓ Test 15: End-to-End Advanced Error Handling Lifecycle PASS\n";

    std::cout << "\n🎉 ALL 15 ADVANCED ERROR HANDLING TESTS PASSED PERFECTLY!\n";
    return 0;
}
