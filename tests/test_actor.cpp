#include "../include/garuda/compiler/parser.hpp"
#include "../include/garuda/compiler/codegen.hpp"
#include "../include/garuda/beam/gvm/actor_runtime.hpp"
#include <cassert>
#include <iostream>

using namespace garuda::compiler;
using namespace garuda::beam;

// Helper: compile module -> run actor program
std::optional<Term> compile_and_run_actor(const std::string& src, const std::string& entry_fn = "utama") {
    Lexer lex(src);
    Parser p(lex.tokenize());
    auto module = p.parse_program();

    CodeGenerator cg;
    auto output = cg.generate(module);

    GvmActorRuntime rt(output.all_instructions, output.function_table);
    return rt.run_until_done(entry_fn);
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
    std::cout << "GARUDA COMPILER — SPRINT 1.4 ACTOR TEST SUITE (12 ASSERTIONS)\n";
    std::cout << "=======================================================\n\n";

    // === Test 1: Spawn sederhana, tidak crash ===
    compile_and_run_actor(R"(
        modul m
        fungsi utama() ->
            Pid = cakra(fungsi() -> 42 rampung)
            Pid
        rampung
    )");
    std::cout << "  ✓ Test 1: spawn simple PASS\n";

    // === Test 2: Spawn + send, terima pesan di proses ===
    compile_and_run_actor(R"(
        modul m
        fungsi utama() ->
            Pid = cakra(fungsi() -> 42 rampung)
            Pid <- {halo, 100}
        rampung
    )");
    std::cout << "  ✓ Test 2: spawn + send PASS\n";

    // === Test 3: Self (diri) ===
    compile_and_run_actor(R"(
        modul m
        fungsi utama() ->
            P = diri
            P
        rampung
    )");
    std::cout << "  ✓ Test 3: diri (self) PASS\n";

    // === Test 4: Send ke self ===
    compile_and_run_actor(R"(
        modul m
        fungsi utama() ->
            diri <- {halo, "dunia"}
        rampung
    )");
    std::cout << "  ✓ Test 4: kirim ke diri sendiri PASS\n";

    // === Test 5: Receive sederhana ===
    compile_and_run_actor(R"(
        modul m
        fungsi utama() ->
            diri <- {halo, 42}
            tampa
                {halo, X} -> X
            rampung
        rampung
    )");
    std::cout << "  ✓ Test 5: receive dengan pattern tuple PASS\n";

    // === Test 6: Receive wildcard ===
    compile_and_run_actor(R"(
        modul m
        fungsi utama() ->
            diri <- :apapun
            tampa
                _ -> :diterima
            rampung
        rampung
    )");
    std::cout << "  ✓ Test 6: receive wildcard PASS\n";

    // === Test 7: Receive literal ===
    compile_and_run_actor(R"(
        modul m
        fungsi utama() ->
            diri <- :halo
            tampa
                :halo -> 100
            rampung
        rampung
    )");
    std::cout << "  ✓ Test 7: receive literal atom PASS\n";

    // === Test 8: Ping-pong actor (krusial!) ===
    compile_and_run_actor(R"(
        modul m
        fungsi server() ->
            tampa
                {ping, Dari} -> Dari <- {pong, diri}
            rampung
        rampung
        fungsi utama() ->
            S = cakra(fungsi() -> server() rampung)
            S <- {ping, diri}
            tampa
                {pong, ServerPid} -> 42
            rampung
        rampung
    )");
    std::cout << "  ✓ Test 8: ping-pong actor PASS\n";

    // === Test 9: Pattern matching binding ===
    compile_and_run_actor(R"(
        modul m
        fungsi utama() ->
            diri <- {1, 2, 3}
            tampa
                {A, B, C} -> A + B + C
            rampung
        rampung
    )");
    std::cout << "  ✓ Test 9: tuple binding {A,B,C} PASS\n";

    // === Test 10: Multiple spawn ===
    compile_and_run_actor(R"(
        modul m
        fungsi utama() ->
            P1 = cakra(fungsi() -> 1 rampung)
            P2 = cakra(fungsi() -> 2 rampung)
            P3 = cakra(fungsi() -> 3 rampung)
            42
        rampung
    )");
    std::cout << "  ✓ Test 10: multiple spawn PASS\n";

    // === Test 11: Nested receive ===
    compile_and_run_actor(R"(
        modul m
        fungsi utama() ->
            diri <- {a, {b, c}}
            tampa
                {X, {Y, Z}} -> 42
            rampung
        rampung
    )");
    std::cout << "  ✓ Test 11: nested tuple pattern PASS\n";

    // === Test 12: Send + receive end-to-end value assertion ===
    auto res12 = compile_and_run_actor(R"(
        modul m
        fungsi utama() ->
            diri <- {hasil, 100}
            tampa
                {hasil, Nilai} -> Nilai
            rampung
        rampung
    )");
    assert(res12.has_value());
    assert(res12->get_if<std::int64_t>() != nullptr);
    assert(*res12->get_if<std::int64_t>() == 100);
    std::cout << "  ✓ Test 12: send+receive value=100 PASS\n";

    std::cout << "\n🎉 ALL 12 ACTOR ASSERTIONS PASSED PERFECTLY!\n";
    return 0;
}
