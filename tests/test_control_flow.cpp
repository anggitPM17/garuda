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

std::optional<Term> compile_and_run_cf(const std::string& src, const std::string& entry_fn = "utama") {
    Lexer lex(src);
    Parser p(lex.tokenize());
    auto module = p.parse_program();

    CodeGenerator cg;
    auto output = cg.generate(module);

    GvmActorRuntime rt(output.all_instructions, output.function_table);
    rt.set_verbose(false);
    return rt.run_until_done(entry_fn, 5000000);
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
    std::cout << "GARUDA COMPILER — CONTROL FLOW TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // === Test 1: yen tanpa lain (Kondisi False -> lanjut ke statement berikutnya 999) ===
    run_test_with_timeout("Test 1: yen tanpa lain (False -> Lanjut ke statement berikutnya)", []() {
        std::string src = R"(
            modul m
            fungsi tes_tanpa_lain(X) ->
                yen X > 10 ->
                    100
                rampung
                999
            rampung
            fungsi utama() ->
                tes_tanpa_lain(5)
            rampung
        )";
        auto res = compile_and_run_cf(src);
        GARUDA_CHECK(res.has_value(), "res must have value");
        GARUDA_CHECK(*res->get_if<std::int64_t>() == 999, "tes_tanpa_lain(5) must be 999");
        std::cout << "  ✓ Test 1: yen tanpa lain (False -> Lanjut ke 999) PASS\n";
    }, 5);

    // === Test 2: yen / lain pemilihan cabang dasar (True -> 100, False -> 200) ===
    run_test_with_timeout("Test 2: yen / lain (True -> 100, False -> 200)", []() {
        std::string src1 = R"(
            modul m
            fungsi tes_if(X) ->
                yen X > 10 ->
                    100
                lain ->
                    200
                rampung
            rampung
            fungsi utama() ->
                tes_if(15)
            rampung
        )";
        auto res1 = compile_and_run_cf(src1);
        GARUDA_CHECK(res1.has_value(), "res1 must have value");
        GARUDA_CHECK(*res1->get_if<std::int64_t>() == 100, "tes_if(15) must be 100");

        std::string src2 = R"(
            modul m
            fungsi tes_if(X) ->
                yen X > 10 ->
                    100
                lain ->
                    200
                rampung
            rampung
            fungsi utama() ->
                tes_if(5)
            rampung
        )";
        auto res2 = compile_and_run_cf(src2);
        GARUDA_CHECK(res2.has_value(), "res2 must have value");
        GARUDA_CHECK(*res2->get_if<std::int64_t>() == 200, "tes_if(5) must be 200");

        std::cout << "  ✓ Test 2: yen / lain (True -> 100, False -> 200) PASS\n";
    }, 5);

    // === Test 3: yen bersarang 3 level (Pilih cabang tengah B=0 -> 999) ===
    run_test_with_timeout("Test 3: yen bersarang 3 level (B=0 -> 999)", []() {
        std::string src = R"(
            modul m
            fungsi tes_bersarang(A, B, C) ->
                yen A > 0 ->
                    yen B > 0 ->
                        yen C > 0 ->
                            777
                        lain ->
                            888
                        rampung
                    lain ->
                        999
                    rampung
                lain ->
                    111
                rampung
            rampung
            fungsi utama() ->
                tes_bersarang(1, 0, 1)
            rampung
        )";
        auto res = compile_and_run_cf(src);
        GARUDA_CHECK(res.has_value(), "res must have value");
        GARUDA_CHECK(*res->get_if<std::int64_t>() == 999, "tes_bersarang(1,0,1) must be 999");
        std::cout << "  ✓ Test 3: yen bersarang 3 level (B=0 -> 999) PASS\n";
    }, 5);

    // === Test 4: yen dengan else-if chain (N=3 -> 300) ===
    run_test_with_timeout("Test 4: yen else-if chain (N=3 -> 300)", []() {
        std::string src = R"(
            modul m
            fungsi tes_chain(N) ->
                yen N == 1 ->
                    100
                lain ->
                    yen N == 2 ->
                        200
                    lain ->
                        300
                    rampung
                rampung
            rampung
            fungsi utama() ->
                tes_chain(3)
            rampung
        )";
        auto res = compile_and_run_cf(src);
        GARUDA_CHECK(res.has_value(), "res must have value");
        GARUDA_CHECK(*res->get_if<std::int64_t>() == 300, "tes_chain(3) must be 300");
        std::cout << "  ✓ Test 4: yen else-if chain (N=3 -> 300) PASS\n";
    }, 5);

    // === Test 5: SKIPPED — cond multi-cabang (belum ada di parser) ===
    std::cout << "  ⏭ Test 5: cond multi-cabang SKIPPED (cond belum didukung di parser)\n";

    // === Test 6: SKIPPED — case pattern (belum ada di codegen) ===
    std::cout << "  ⏭ Test 6: case pattern SKIPPED (case/tinjau belum ditangani di codegen)\n";

    // === Test 7: Hitung mundur 10->0 ===
    run_test_with_timeout("Test 7: Hitung mundur 10->0", []() {
        std::string src = R"(
            modul m
            fungsi mundur(N) ->
                yen N <= 0 ->
                    0
                lain ->
                    mundur(N - 1)
                rampung
            rampung
            fungsi utama() ->
                mundur(10)
            rampung
        )";
        auto res = compile_and_run_cf(src);
        GARUDA_CHECK(res.has_value(), "res must have value");
        GARUDA_CHECK(*res->get_if<std::int64_t>() == 0, "mundur(10) must be 0");
        std::cout << "  ✓ Test 7: Hitung mundur 10->0 PASS (Hasil: 0)\n";
    }, 5);

    std::cout << "\n🎉 CONTROL FLOW TESTS COMPLETED!\n";
    return 0;
}
