#include "../include/garuda/compiler/lexer.hpp"
#include "../include/garuda/compiler/parser.hpp"
#include "../include/garuda/compiler/codegen.hpp"
#include <chrono>
#include <future>
#include <iostream>

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
    std::cout << "GARUDA COMPILER — IN-PROCESS COMPILER API SUITE\n";
    std::cout << "=======================================================\n\n";

    // === Test 1: In-Process Lexer -> Parser -> Codegen Pipeline ===
    run_test_with_timeout("Test 1: In-Process Compiler Pipeline Execution", []() {
        std::string src = R"(
            modul EmbeddableApp
            fungsi hitung(X) ->
                X * 2
            rampung
            fungsi utama() ->
                hitung(21)
            rampung
        )";

        Lexer lexer(src);
        auto tokens = lexer.tokenize();
        GARUDA_CHECK(!tokens.empty(), "Tokens vector must not be empty");

        Parser parser(tokens);
        auto ast = parser.parse_program();
        GARUDA_CHECK(ast != nullptr, "Parsed AST root must not be nullptr");
        GARUDA_CHECK(ast->name() == "EmbeddableApp", "Module name in AST must match 'EmbeddableApp'");

        CodeGenerator cg;
        CodeGeneratorOutput output = cg.generate(ast);

        GARUDA_CHECK(output.module_name == "EmbeddableApp", "CodeGeneratorOutput module_name must match");
        GARUDA_CHECK(!output.all_instructions.empty(), "Instruction vector must contain compiled opcodes");
        GARUDA_CHECK(output.function_table.find("utama") != output.function_table.end(), "Function table must contain 'utama'");
        GARUDA_CHECK(output.function_table.find("hitung") != output.function_table.end(), "Function table must contain 'hitung'");

        std::cout << "  ✓ Test 1: In-Process Compiler Pipeline Execution PASS (Opcodes: "
                  << output.all_instructions.size() << ")\n";
    }, 5);

    // === Test 2: In-Memory Syntax Error Handling ===
    run_test_with_timeout("Test 2: In-Memory Syntax Error Handling", []() {
        std::string invalid_src = "modul Invalid\nfungsi utama(\n    42\nrampung";
        Lexer lexer(invalid_src);
        Parser parser(lexer.tokenize());

        bool caught = false;
        try {
            parser.parse_program();
        } catch (const ParseError& err) {
            caught = true;
            GARUDA_CHECK(err.line >= 2, "ParseError line must be >= 2");
        }
        GARUDA_CHECK(caught, "ParseError must be thrown for incomplete function declaration");

        std::cout << "  ✓ Test 2: In-Memory Syntax Error Handling PASS\n";
    }, 5);

    std::cout << "\n🎉 ALL IN-PROCESS COMPILER API TESTS PASSED PERFECTLY!\n";
    return 0;
}
