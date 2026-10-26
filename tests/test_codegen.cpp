#include "../include/garuda/compiler/parser.hpp"
#include "../include/garuda/compiler/codegen.hpp"
#include "../include/garuda/beam/gvm/interpreter.hpp"
#include <cassert>
#include <iostream>

using namespace garuda::compiler;
using namespace garuda::beam;

#define TEST_ASSERT(cond, msg) do { \
    if (!(cond)) { \
        std::cerr << "\n❌ ASSERTION FAILED: " << (msg) << " at " << __FILE__ << ":" << __LINE__ << "\n"; \
        std::exit(1); \
    } \
} while(0)

int64_t compile_and_run(const std::string& src) {
    Lexer lex(src);
    Parser p(lex.tokenize());
    auto module = p.parse_program();

    CodeGenerator cg;
    auto output = cg.generate(module);

    GvmInterpreter interp(output.all_instructions);
    GvmProcessState state;
    Runtime rt;
    Pid process_pid = rt.spawn([](Process&) { return Status::Done; });

    Process* proc_ptr = rt.find(process_pid);
    TEST_ASSERT(proc_ptr != nullptr, "proc_ptr must not be null");

    Status status = interp.execute_slice(*proc_ptr, state);
    TEST_ASSERT(status == Status::Done, "status must be Done");

    auto* val = state.x_regs[0].get_if<std::int64_t>();
    return val ? *val : 0;
}

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
    std::cout << "GARUDA COMPILER — SPRINT 1.3 CODEGEN TEST SUITE (15 ASSERTIONS)\n";
    std::cout << "=======================================================\n\n";

    // Test 1: Arithmetic simple 5 + 3 = 8
    assert(compile_and_run("modul m fungsi utama() -> 5 + 3 rampung") == 8);
    std::cout << "  ✓ Test 1: 5 + 3 = 8 PASS\n";

    // Test 2: Presedensi 2 + 3 * 4 = 14
    assert(compile_and_run("modul m fungsi utama() -> 2 + 3 * 4 rampung") == 14);
    std::cout << "  ✓ Test 2: 2 + 3 * 4 = 14 PASS\n";

    // Test 3: Sub 10 - 3 = 7
    assert(compile_and_run("modul m fungsi utama() -> 10 - 3 rampung") == 7);
    std::cout << "  ✓ Test 3: 10 - 3 = 7 PASS\n";

    // Test 4: Mul 6 * 7 = 42
    assert(compile_and_run("modul m fungsi utama() -> 6 * 7 rampung") == 42);
    std::cout << "  ✓ Test 4: 6 * 7 = 42 PASS\n";

    // Test 5: Div 20 / 4 = 5
    assert(compile_and_run("modul m fungsi utama() -> 20 / 4 rampung") == 5);
    std::cout << "  ✓ Test 5: 20 / 4 = 5 PASS\n";

    // Test 6: Nested (2 + 3) * (4 + 1) = 25
    assert(compile_and_run("modul m fungsi utama() -> (2 + 3) * (4 + 1) rampung") == 25);
    std::cout << "  ✓ Test 6: (2+3)*(4+1) = 25 PASS\n";

    // Test 7: Assignment X = 42
    assert(compile_and_run("modul m fungsi utama() -> X = 42 rampung") == 42);
    std::cout << "  ✓ Test 7: X = 42 PASS\n";

    // Test 8: Assignment + use
    assert(compile_and_run("modul m fungsi utama() -> X = 10 Y = 20 X + Y rampung") == 30);
    std::cout << "  ✓ Test 8: X=10, Y=20, X+Y=30 PASS\n";

    // Test 9: Function call tambah(3, 4) = 7
    const char* src9 = R"(
        modul m
        fungsi tambah(A, B) -> A + B rampung
        fungsi utama() -> tambah(3, 4) rampung
    )";
    assert(compile_and_run(src9) == 7);
    std::cout << "  ✓ Test 9: tambah(3, 4) = 7 PASS\n";

    // Test 10: Nested function call tambah(tambah(1, 2), 3) = 6
    const char* src10 = R"(
        modul m
        fungsi tambah(A, B) -> A + B rampung
        fungsi utama() -> tambah(tambah(1, 2), 3) rampung
    )";
    assert(compile_and_run(src10) == 6);
    std::cout << "  ✓ Test 10: tambah(tambah(1,2), 3) = 6 PASS\n";

    // Test 11: Cetak
    assert(compile_and_run("modul m fungsi utama() -> cetak(42) rampung") == 42);
    std::cout << "  ✓ Test 11: cetak(42) PASS\n";

    // Test 12: If yen (X = 5) -> 100
    const char* src12 = R"(
        modul m
        fungsi utama() ->
            X = 5
            yen X > 0 -> 100 lain -> 200 rampung
        rampung
    )";
    assert(compile_and_run(src12) == 100);
    std::cout << "  ✓ Test 12: yen X>0 -> 100 lain -> 200 (X=5) = 100 PASS\n";

    // Test 13: If false (X = -5) -> 200
    const char* src13 = R"(
        modul m
        fungsi utama() ->
            X = -5
            yen X > 0 -> 100 lain -> 200 rampung
        rampung
    )";
    assert(compile_and_run(src13) == 200);
    std::cout << "  ✓ Test 13: yen X>0 -> 100 lain -> 200 (X=-5) = 200 PASS\n";

    // Test 14: Comparison 5 == 5 = 1 (true)
    assert(compile_and_run("modul m fungsi utama() -> 5 == 5 rampung") == 1);
    std::cout << "  ✓ Test 14: 5 == 5 = true PASS\n";

    // Test 15: Comparison false 5 < 3 = 0 (false)
    assert(compile_and_run("modul m fungsi utama() -> 5 < 3 rampung") == 0);
    std::cout << "  ✓ Test 15: 5 < 3 = false PASS\n";

    // Test 16: Recursive Factorial fakt(5) = 120
    const char* src16 = R"(
        modul m
        fungsi fakt(N) ->
            yen N <= 1 ->
                1
            lain ->
                N * fakt(N - 1)
            rampung
        rampung
        fungsi utama() ->
            fakt(5)
        rampung
    )";
    assert(compile_and_run(src16) == 120);
    std::cout << "  ✓ Test 16: Recursive Factorial fakt(5) = 120 PASS\n";

    std::cout << "\n🎉 ALL 15 CODEGEN ASSERTIONS PASSED PERFECTLY!\n";
    return 0;
}
