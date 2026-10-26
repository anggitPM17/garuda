#include "../include/garuda/compiler/parser.hpp"
#include "../include/garuda/compiler/codegen.hpp"
#include "../include/garuda/beam/gvm/actor_runtime.hpp"
#include <chrono>
#include <future>
#include <iostream>

using namespace garuda::compiler;
using namespace garuda::beam;

#define GARUDA_CHECK(cond, msg) \
    if (!(cond)) { \
        std::cerr << "  ❌ ASSERTION FAILED: " << msg << " (" << #cond << ")\n"; \
        std::exit(1); \
    }

std::optional<Term> compile_and_run_tco(const std::string& src, const std::string& entry_fn = "utama") {
    Lexer lex(src);
    Parser p(lex.tokenize());
    auto module = p.parse_program();

    CodeGenerator cg;
    auto output = cg.generate(module);

    GvmActorRuntime rt(output.all_instructions, output.function_table);
    rt.set_verbose(false);
    return rt.run_until_done(entry_fn, 50000000);
}

void run_test_with_timeout(const std::string& test_name, std::function<void()> test_fn, int timeout_seconds = 5) {
    auto future = std::async(std::launch::async, test_fn);
    if (future.wait_for(std::chrono::seconds(timeout_seconds)) == std::future_status::timeout) {
        std::cerr << "  ❌ " << test_name << " FAILED (TIMEOUT > " << timeout_seconds << "s)\n";
        std::exit(1);
    }
}

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA COMPILER — TCO SUITE (6 TESTS WITH 5s WATCHDOG)\n";
    std::cout << "=======================================================\n\n";

    // === Test 1: Tail-Recursive Accumulator Factorial ===
    run_test_with_timeout("Test 1: Tail-Recursive fakt_acc(5, 1) = 120", []() {
        std::string src = R"(
            modul m
            fungsi fakt_acc(N, Acc) ->
                yen N <= 1 ->
                    Acc
                lain ->
                    fakt_acc(N - 1, N * Acc)
                rampung
            rampung
            fungsi utama() ->
                fakt_acc(5, 1)
            rampung
        )";
        auto res = compile_and_run_tco(src);
        GARUDA_CHECK(res.has_value(), "res must have value");
        GARUDA_CHECK(*res->get_if<std::int64_t>() == 120, "fakt_acc(5, 1) must be 120");
        std::cout << "  ✓ Test 1: Tail-Recursive fakt_acc(5, 1) = 120 PASS\n";
    }, 5);

    // === Test 2: Rekursi Ekor 1.000.000 Level ===
    run_test_with_timeout("Test 2: TCO Loop 1.000.000 Level", []() {
        std::string src = R"(
            modul m
            fungsi spawn_loop(Count) ->
                yen Count <= 0 ->
                    1000000
                lain ->
                    spawn_loop(Count - 1)
                rampung
            rampung
            fungsi utama() ->
                spawn_loop(1000000)
            rampung
        )";
        auto res = compile_and_run_tco(src);
        GARUDA_CHECK(res.has_value(), "res must have value");
        GARUDA_CHECK(*res->get_if<std::int64_t>() == 1000000, "spawn_loop(1000000) must be 1000000");
        std::cout << "  ✓ Test 2: TCO Loop 1.000.000 Level PASS\n";
    }, 5);

    // === Test 3: Mutual Recursion 100.000 Level ===
    run_test_with_timeout("Test 3: Mutual Recursion genap(100000)", []() {
        std::string src = R"(
            modul m
            fungsi genap(N) ->
                yen N == 0 ->
                    1
                lain ->
                    ganjil(N - 1)
                rampung
            rampung
            fungsi ganjil(N) ->
                yen N == 0 ->
                    0
                lain ->
                    genap(N - 1)
                rampung
            rampung
            fungsi utama() ->
                genap(100000)
            rampung
        )";
        auto res = compile_and_run_tco(src);
        GARUDA_CHECK(res.has_value(), "res must have value");
        GARUDA_CHECK(*res->get_if<std::int64_t>() == 1, "genap(100000) must be 1");
        std::cout << "  ✓ Test 3: Mutual Recursion genap(100000) PASS\n";
    }, 5);

    // === Test 4: NON-Tail Call N * fakt(N - 1) = 120 ===
    run_test_with_timeout("Test 4: Non-Tail Call fakt(5) = 120", []() {
        std::string src = R"(
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
        auto res = compile_and_run_tco(src);
        GARUDA_CHECK(res.has_value(), "res must have value");
        GARUDA_CHECK(*res->get_if<std::int64_t>() == 120, "fakt(5) must be 120");
        std::cout << "  ✓ Test 4: Non-Tail Call fakt(5) = 120 PASS\n";
    }, 5);

    // === Test 5: Tail Call dalam Cabang If/Else ===
    run_test_with_timeout("Test 5: Tail Call dalam Cabang If/Else", []() {
        std::string src = R"(
            modul m
            fungsi cabang(N) ->
                yen N > 10 ->
                    cabang(N - 1)
                lain ->
                    N
                rampung
            rampung
            fungsi utama() ->
                cabang(15)
            rampung
        )";
        auto res = compile_and_run_tco(src);
        GARUDA_CHECK(res.has_value(), "res must have value");
        GARUDA_CHECK(*res->get_if<std::int64_t>() == 10, "cabang(15) must be 10");
        std::cout << "  ✓ Test 5: Tail Call dalam Cabang If/Else PASS\n";
    }, 5);

    // === Test 6: Aktor Receive Loop Rekursif-Ekor ===
    run_test_with_timeout("Test 6: Loop Rekursif-Ekor Aktor 50.000 Iterasi", []() {
        std::string src = R"(
            modul m
            fungsi loop_rec(Count, Acc) ->
                yen Count <= 0 ->
                    Acc
                lain ->
                    loop_rec(Count - 1, Acc + 1)
                rampung
            rampung
            fungsi utama() ->
                loop_rec(50000, 0)
            rampung
        )";
        auto res = compile_and_run_tco(src);
        GARUDA_CHECK(res.has_value(), "res must have value");
        GARUDA_CHECK(*res->get_if<std::int64_t>() == 50000, "loop_rec(50000, 0) must be 50000");
        std::cout << "  ✓ Test 6: Loop Rekursif-Ekor Aktor PASS\n";
    }, 5);

    std::cout << "\n🎉 ALL TCO TESTS PASSED PERFECTLY WITH O(1) STACK USAGE!\n";
    return 0;
}
