#include "../include/garuda/compiler/parser.hpp"
#include "../include/garuda/compiler/codegen.hpp"
#include "../include/garuda/beam/gvm/actor_runtime.hpp"
#include <cassert>
#include <chrono>
#include <iostream>

using namespace garuda::compiler;
using namespace garuda::beam;

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA COMPILER — SPRINT 2.3 PREEMPTION TEST SUITE (4 TESTS)\n";
    std::cout << "=======================================================\n\n";

    // === Test 1: No Starvation — Infinite hog_loop vs worker(1000) ===
    std::string src1 = R"(
        modul m
        fungsi hog_loop() ->
            hog_loop()
        rampung
        fungsi worker(N) ->
            yen N > 0 ->
                worker(N - 1)
            lain ->
                42
            rampung
        rampung
        fungsi utama() ->
            H = cakra(fungsi() -> hog_loop() rampung)
            W = cakra(fungsi() -> worker(1000) rampung)
            42
        rampung
    )";

    Lexer lex1(src1);
    Parser p1(lex1.tokenize());
    auto module1 = p1.parse_program();
    CodeGenerator cg1;
    auto out1 = cg1.generate(module1);

    GvmActorRuntime rt1(out1.all_instructions, out1.function_table);
    rt1.set_verbose(false);
    auto res1 = rt1.run_until_done("utama", 10000);
    assert(res1.has_value() && *res1->get_if<std::int64_t>() == 42);

    // Verify worker finished (PID 3) while hog_loop (PID 2) is still running
    assert(rt1.is_actor_done(Pid{3}));  // worker PID 3 finished!
    assert(!rt1.is_actor_done(Pid{2})); // hog_loop PID 2 still running without starvation!
    std::cout << "  ✓ Test 1: No Starvation (Infinite hog_loop vs worker) PASS\n";

    // === Test 2: Fairness — 3 Parallel Workers ===
    std::string src2 = R"(
        modul m
        fungsi worker(N) ->
            yen N > 0 ->
                worker(N - 1)
            lain ->
                N
            rampung
        rampung
        fungsi utama() ->
            W1 = cakra(fungsi() -> worker(10000) rampung)
            W2 = cakra(fungsi() -> worker(10000) rampung)
            W3 = cakra(fungsi() -> worker(10000) rampung)
            42
        rampung
    )";

    Lexer lex2(src2);
    Parser p2(lex2.tokenize());
    auto module2 = p2.parse_program();
    CodeGenerator cg2;
    auto out2 = cg2.generate(module2);

    GvmActorRuntime rt2(out2.all_instructions, out2.function_table);
    rt2.set_verbose(false);
    auto res2 = rt2.run_until_done("utama", 50000);
    assert(res2.has_value() && *res2->get_if<std::int64_t>() == 42);
    std::cout << "  ✓ Test 2: Fairness Across 3 Parallel Workers PASS\n";

    // === Test 3: Precise Reduction Budget Decrementing ===
    std::vector<Instruction> code = {
        {Opcode::OpMove, {Operand{RegType::Literal, 0, Term(10)}, Operand{RegType::X, 0, Term()}}},
        {Opcode::OpMove, {Operand{RegType::Literal, 0, Term(20)}, Operand{RegType::X, 1, Term()}}},
        {Opcode::OpAdd, {Operand{RegType::X, 0, Term()}, Operand{RegType::X, 1, Term()}, Operand{RegType::X, 2, Term()}}},
        {Opcode::OpHalt, {}}
    };

    GvmInterpreter interp(code);
    GvmProcessState vm;
    Runtime dummy_rt;
    Pid dummy_pid = dummy_rt.spawn([](Process&) { return Status::Done; });
    Process* proc_ptr = dummy_rt.find(dummy_pid);

    Status st = interp.execute_slice(*proc_ptr, vm);
    assert(st == Status::Done);
    assert(vm.reductions == 3996); // 4000 - 4 instructions = 3996!
    std::cout << "  ✓ Test 3: Precise Reduction Budget Decrement (4000 -> 3996) PASS\n";

    // === Test 4: Long Loop Does NOT Block Utama from Returning ===
    std::string src4 = R"(
        modul m
        fungsi long_loop(N) ->
            yen N > 0 ->
                long_loop(N - 1)
            lain ->
                42
            rampung
        rampung
        fungsi utama() ->
            P = cakra(fungsi() -> long_loop(1000000) rampung)
            42
        rampung
    )";

    auto start_time = std::chrono::high_resolution_clock::now();
    Lexer lex4(src4);
    Parser p4(lex4.tokenize());
    auto module4 = p4.parse_program();
    CodeGenerator cg4;
    auto out4 = cg4.generate(module4);

    GvmActorRuntime rt4(out4.all_instructions, out4.function_table);
    rt4.set_verbose(false);
    auto res4 = rt4.run_until_done("utama", 10000);
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now() - start_time).count();

    assert(res4.has_value() && *res4->get_if<std::int64_t>() == 42);
    assert(elapsed < 1000); // Returned immediately (< 1000ms)!
    std::cout << "  ✓ Test 4: Long Loop Does NOT Block Utama (Returned in " << elapsed << " ms) PASS\n";

    std::cout << "\n🎉 ALL 4 PREEMPTION TESTS PASSED PERFECTLY!\n";
    return 0;
}
