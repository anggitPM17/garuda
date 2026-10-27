#include "../include/garuda/compiler/parser.hpp"
#include "../include/garuda/compiler/codegen.hpp"
#include "../include/garuda/beam/gvm/actor_runtime.hpp"
#include <chrono>
#include <future>
#include <iostream>
#include <thread>
#include <vector>

using namespace garuda::compiler;
using namespace garuda::beam;

#define GARUDA_CHECK(cond, msg) \
    if (!(cond)) { \
        std::cerr << "  ❌ ASSERTION FAILED: " << msg << " (" << #cond << ")\n"; \
        std::exit(1); \
    }

void run_test_with_timeout(const std::string& test_name, std::function<void()> test_fn, int timeout_seconds = 10) {
    auto future = std::async(std::launch::async, test_fn);
    if (future.wait_for(std::chrono::seconds(timeout_seconds)) == std::future_status::timeout) {
        std::cerr << "  ❌ " << test_name << " FAILED (TIMEOUT > " << timeout_seconds << "s)\n";
        std::exit(1);
    }
}

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA COMPILER — RUNTIME STRESS & PREEMPTION SUITE\n";
    std::cout << "=======================================================\n\n";

    // === Test 1: Global Lock pada Spawn (10.000 Aktor / 10 Thread) ===
    run_test_with_timeout("Test 1: Global Lock pada Spawn (10.000 Aktor / 10 Thread)", []() {
        std::string src = R"(
            modul m
            fungsi worker() ->
                42
            rampung
            fungsi utama() ->
                100
            rampung
        )";
        Lexer lex(src);
        Parser p(lex.tokenize());
        auto module = p.parse_program();

        CodeGenerator cg;
        auto output = cg.generate(module);

        GvmActorRuntime rt(output.all_instructions, output.function_table);
        rt.set_verbose(false);

        auto start = std::chrono::high_resolution_clock::now();

        std::size_t worker_ip = output.function_table["worker"];
        std::vector<std::thread> threads;
        for (int t = 0; t < 10; ++t) {
            threads.emplace_back([&rt, worker_ip]() {
                for (int i = 0; i < 1000; ++i) {
                    rt.spawn_actor(worker_ip);
                }
            });
        }
        for (auto& th : threads) {
            if (th.joinable()) th.join();
        }

        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now() - start).count();

        GARUDA_CHECK(rt.active_actor_count() >= 10000, "Must have 10,000 active actors spawned");
        GARUDA_CHECK(rt.get_total_actors_spawned() >= 10000, "Spawn counter must match");
        GARUDA_CHECK(elapsed < 2000, "Parallel spawn of 10,000 actors must finish < 2000 ms");
        std::cout << "  ✓ Test 1: Global Lock pada Spawn PASS (" << elapsed << " ms)\n";
    }, 10);

    // === Test 2: Global Lock pada Messaging (100.000 Pesan / 1.000 Pasang) ===
    run_test_with_timeout("Test 2: Global Lock pada Messaging (100.000 Pesan)", []() {
        std::string src = R"(
            modul m
            fungsi receiver() ->
                tampa
                    {ping, Dari} ->
                        receiver()
                rampung
            rampung
            fungsi utama() ->
                100
            rampung
        )";
        Lexer lex(src);
        Parser p(lex.tokenize());
        auto module = p.parse_program();

        CodeGenerator cg;
        auto output = cg.generate(module);

        GvmActorRuntime rt(output.all_instructions, output.function_table);
        rt.set_verbose(false);

        auto start = std::chrono::high_resolution_clock::now();

        std::size_t rec_ip = output.function_table["receiver"];
        std::vector<Pid> targets;
        targets.reserve(1000);
        for (int i = 0; i < 1000; ++i) {
            targets.push_back(rt.spawn_actor(rec_ip));
        }

        std::vector<std::thread> senders;
        for (int t = 0; t < 10; ++t) {
            senders.emplace_back([&rt, &targets, t]() {
                Term msg = tuple_of({atom("ping"), Term(static_cast<std::int64_t>(t))});
                for (int i = 0; i < 100; ++i) {
                    for (int p_idx = t * 100; p_idx < (t + 1) * 100; ++p_idx) {
                        rt.send_message(targets[p_idx], msg);
                    }
                }
            });
        }
        for (auto& s : senders) {
            if (s.joinable()) s.join();
        }

        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now() - start).count();

        GARUDA_CHECK(rt.get_total_messages_sent() == 100000, "Message counter must match");
        GARUDA_CHECK(elapsed < 3000, "100,000 concurrent messages must be sent < 3000 ms");
        std::cout << "  ✓ Test 2: Global Lock pada Messaging PASS (" << elapsed << " ms)\n";
    }, 10);

    // === Test 3: Preemption CPU Adil (Aktor Rakus vs Aktor Normal) ===
    run_test_with_timeout("Test 3: Preemption CPU Adil", []() {
        std::string src = R"(
            modul m
            fungsi greedy_loop() ->
                greedy_loop()
            rampung
            fungsi fakt(N) ->
                yen N <= 1 ->
                    1
                lain ->
                    N * fakt(N - 1)
                rampung
            rampung
            fungsi utama() ->
                fakt(10)
            rampung
        )";
        Lexer lex(src);
        Parser p(lex.tokenize());
        auto module = p.parse_program();

        CodeGenerator cg;
        auto output = cg.generate(module);

        GvmActorRuntime rt(output.all_instructions, output.function_table);
        rt.set_verbose(false);

        // Spawn 1 greedy actor running infinite loop
        std::size_t greedy_ip = output.function_table["greedy_loop"];
        rt.spawn_actor(greedy_ip);

        // Run normal actor (utama = fakt(10)) alongside greedy actor
        auto start = std::chrono::high_resolution_clock::now();
        auto res = rt.run_until_done("utama", 500000);

        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now() - start).count();

        GARUDA_CHECK(res.has_value(), "Normal actor must complete execution despite greedy actor");
        GARUDA_CHECK(*res->get_if<std::int64_t>() == 3628800, "fakt(10) must be 3628800");
        GARUDA_CHECK(elapsed < 500, "Normal actor must finish < 500 ms despite greedy actor");
        std::cout << "  ✓ Test 3: Preemption CPU Adil PASS (fakt(10) = 3628800 dalam " << elapsed << " ms)\n";
    }, 10);

    std::cout << "\n🎉 ALL RUNTIME STRESS TESTS PASSED PERFECTLY!\n";
    return 0;
}
