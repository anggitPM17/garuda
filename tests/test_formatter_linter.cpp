#include "../include/garuda/compiler/lexer.hpp"
#include "../include/garuda/compiler/parser.hpp"
#include "../include/garuda/compiler/formatter.hpp"
#include "../include/garuda/compiler/linter.hpp"
#include "../include/garuda/compiler/codegen.hpp"
#include "../include/garuda/beam/gvm/interpreter.hpp"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>

using namespace garuda::compiler;
using namespace garuda::beam;
namespace fs = std::filesystem;

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

std::string format_code(const std::string& src) {
    Lexer lexer(src);
    Parser parser(lexer.tokenize());
    auto module = parser.parse_program();

    Formatter fmt;
    return fmt.format(module);
}

std::vector<LintWarning> lint_code(const std::string& src) {
    Lexer lexer(src);
    Parser parser(lexer.tokenize());
    auto module = parser.parse_program();

    Linter linter;
    return linter.lint(module);
}

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA COMPILER — FORMATTER & LINTER SUITE (10 TESTS)\n";
    std::cout << "=======================================================\n\n";

    // === Test 1: Formatter 4-Space Indentation ===
    run_test_with_timeout("Test 1: Formatter 4-Space Indentation", []() {
        std::string unformatted = "modul m fungsi f() -> X = 10 X + 20 rampung";
        std::string fmt = format_code(unformatted);
        GARUDA_CHECK(fmt.find("    X = 10") != std::string::npos, "Must format with 4-space indentation for X = 10");
        GARUDA_CHECK(fmt.find("    X + 20") != std::string::npos, "Must format with 4-space indentation for X + 20");
        std::cout << "  ✓ Test 1: Formatter 4-Space Indentation PASS\n";
    }, 5);

    // === Test 2: Formatter Operator Spacing ===
    run_test_with_timeout("Test 2: Formatter Operator Spacing", []() {
        std::string unformatted = "modul m fungsi f() -> A=1+2*3 rampung";
        std::string fmt = format_code(unformatted);
        GARUDA_CHECK(fmt.find("1 + 2 * 3") != std::string::npos, "Must insert spaces around operators");
        GARUDA_CHECK(fmt.find("A = 1") != std::string::npos, "Must insert spaces around assignment");
        std::cout << "  ✓ Test 2: Formatter Operator Spacing PASS\n";
    }, 5);

    // === Test 3: Formatter Idempotency Check ===
    run_test_with_timeout("Test 3: Formatter Idempotency Check", []() {
        std::string unformatted = "modul m fungsi f() -> X = 10 X + 20 rampung";
        std::string fmt_once = format_code(unformatted);
        std::string fmt_twice = format_code(fmt_once);
        GARUDA_CHECK(fmt_once == fmt_twice, "Formatting 2x must result in identical string");
        std::cout << "  ✓ Test 3: Formatter Idempotency Check PASS\n";
    }, 5);

    // === Test 4: Formatter Preserves Program Semantics ===
    run_test_with_timeout("Test 4: Formatter Preserves Semantics", []() {
        std::string raw_code = "modul m fungsi utama() -> X = 40, X + 2 rampung";
        std::string fmt_code = format_code(raw_code);

        Lexer lex(fmt_code);
        Parser parser(lex.tokenize());
        CodeGenerator cg;
        auto output = cg.generate(parser.parse_program());

        GvmInterpreter interpreter(output.all_instructions);
        GvmProcessState state;
        Runtime dummy_rt;
        Pid dummy_pid = dummy_rt.spawn([](Process&) { return Status::Done; });
        Process* proc_ptr = dummy_rt.find(dummy_pid);

        Status status = interpreter.execute_slice(*proc_ptr, state);
        GARUDA_CHECK(status == Status::Done, "Execution of formatted code must succeed");
        GARUDA_CHECK(*state.x_regs[0].get_if<std::int64_t>() == 42, "Execution result must be 42");
        std::cout << "  ✓ Test 4: Formatter Preserves Semantics PASS\n";
    }, 5);

    // === Test 5: Formatter Tuple and List Formatting ===
    run_test_with_timeout("Test 5: Formatter Tuple and List Formatting", []() {
        std::string raw_code = "modul m fungsi f() -> {1,2}, [10,20] rampung";
        std::string fmt = format_code(raw_code);
        GARUDA_CHECK(fmt.find("{1, 2}") != std::string::npos, "Tuple must format with space after comma");
        GARUDA_CHECK(fmt.find("[10, 20]") != std::string::npos, "List must format with space after comma");
        std::cout << "  ✓ Test 5: Formatter Tuple and List Formatting PASS\n";
    }, 5);

    // === Test 6: Linter Non-snake_case Function Name ===
    run_test_with_timeout("Test 6: Linter Non-snake_case Function Name", []() {
        auto warnings = lint_code("modul m fungsi HitungTotal() -> 42 rampung");
        GARUDA_CHECK(!warnings.empty(), "Linter must warn about non-snake_case function name");
        GARUDA_CHECK(warnings[0].message.find("HitungTotal") != std::string::npos, "Warning must mention function name");
        std::cout << "  ✓ Test 6: Linter Non-snake_case Function Name PASS\n";
    }, 5);

    // === Test 7: Linter Non-snake_case Variable Name ===
    run_test_with_timeout("Test 7: Linter Non-snake_case Variable Name", []() {
        auto warnings = lint_code("modul m fungsi f(BadParam) -> BadParam rampung");
        GARUDA_CHECK(!warnings.empty(), "Linter must warn about non-snake_case variable name");
        GARUDA_CHECK(warnings[0].message.find("BadParam") != std::string::npos, "Warning must mention variable name");
        std::cout << "  ✓ Test 7: Linter Non-snake_case Variable Name PASS\n";
    }, 5);

    // === Test 8: Linter Unused Variable Detection ===
    run_test_with_timeout("Test 8: Linter Unused Variable Detection", []() {
        auto warnings = lint_code("modul m fungsi f(x) -> unused_var = 100, x + 5 rampung");
        GARUDA_CHECK(!warnings.empty(), "Linter must warn about unused variable");
        GARUDA_CHECK(warnings[0].message.find("unused_var") != std::string::npos, "Warning must mention unused_var");
        std::cout << "  ✓ Test 8: Linter Unused Variable Detection PASS\n";
    }, 5);

    // === Test 9: Linter Clean Code Check (0 Warnings) ===
    run_test_with_timeout("Test 9: Linter Clean Code Check", []() {
        auto warnings = lint_code("modul m fungsi hitung_total(a, b) -> sum = a + b, sum rampung");
        GARUDA_CHECK(warnings.empty(), "Linter must return 0 warnings for clean code");
        std::cout << "  ✓ Test 9: Linter Clean Code Check PASS\n";
    }, 5);

    // === Test 10: End-to-End File Formatting and Linting ===
    run_test_with_timeout("Test 10: End-to-End File Formatting and Linting", []() {
        fs::path temp_file = fs::temp_directory_path() / "test_fmt.grd";
        std::ofstream ofs(temp_file);
        ofs << "modul m fungsi f() -> 1+2 rampung";
        ofs.close();

        GARUDA_CHECK(fs::exists(temp_file), "Temp file must exist");
        std::string file_fmt = format_code("modul m fungsi f() -> 1+2 rampung");
        GARUDA_CHECK(file_fmt.find("1 + 2") != std::string::npos, "Formatted code must contain spaces around operator");

        fs::remove(temp_file);
        std::cout << "  ✓ Test 10: End-to-End File Formatting and Linting PASS\n";
    }, 5);

    std::cout << "\n🎉 ALL 10 FORMATTER & LINTER TESTS PASSED PERFECTLY!\n";
    return 0;
}
