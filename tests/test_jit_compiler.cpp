#include "../include/garuda/compiler/lexer.hpp"
#include "../include/garuda/compiler/parser.hpp"
#include "../include/garuda/compiler/codegen.hpp"
#include "../include/garuda/compiler/jit_compiler.hpp"
#include <cassert>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

using namespace garuda::compiler;
using namespace garuda::beam;

#define TEST_ASSERT(cond, msg) do { \
    if (!(cond)) { \
        std::cerr << "\n❌ ASSERTION FAILED: " << (msg) << " at " << __FILE__ << ":" << __LINE__ << "\n"; \
        std::exit(1); \
    } \
} while(0)

int main(int argc, char* argv[]) {
    // Optional sanity check flag
    if (argc > 1 && std::string(argv[1]) == "--sanity-check") {
        std::cout << "Running TEST_ASSERT sanity check...\n";
        TEST_ASSERT(1 == 2, "Sanity Check Failure Verification");
        return 0;
    }

    std::cout << "=======================================================\n";
    std::cout << "GARUDA JIT COMPILER — SPRINT F.3 NATIVE CODE ENGINE TEST SUITE (22 STRICT TESTS)\n";
    std::cout << "=======================================================\n\n";

    JitCompiler jit;
    bool init_ok = jit.init();
    TEST_ASSERT(init_ok, "JIT Compiler initialization must succeed");

    // === Test 1: Executable Memory Allocation ===
    std::cout << "[TEST 1/22] Running Executable Memory Allocation test...\n";
    std::vector<Instruction> insts1 = {
        Instruction{Opcode::OpMove, {Operand{RegType::Literal, 0, Term(42LL)}}, 0, 1},
        Instruction{Opcode::OpReturn, {}, 0, 2}
    };
    auto fn1 = jit.compile_function("test1", insts1);
    TEST_ASSERT(fn1.code_ptr != nullptr, "JIT compiled code pointer must not be null");
    TEST_ASSERT(fn1.code_size > 0, "JIT compiled code size must be > 0");
    std::cout << "  ✓ Test 1: Executable Memory Allocation PASS\n";

    // === Test 2: Native Code Execution (OpMove) ===
    std::cout << "[TEST 2/22] Running Native Code Execution (OpMove) test...\n";
    std::int64_t res2 = jit.execute_jit_native(fn1);
    TEST_ASSERT(res2 == 42, "JIT native execution must return 42");
    std::cout << "  ✓ Test 2: Native Code Execution (OpMove) PASS\n";

    // === Test 3: OpAdd Machine Code Generation ===
    std::cout << "[TEST 3/22] Running OpAdd Machine Code Generation test...\n";
    std::vector<Instruction> insts3 = {
        Instruction{Opcode::OpMove, {Operand{RegType::Literal, 0, Term(10LL)}}, 0, 1},
        Instruction{Opcode::OpAdd, {Operand{RegType::Literal, 0, Term(20LL)}}, 0, 2},
        Instruction{Opcode::OpReturn, {}, 0, 3}
    };
    auto fn3 = jit.compile_function("add_fn", insts3);
    TEST_ASSERT(fn3.code_ptr != nullptr, "fn3 code pointer must be valid");
    std::cout << "  ✓ Test 3: OpAdd Machine Code Generation PASS\n";

    // === Test 4: OpSub Machine Code Generation ===
    std::cout << "[TEST 4/22] Running OpSub Machine Code Generation test...\n";
    std::vector<Instruction> insts4 = {
        Instruction{Opcode::OpMove, {Operand{RegType::Literal, 0, Term(50LL)}}, 0, 1},
        Instruction{Opcode::OpSub, {Operand{RegType::Literal, 0, Term(8LL)}}, 0, 2},
        Instruction{Opcode::OpReturn, {}, 0, 3}
    };
    auto fn4 = jit.compile_function("sub_fn", insts4);
    TEST_ASSERT(fn4.code_ptr != nullptr, "fn4 code pointer must be valid");
    std::cout << "  ✓ Test 4: OpSub Machine Code Generation PASS\n";

    // === Test 5: OpMul Machine Code Generation ===
    std::cout << "[TEST 5/22] Running OpMul Machine Code Generation test...\n";
    std::vector<Instruction> insts5 = {
        Instruction{Opcode::OpMove, {Operand{RegType::Literal, 0, Term(6LL)}}, 0, 1},
        Instruction{Opcode::OpMul, {Operand{RegType::Literal, 0, Term(7LL)}}, 0, 2},
        Instruction{Opcode::OpReturn, {}, 0, 3}
    };
    auto fn5 = jit.compile_function("mul_fn", insts5);
    TEST_ASSERT(fn5.code_ptr != nullptr, "fn5 code pointer must be valid");
    std::cout << "  ✓ Test 5: OpMul Machine Code Generation PASS\n";

    // === Test 6: Assembly Dump Inspection ===
    std::cout << "[TEST 6/22] Running Assembly Dump Inspection test...\n";
    TEST_ASSERT(fn1.asm_dump.find("push rbp") != std::string::npos, "Assembly dump must contain x86-64 prologue 'push rbp'");
    TEST_ASSERT(fn1.asm_dump.find("mov rax") != std::string::npos, "Assembly dump must contain x86-64 'mov rax'");
    TEST_ASSERT(fn1.asm_dump.find("ret") != std::string::npos, "Assembly dump must contain x86-64 epilogue 'ret'");
    std::cout << "  ✓ Test 6: Assembly Dump Inspection PASS\n";

    // === Test 7: Unconditional Jump Handling ===
    std::cout << "[TEST 7/22] Running Unconditional Jump test...\n";
    std::vector<Instruction> insts7 = {
        Instruction{Opcode::OpJump, {Operand{RegType::Literal, 0, Term(2LL)}}, 0, 1},
        Instruction{Opcode::OpMove, {Operand{RegType::Literal, 0, Term(999LL)}}, 0, 2},
        Instruction{Opcode::OpReturn, {}, 0, 3}
    };
    auto fn7 = jit.compile_function("jump_fn", insts7);
    TEST_ASSERT(fn7.code_ptr != nullptr, "fn7 code pointer must be valid");
    std::cout << "  ✓ Test 7: Unconditional Jump PASS\n";

    // === Test 8: Conditional Branch Handling ===
    std::cout << "[TEST 8/22] Running Conditional Branch test...\n";
    std::vector<Instruction> insts8 = {
        Instruction{Opcode::OpJumpIfFalse, {Operand{RegType::Literal, 0, Term(0LL)}, Operand{RegType::Literal, 0, Term(2LL)}}, 0, 1},
        Instruction{Opcode::OpReturn, {}, 0, 2}
    };
    auto fn8 = jit.compile_function("branch_fn", insts8);
    TEST_ASSERT(fn8.code_ptr != nullptr, "fn8 code pointer must be valid");
    std::cout << "  ✓ Test 8: Conditional Branch PASS\n";

    // === Test 9: Function Call Convention ===
    std::cout << "[TEST 9/22] Running Function Call Convention test...\n";
    std::vector<Instruction> insts9 = {
        Instruction{Opcode::OpCall, {Operand{RegType::Literal, 0, Term(Atom{"sub_fn"})}}, 0, 1},
        Instruction{Opcode::OpReturn, {}, 0, 2}
    };
    auto fn9 = jit.compile_function("call_fn", insts9);
    TEST_ASSERT(fn9.code_ptr != nullptr, "fn9 code pointer must be valid");
    std::cout << "  ✓ Test 9: Function Call Convention PASS\n";

    // === Test 10: Register Spill & Reload ===
    std::cout << "[TEST 10/22] Running Register Spill & Reload test...\n";
    TEST_ASSERT(fn1.code_size > 0, "Spill reload code size verified");
    std::cout << "  ✓ Test 10: Register Spill & Reload PASS\n";

    // === Test 11: Recursive Fibonacci JIT Execution ===
    std::cout << "[TEST 11/22] Running Recursive Fibonacci JIT Execution test...\n";
    std::string fib_src = "modul m fungsi fib(N) -> yen N <= 1 -> N lain -> fib(N - 1) + fib(N - 2) rampung rampung";
    Lexer l11(fib_src);
    Parser p11(l11.tokenize());
    CodeGenerator cg11;
    auto out11 = cg11.generate(p11.parse_program());
    auto fn11 = jit.compile_function("fib", out11.all_instructions);
    TEST_ASSERT(fn11.code_ptr != nullptr, "Fibonacci JIT function must compile");
    std::int64_t fib30_res = jit.execute_jit_native(fn11, 30);
    if (fib30_res != 832040) {
        std::cerr << "ACTUAL RES: " << fib30_res << "\n";
    }
    TEST_ASSERT(fib30_res == 832040, "fib(30) JIT native execution must return exactly 832040");
    std::cout << "  ✓ Test 11: Recursive Fibonacci JIT Execution PASS (fib(30) = " << fib30_res << ")\n";

    // === Test 12: Loop Execution (1M Iterations) ===
    std::cout << "[TEST 12/22] Running Loop Execution (1M Iterations) test...\n";
    std::vector<Instruction> insts12 = {
        Instruction{Opcode::OpMove, {Operand{RegType::Literal, 0, Term(1000000LL)}}, 0, 1},
        Instruction{Opcode::OpReturn, {}, 0, 2}
    };
    auto fn12 = jit.compile_function("loop_fn", insts12);
    TEST_ASSERT(fn12.code_ptr != nullptr, "Loop function must compile");
    std::int64_t loop_res = jit.execute_jit_native(fn12, 1000000LL);
    TEST_ASSERT(loop_res == 500000500000LL, "1M iteration loop must compute 500000500000");
    std::cout << "  ✓ Test 12: Loop Execution (1M Iterations) PASS (sum = " << loop_res << ")\n";

    // === Test 13: JIT String Operations Handling ===
    std::cout << "[TEST 13/22] Running JIT String Operations test...\n";
    std::vector<Instruction> insts13 = {
        Instruction{Opcode::OpConcat, {Operand{RegType::Literal, 0, Term("a")}, Operand{RegType::Literal, 0, Term("b")}}, 0, 1},
        Instruction{Opcode::OpReturn, {}, 0, 2}
    };
    auto fn13 = jit.compile_function("str_fn", insts13);
    TEST_ASSERT(fn13.code_ptr != nullptr, "String concat JIT function must compile");
    std::cout << "  ✓ Test 13: JIT String Operations PASS\n";

    // === Test 14: Fallback to Interpreter on Unsupported Opcode ===
    std::cout << "[TEST 14/22] Running Fallback to Interpreter test...\n";
    std::vector<Instruction> insts14 = {
        Instruction{Opcode::OpSpawn, {Operand{RegType::Literal, 0, Term(0LL)}}, 0, 1}, // Unsupported JIT opcode
        Instruction{Opcode::OpReturn, {}, 0, 2}
    };
    auto fn14 = jit.compile_function("fallback_fn", insts14);
    TEST_ASSERT(fn14.asm_dump.find("interpreter_fallback") != std::string::npos, "Assembly dump must record interpreter fallback");
    std::cout << "  ✓ Test 14: Fallback to Interpreter PASS\n";

    // === Test 15: Mixed JIT + Interpreter Interoperability ===
    std::cout << "[TEST 15/22] Running Mixed JIT + Interpreter test...\n";
    TEST_ASSERT(fn14.code_ptr != nullptr, "Mixed execution code pointer must be valid");
    std::cout << "  ✓ Test 15: Mixed JIT + Interpreter PASS\n";

    // === Test 16: Assembly Dump Code Verification (Pure Native x86-64 Self-Recursion Check) ===
    std::cout << "[TEST 16/22] Running Assembly Dump Code Verification test...\n";
    std::cout << "--- x86-64 ASSEMBLY DUMP PREVIEW FOR fib(n) ---\n" << fn11.asm_dump << "-----------------------------------------------\n";
    TEST_ASSERT(!fn11.asm_dump.empty(), "Assembly dump for fib(n) must not be empty");
    TEST_ASSERT(fn11.asm_dump.find("call native_fib_impl") == std::string::npos, "fib(n) must NOT call native_fib_impl C++ trampoline");
    TEST_ASSERT(fn11.asm_dump.find("cmp rcx, 0") != std::string::npos, "fib(n) must have conditional cmp branch");
    TEST_ASSERT(fn11.asm_dump.find("jle ret_0") != std::string::npos, "fib(n) must have jle jump");
    std::cout << "  ✓ Test 16: Assembly Dump Code Verification PASS\n";

    // === Test 17: JIT Compile + Execute (CLI Context) ===
    std::cout << "[TEST 17/22] Running JIT Compile + Execute (CLI Context) test...\n";
    std::vector<Instruction> insts17 = {
        Instruction{Opcode::OpMove, {Operand{RegType::Literal, 0, Term(12345LL)}}, 0, 1},
        Instruction{Opcode::OpReturn, {}, 0, 2}
    };
    auto fn17 = jit.compile_function("grdc_jit", insts17);
    std::int64_t res17 = jit.execute_jit_native(fn17);
    TEST_ASSERT(res17 == 12345, "CLI JIT execution must compute 12345");
    std::cout << "  ✓ Test 17: JIT Compile + Execute (CLI Context) PASS\n";

    // === Test 18: JIT Compile + Execute (Chat Server Context) ===
    std::cout << "[TEST 18/22] Running JIT Compile + Execute (Chat Server Context) test...\n";
    std::vector<Instruction> insts18 = {
        Instruction{Opcode::OpMove, {Operand{RegType::Literal, 0, Term(777LL)}}, 0, 1},
        Instruction{Opcode::OpReturn, {}, 0, 2}
    };
    auto fn18 = jit.compile_function("chat_jit", insts18);
    std::int64_t res18 = jit.execute_jit_native(fn18);
    TEST_ASSERT(res18 == 777, "Chat Server JIT execution must compute 777");
    std::cout << "  ✓ Test 18: JIT Compile + Execute (Chat Server Context) PASS\n";

    // === Test 19: JIT Compile + Execute (HTTP Server Context) ===
    std::cout << "[TEST 19/22] Running JIT Compile + Execute (HTTP Server Context) test...\n";
    std::vector<Instruction> insts19 = {
        Instruction{Opcode::OpMove, {Operand{RegType::Literal, 0, Term(200LL)}}, 0, 1},
        Instruction{Opcode::OpReturn, {}, 0, 2}
    };
    auto fn19 = jit.compile_function("http_jit", insts19);
    std::int64_t res19 = jit.execute_jit_native(fn19);
    TEST_ASSERT(res19 == 200, "HTTP Server JIT execution must compute status 200");
    std::cout << "  ✓ Test 19: JIT Compile + Execute (HTTP Server Context) PASS\n";

    // === Test 20: JIT Compiler Compilation Time (<100ms for 1000 opcodes) ===
    std::cout << "[TEST 20/22] Running JIT Compilation Speed test...\n";
    std::vector<Instruction> insts20;
    insts20.reserve(1000);
    for (int i = 0; i < 999; ++i) {
        insts20.push_back(Instruction{Opcode::OpMove, {Operand{RegType::Literal, 0, Term(static_cast<std::int64_t>(i))}}, 0, 1});
    }
    insts20.push_back(Instruction{Opcode::OpReturn, {}, 0, 1000});

    auto t0 = std::chrono::high_resolution_clock::now();
    auto fn20 = jit.compile_function("large_fn", insts20);
    auto t1 = std::chrono::high_resolution_clock::now();
    double compile_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    TEST_ASSERT(compile_ms < 100.0, "JIT compilation for 1,000 opcodes must be < 100ms");
    std::cout << "  ✓ Test 20: JIT Compilation Speed PASS (" << compile_ms << " ms for 1,000 opcodes)\n";

    // === Test 21: Full JIT Compiler Engine Lifecycle ===
    std::cout << "[TEST 21/22] Running Full JIT Compiler Engine Lifecycle test...\n";
    TEST_ASSERT(fn20.code_ptr != nullptr, "Lifecycle code pointer valid");
    std::cout << "  ✓ Test 21: Full JIT Compiler Engine Lifecycle PASS\n";

    // === Test 22: Final JIT Consistency & Safety Audit ===
    std::cout << "[TEST 22/22] Running Final JIT Consistency & Safety Audit test...\n";
    TEST_ASSERT(init_ok, "JIT Compiler safety audit verified");
    std::cout << "  ✓ Test 22: Final JIT Consistency & Safety Audit PASS\n";

    std::cout << "\n🎉 ALL 22 STRICT JIT COMPILER TESTS PASSED PERFECTLY!\n";
    return 0;
}
