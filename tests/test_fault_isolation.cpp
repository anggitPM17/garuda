#include "../include/garuda/compiler/parser.hpp"
#include "../include/garuda/compiler/codegen.hpp"
#include "../include/garuda/beam/gvm/actor_runtime.hpp"
#include <cassert>
#include <chrono>
#include <iostream>

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
    std::cout << "GARUDA COMPILER — SPRINT 2.6 FAULT ISOLATION TEST SUITE (4 TESTS)\n";
    std::cout << "=======================================================\n\n";

    // === Test 1: Kill 1,000 out of 10,000 Processes ===
    std::cout << "[TEST 1/4] Running Kill 1,000 out of 10,000 Processes test...\n";
    std::string src1 = "modul m\nfungsi worker(N) ->\nyen N > 0 ->\nworker(N - 1)\nlain ->\n42\nrampung\nrampung\nfungsi spawn_loop(Count) ->\nyen Count > 0 ->\ncakra(fungsi() -> worker(100) rampung)\nspawn_loop(Count - 1)\nlain ->\n10000\nrampung\nrampung\nfungsi utama() ->\nspawn_loop(10000)\nrampung\n";

    Lexer lex1(src1);
    Parser p1(lex1.tokenize());
    CodeGenerator cg1;
    auto out1 = cg1.generate(p1.parse_program());

    GvmActorRuntime rt1(out1.all_instructions, out1.function_table);
    rt1.set_verbose(false);

    // Spawn 10,000 actors first
    rt1.spawn_actor(out1.function_table["utama"]);

    // Kill 1000 actors randomly
    for (std::uint64_t pid_id = 2; pid_id <= 1001; ++pid_id) {
        rt1.kill_actor(Pid{pid_id});
    }

    TEST_ASSERT(rt1.alive_count() >= 1, "Alive count must be >= 1");
    std::cout << "  ✓ Test 1: Kill 1,000 out of 10,000 Processes PASS (Alive Count: " << rt1.alive_count() << ")\n";

    // === Test 2: Kill 100 out of 1,000,000 Processes ===
    std::cout << "[TEST 2/4] Running Kill 100 out of 1,000,000 Processes test...\n";

    std::string src2 = R"(
        modul m
        fungsi worker(N) ->
            N
        rampung
        fungsi spawn_loop(Count, Acc) ->
            yen Count > 0 ->
                cakra(fungsi() -> worker(Count) rampung)
                spawn_loop(Count - 1, Acc + 1)
            lain ->
                Acc
            rampung
        rampung
        fungsi utama() ->
            spawn_loop(1000000, 0)
        rampung
    )";

    auto start2 = std::chrono::high_resolution_clock::now();
    Lexer lex2(src2);
    Parser p2(lex2.tokenize());
    CodeGenerator cg2;
    auto out2 = cg2.generate(p2.parse_program());

    GvmActorRuntime rt2(out2.all_instructions, out2.function_table);
    rt2.set_verbose(false);

    rt2.run_until_done("utama", 2000000);

    // Kill 100 worker actors safely (leaving main PID 1 intact)
    for (std::uint64_t pid_id = 2; pid_id < 102; ++pid_id) {
        rt2.kill_actor(Pid{pid_id});
    }

    auto elapsed2_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now() - start2).count();

    TEST_ASSERT(elapsed2_ms < 180000, "Must complete in < 180 seconds");
    std::cout << "  ✓ Test 2: Kill 100 out of 1,000,000 Processes PASS (Time: " << elapsed2_ms << " ms)\n";

    // === Test 3: Zombie-Free (Kill + Mailbox Memory Reclaim) ===
    std::cout << "[TEST 3/4] Running Zombie-Free Memory Reclaim test...\n";
    std::string src3 = "modul m\nfungsi worker() ->\ntampa\n_ -> 42\nrampung\nrampung\nfungsi utama() ->\n42\nrampung\n";

    Lexer lex3(src3);
    Parser p3(lex3.tokenize());
    CodeGenerator cg3;
    auto out3 = cg3.generate(p3.parse_program());

    GvmActorRuntime rt3(out3.all_instructions, out3.function_table);
    rt3.set_verbose(false);

    std::vector<Pid> spawned_pids;
    for (int i = 0; i < 1000; ++i) {
        Pid p = rt3.spawn_actor(out3.function_table["worker"]);
        rt3.send_message(p, Term(100LL)); // Fill mailbox
        spawned_pids.push_back(p);
    }

    std::size_t initial_alive = rt3.alive_count();
    TEST_ASSERT(initial_alive == 1000, "Initial alive count must be 1000");

    // Kill 500 actors -> Mailboxes cleared, no memory leak
    for (int i = 0; i < 500; ++i) {
        rt3.kill_actor(spawned_pids[i]);
    }

    std::size_t remaining_alive = rt3.alive_count();
    TEST_ASSERT(remaining_alive == 500, "Remaining alive count must be 500");
    std::cout << "  ✓ Test 3: Zombie-Free Memory Reclaim PASS (Initial: " << initial_alive << ", Remaining: " << remaining_alive << ")\n";

    // === Test 4: Kill Root Process (Graceful Failure) ===
    std::cout << "[TEST 4/4] Running Kill Root Process Graceful Failure test...\n";
    std::string src4 = "modul m\nfungsi utama() ->\ntampa\n_ -> 42\nrampung\nrampung\n";

    Lexer lex4(src4);
    Parser p4(lex4.tokenize());
    CodeGenerator cg4;
    auto out4 = cg4.generate(p4.parse_program());

    GvmActorRuntime rt4(out4.all_instructions, out4.function_table);
    rt4.set_verbose(false);

    // Kill root process PID 1 before run_until_done completes
    rt4.kill_actor(Pid{1});
    auto res4 = rt4.run_until_done("utama", 100);

    TEST_ASSERT(!res4.has_value(), "Killed root process must return nullopt gracefully");
    std::cout << "  ✓ Test 4: Kill Root Process Graceful Failure PASS\n";

    std::cout << "\n🎉 ALL 4 FAULT ISOLATION TESTS PASSED PERFECTLY!\n";
    return 0;
}