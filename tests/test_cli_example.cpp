#include "../include/garuda/compiler/lexer.hpp"
#include "../include/garuda/compiler/parser.hpp"
#include "../include/garuda/compiler/codegen.hpp"
#include "../include/garuda/beam/io/filesystem.hpp"
#include "../include/garuda/beam/io/process.hpp"
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

void run_test_with_timeout(const std::string& test_name, std::function<void()> test_fn, int timeout_seconds = 5) {
    auto future = std::async(std::launch::async, test_fn);
    if (future.wait_for(std::chrono::seconds(timeout_seconds)) == std::future_status::timeout) {
        std::cerr << "  ❌ " << test_name << " FAILED (TIMEOUT > " << timeout_seconds << "s)\n";
        std::exit(1); \
    }
}

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA CLI EXAMPLE PROJECT ('salam') SUITE\n";
    std::cout << "=======================================================\n\n";

    // === Test 1: Compile CLI Tool 'main.grd' ===
    run_test_with_timeout("Test 1: Compile CLI Tool main.grd", []() {
        std::string grd_path = "examples/cli_tool/main.grd";
        GARUDA_CHECK(Fs::exists(grd_path), "examples/cli_tool/main.grd must exist");

        std::string src = Fs::read_file(grd_path);
        GARUDA_CHECK(!src.empty(), "Source file content must not be empty");

        Lexer lexer(src);
        auto tokens = lexer.tokenize();
        GARUDA_CHECK(!tokens.empty(), "Lexer must produce tokens");

        Parser parser(tokens);
        auto ast = parser.parse_program();
        GARUDA_CHECK(ast != nullptr, "Parser AST must not be nullptr");

        CodeGenerator cg;
        CodeGeneratorOutput output = cg.generate(ast);
        GARUDA_CHECK(!output.all_instructions.empty(), "Compiled bytecode must contain instructions");
        GARUDA_CHECK(output.function_table.find("utama") != output.function_table.end(), "Function table must contain 'utama'");

        std::cout << "  ✓ Test 1: Compile CLI Tool main.grd PASS (Instructions: "
                  << output.all_instructions.size() << ")\n";
    }, 5);

    // === Test 2: Verify Project Structure ===
    run_test_with_timeout("Test 2: Verify Project Structure", []() {
        GARUDA_CHECK(Fs::exists("examples/cli_tool/README.md"), "README.md must exist");
        GARUDA_CHECK(Fs::exists("examples/cli_tool/grdmix.toml"), "grdmix.toml must exist");

        std::string readme = Fs::read_file("examples/cli_tool/README.md");
        GARUDA_CHECK(readme.find("salam") != std::string::npos, "README.md must mention 'salam'");

        std::string toml = Fs::read_file("examples/cli_tool/grdmix.toml");
        GARUDA_CHECK(toml.find("salam_cli") != std::string::npos, "grdmix.toml must define 'salam_cli'");

        std::cout << "  ✓ Test 2: Verify Project Structure PASS\n";
    }, 5);

    // === Test 3: Process Execution Simulation ===
    run_test_with_timeout("Test 3: Process Execution Simulation", []() {
        auto [code, output] = SystemProcess::run("echo Halo, Budi!");
        GARUDA_CHECK(code == 0, "Process exit code must be 0");
        GARUDA_CHECK(output.find("Halo, Budi!") != std::string::npos, "Output must contain 'Halo, Budi!'");

        std::cout << "  ✓ Test 3: Process Execution Simulation PASS\n";
    }, 5);

    std::cout << "\n🎉 ALL CLI EXAMPLE PROJECT TESTS PASSED PERFECTLY!\n";
    return 0;
}
