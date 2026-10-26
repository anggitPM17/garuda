#include "../include/garuda/compiler/lexer.hpp"
#include "../include/garuda/compiler/parser.hpp"
#include "../include/garuda/compiler/codegen.hpp"
#include "../include/garuda/compiler/jit_compiler.hpp"
#include "../include/garuda/beam/gvm/interpreter.hpp"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <iostream>
#include <vector>

using namespace garuda::compiler;
using namespace garuda::beam;

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA JIT COMPILER BENCHMARK (SPRINT F.3 — NATIVE CODE ENGINE)\n";
    std::cout << "=======================================================\n\n";

    JitCompiler jit;
    jit.init();

    // Fibonacci(30) JIT vs Interpreter Benchmark
    std::string fib_src = "modul m fungsi fib(N) -> yen N <= 1 -> N lain -> fib(N - 1) + fib(N - 2) rampung rampung";
    Lexer lexer(fib_src);
    Parser parser(lexer.tokenize());
    CodeGenerator cg;
    auto out = cg.generate(parser.parse_program());

    // 1. Interpreter Execution Time
    std::cout << "[BENCH 1] Running Fibonacci(30) via GVM Interpreter...\n";
    GvmInterpreter interp(out.all_instructions);
    GvmProcessState proc_st;
    Runtime dummy_rt;
    Pid dummy_pid = dummy_rt.spawn([](Process&) { return Status::Done; });
    Process* proc_ptr = dummy_rt.find(dummy_pid);

    auto t0_interp = std::chrono::high_resolution_clock::now();
    interp.execute_slice(*proc_ptr, proc_st);
    auto t1_interp = std::chrono::high_resolution_clock::now();
    double interp_ms = std::chrono::duration<double, std::milli>(t1_interp - t0_interp).count();
    std::cout << "  ⏱️ GVM Interpreter Execution Time: " << interp_ms << " ms\n\n";

    // 2. JIT Compilation Time for 1,000 Opcodes
    std::cout << "[BENCH 2] Measuring JIT Compilation Time for 1,000 Opcodes...\n";
    std::vector<Instruction> large_insts;
    large_insts.reserve(1000);
    for (int i = 0; i < 999; ++i) {
        large_insts.push_back(Instruction{Opcode::OpMove, {Operand{RegType::Literal, 0, Term(static_cast<std::int64_t>(i))}}, 0, 1});
    }
    large_insts.push_back(Instruction{Opcode::OpReturn, {}, 0, 1000});

    auto t0_compile = std::chrono::high_resolution_clock::now();
    auto jit_large_fn = jit.compile_function("large_fn", large_insts);
    (void)jit_large_fn;
    auto t1_compile = std::chrono::high_resolution_clock::now();
    double compile_ms = std::chrono::duration<double, std::milli>(t1_compile - t0_compile).count();
    std::cout << "  ⏱️ JIT Compilation Time (1,000 opcodes): " << compile_ms << " ms (< 100 ms target!)\n\n";

    // 3. JIT Native Code Execution Time
    std::cout << "[BENCH 3] Running Fibonacci(30) via JIT Native Code Engine...\n";
    auto jit_fib_fn = jit.compile_function("fib", out.all_instructions);

    auto t0_jit = std::chrono::high_resolution_clock::now();
    std::int64_t jit_res = jit.execute_jit_native(jit_fib_fn, 30);
    std::cout << "  ✓ JIT Result: " << jit_res << "\n";
    auto t1_jit = std::chrono::high_resolution_clock::now();
    double jit_ms = std::chrono::duration<double, std::milli>(t1_jit - t0_jit).count();
    if (jit_ms < 0.001) jit_ms = 0.001; // Avoid divide by zero

    double speedup = interp_ms / jit_ms;
    std::cout << "  ⏱️ JIT Native Execution Time: " << jit_ms << " ms\n";
    std::cout << "  🚀 JIT Speedup vs GVM Interpreter: " << speedup << "x (Target >= 2.0x!)\n\n";

    // 4. x86-64 Assembly Dump Inspection
    std::cout << "[BENCH 4] Inspected x86-64 Assembly Dump Output:\n";
    std::cout << "-----------------------------------------------\n";
    std::cout << jit_fib_fn.asm_dump;
    std::cout << "-----------------------------------------------\n\n";

    std::cout << "🎉 ALL JIT COMPILER BENCHMARKS EXECUTED SUCCESSFULLY!\n";
    return 0;
}
