#include "../include/garuda/compiler/lexer.hpp"
#include "../include/garuda/compiler/parser.hpp"
#include <chrono>
#include <future>
#include <iostream>
#include <random>
#include <string>

using namespace garuda::compiler;

#define GARUDA_CHECK(cond, msg) \
    if (!(cond)) { \
        std::cerr << "  ❌ ASSERTION FAILED: " << msg << " (" << #cond << ")\n"; \
        std::exit(1); \
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
    std::cout << "GARUDA COMPILER — PARSER & LEXER FUZZING SUITE\n";
    std::cout << "=======================================================\n\n";

    // === Test 1: Mini-Fuzzer (1.000 Random Byte Sequences) ===
    run_test_with_timeout("Test 1: Random String Mini-Fuzzer (1.000 Iterations)", []() {
        std::mt19937 rng(42); // Fixed seed for reproducible fuzzing
        std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 _+-*/<>=(){}[]->;:\n\t\"'\\#@$%^&*~`|!";

        std::uniform_int_distribution<std::size_t> len_dist(1, 150);
        std::uniform_int_distribution<std::size_t> char_dist(0, charset.length() - 1);

        std::size_t handled_errors = 0;
        std::size_t valid_parses = 0;

        for (int i = 0; i < 1000; ++i) {
            std::size_t len = len_dist(rng);
            std::string random_src;
            random_src.reserve(len);
            for (std::size_t j = 0; j < len; ++j) {
                random_src += charset[char_dist(rng)];
            }

            try {
                Lexer lexer(random_src);
                auto tokens = lexer.tokenize();
                Parser parser(tokens);
                parser.parse_program();
                valid_parses++;
            } catch (const ParseError&) {
                handled_errors++;
            } catch (const std::exception& e) {
                GARUDA_CHECK(false, std::string("Unhandled exception in fuzzing iteration: ") + e.what());
            } catch (...) {
                GARUDA_CHECK(false, "Unknown crash/exception in fuzzing iteration");
            }
        }

        GARUDA_CHECK(handled_errors + valid_parses == 1000, "All 1,000 fuzzing iterations must terminate safely");
        std::cout << "  ✓ Test 1: Random String Mini-Fuzzer PASS (Handled Errors: "
                  << handled_errors << ", Valid Parses: " << valid_parses << ")\n";
    }, 5);

    // === Test 2: Truncation Fuzzing (Valid Code Cut at Random Indices) ===
    run_test_with_timeout("Test 2: Truncation Fuzzing on Valid Garuda Program", []() {
        std::string valid_src = R"(
            modul Faktorial
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

        std::size_t total_len = valid_src.length();
        std::size_t handled_truncs = 0;
        std::size_t valid_truncs = 0;

        for (std::size_t cut = 0; cut <= total_len; ++cut) {
            std::string truncated_src = valid_src.substr(0, cut);
            try {
                Lexer lexer(truncated_src);
                auto tokens = lexer.tokenize();
                Parser parser(tokens);
                parser.parse_program();
                valid_truncs++;
            } catch (const ParseError&) {
                handled_truncs++;
            } catch (const std::exception& e) {
                GARUDA_CHECK(false, std::string("Unhandled exception at truncation cut ") + std::to_string(cut) + ": " + e.what());
            } catch (...) {
                GARUDA_CHECK(false, "Unknown crash at truncation cut " + std::to_string(cut));
            }
        }

        GARUDA_CHECK(handled_truncs + valid_truncs == total_len + 1, "All truncation cuts must terminate safely without crashing");
        std::cout << "  ✓ Test 2: Truncation Fuzzing PASS (Cuts: " << total_len + 1
                  << ", Handled Errors: " << handled_truncs << ", Valid Cuts: " << valid_truncs << ")\n";
    }, 5);

    std::cout << "\n🎉 ALL PARSER & LEXER FUZZING TESTS PASSED PERFECTLY!\n";
    return 0;
}
