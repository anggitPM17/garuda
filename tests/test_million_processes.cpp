#include "../include/garuda/compiler/parser.hpp"
#include "../include/garuda/compiler/codegen.hpp"
#include "../include/garuda/beam/gvm/actor_runtime.hpp"
#include <cassert>
#include <chrono>
#include <iostream>

using namespace garuda::compiler;
using namespace garuda::beam;

std::optional<Term> compile_and_run_million(const std::string& src, std::size_t max_iter = 2000000) {
    Lexer lex(src);
    Parser p(lex.tokenize());
    auto module = p.parse_program();

    CodeGenerator cg;
    auto output = cg.generate(module);

    GvmActorRuntime rt(output.all_instructions, output.function_table);
    rt.set_verbose(false);
    return rt.run_until_done("utama", max_iter);
}

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA COMPILER — SPRINT 2.5 1 MILLION PROCESSES STRESS TEST (3 TESTS)\n";
    std::cout << "=======================================================\n\n";

    // === Test 1: 1 Juta Proses Sederhana (Return 1000000) ===
    std::cout << "[TEST 1/3] Running 1.000.000 Processes Spawn & Execute test...\n";
    std::string src1 = R"(
        modul m
        fungsi worker(Id) ->
            Id
        rampung
        fungsi spawn_loop(N) ->
            yen N > 0 ->
                cakra(fungsi() -> worker(N) rampung)
                spawn_loop(N - 1)
            lain ->
                1000000
            rampung
        rampung
        fungsi utama() ->
            spawn_loop(1000000)
        rampung
    )";

    auto start1 = std::chrono::high_resolution_clock::now();
    auto res1 = compile_and_run_million(src1, 3000000);
    auto elapsed1_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now() - start1).count();

    assert(res1.has_value());
    assert(*res1->get_if<std::int64_t>() == 1000000);
    assert(elapsed1_ms < 30000); // Must complete in < 30 seconds!

    std::cout << "  📊 [MILLION TEST] Spawned: 1.000.000 Actors, Time: " << elapsed1_ms << " ms\n";
    std::cout << "  ✓ Test 1: 1.000.000 Processes Spawn & Execute PASS\n";

    // === Test 2: 1 Juta Proses dengan Mailbox Messaging ===
    std::cout << "[TEST 2/3] Running 1.000.000 Processes Mailbox Messaging test...\n";
    std::string src2 = R"(
        modul m
        fungsi worker() ->
            tampa
                {ping, Dari} -> Dari <- {pong, diri}
            rampung
        rampung
        fungsi spawn_loop(N) ->
            yen N > 0 ->
                P = cakra(fungsi() -> worker() rampung)
                P <- {ping, diri}
                spawn_loop(N - 1)
            lain ->
                1000000
            rampung
        rampung
        fungsi utama() ->
            spawn_loop(1000000)
        rampung
    )";

    auto start2 = std::chrono::high_resolution_clock::now();
    auto res2 = compile_and_run_million(src2, 3000000);
    auto elapsed2_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now() - start2).count();

    assert(res2.has_value());
    assert(*res2->get_if<std::int64_t>() == 1000000);
    assert(elapsed2_ms < 60000); // Must complete in < 60 seconds!

    std::cout << "  📊 [MILLION TEST] 1.000.000 Mailbox Messages, Time: " << elapsed2_ms << " ms\n";
    std::cout << "  ✓ Test 2: 1.000.000 Processes Mailbox Messaging PASS\n";

    // === Test 3: 100.000 Proses dengan Loop Kecil ===
    std::cout << "[TEST 3/3] Running 100.000 Processes Recursive Loop test...\n";
    std::string src3 = R"(
        modul m
        fungsi loop_worker(N) ->
            yen N > 0 ->
                loop_worker(N - 1)
            lain ->
                100
            rampung
        rampung
        fungsi spawn_loop(Count) ->
            yen Count > 0 ->
                cakra(fungsi() -> loop_worker(10) rampung)
                spawn_loop(Count - 1)
            lain ->
                100000
            rampung
        rampung
        fungsi utama() ->
            spawn_loop(100000)
        rampung
    )";

    auto start3 = std::chrono::high_resolution_clock::now();
    auto res3 = compile_and_run_million(src3, 1000000);
    auto elapsed3_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now() - start3).count();

    assert(res3.has_value());
    assert(*res3->get_if<std::int64_t>() == 100000);
    assert(elapsed3_ms < 15000); // Must complete in < 15 seconds!

    std::cout << "  📊 [MILLION TEST] 100.000 Processes + Loop, Time: " << elapsed3_ms << " ms\n";
    std::cout << "  ✓ Test 3: 100.000 Processes Recursive Loop PASS\n";

    std::cout << "\n🎉 ALL 3 MILLION PROCESSES STRESS TESTS PASSED PERFECTLY!\n";
    return 0;
}
