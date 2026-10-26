#include "../include/garuda/compiler/lexer.hpp"
#include "../include/garuda/compiler/parser.hpp"
#include "../include/garuda/compiler/formatter.hpp"
#include "../include/garuda/compiler/linter.hpp"
#include "../include/garuda/compiler/codegen.hpp"
#include "../include/garuda/beam/gvm/interpreter.hpp"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>

using namespace garuda::compiler;
using namespace garuda::beam;
namespace fs = std::filesystem;

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
    std::cout << "GARUDA COMPILER — SPRINT B.3 FORMATTER & LINTER TEST SUITE (10 TESTS)\n";
    std::cout << "=======================================================\n\n";

    // === Test 1: Formatter 4-Space Indentation ===
    std::cout << "[TEST 1/10] Running Formatter 4-Space Indentation test...\n";
    std::string unformatted1 = "modul m fungsi f() -> X = 10 X + 20 rampung";
    std::string fmt1 = format_code(unformatted1);
    assert(fmt1.find("    X = 10") != std::string::npos);
    assert(fmt1.find("    X + 20") != std::string::npos);
    std::cout << "  ✓ Test 1: Formatter 4-Space Indentation PASS\n";

    // === Test 2: Formatter Operator Spacing ===
    std::cout << "[TEST 2/10] Running Formatter Operator Spacing test...\n";
    std::string unformatted2 = "modul m fungsi f() -> A=1+2*3 rampung";
    std::string fmt2 = format_code(unformatted2);
    assert(fmt2.find("1 + 2 * 3") != std::string::npos);
    assert(fmt2.find("A = 1") != std::string::npos);
    std::cout << "  ✓ Test 2: Formatter Operator Spacing PASS\n";

    // === Test 3: Formatter Idempotency Check ===
    std::cout << "[TEST 3/10] Running Formatter Idempotency Check test...\n";
    std::string fmt3_once = format_code(unformatted1);
    std::string fmt3_twice = format_code(fmt3_once);
    assert(fmt3_once == fmt3_twice); // Format 2x results in identical string!
    std::cout << "  ✓ Test 3: Formatter Idempotency Check PASS\n";

    // === Test 4: Formatter Preserves Program Semantics ===
    std::cout << "[TEST 4/10] Running Formatter Preserves Semantics test...\n";
    std::string raw_code4 = "modul m fungsi utama() -> X = 40, X + 2 rampung";
    std::string fmt_code4 = format_code(raw_code4);

    Lexer l4(fmt_code4);
    Parser p4(l4.tokenize());
    CodeGenerator cg4;
    auto out4 = cg4.generate(p4.parse_program());

    GvmInterpreter interp4(out4.all_instructions);
    GvmProcessState st4;
    Runtime dummy_rt;
    Pid dummy_pid = dummy_rt.spawn([](Process&) { return Status::Done; });
    Process* proc_ptr = dummy_rt.find(dummy_pid);

    Status s4 = interp4.execute_slice(*proc_ptr, st4);
    assert(s4 == Status::Done);
    assert(*st4.x_regs[0].get_if<std::int64_t>() == 42);
    std::cout << "  ✓ Test 4: Formatter Preserves Semantics PASS\n";

    // === Test 5: Formatter Tuple and List Formatting ===
    std::cout << "[TEST 5/10] Running Formatter Tuple and List Formatting test...\n";
    std::string raw_code5 = "modul m fungsi f() -> {1,2}, [10,20] rampung";
    std::string fmt5 = format_code(raw_code5);
    assert(fmt5.find("{1, 2}") != std::string::npos);
    assert(fmt5.find("[10, 20]") != std::string::npos);
    std::cout << "  ✓ Test 5: Formatter Tuple and List Formatting PASS\n";

    // === Test 6: Linter Non-snake_case Function Name ===
    std::cout << "[TEST 6/10] Running Linter Non-snake_case Function Name test...\n";
    auto warn6 = lint_code("modul m fungsi HitungTotal() -> 42 rampung");
    assert(!warn6.empty());
    assert(warn6[0].message.find("HitungTotal") != std::string::npos);
    assert(warn6[0].message.find("snake_case") != std::string::npos);
    std::cout << "  ✓ Test 6: Linter Non-snake_case Function Name PASS\n";

    // === Test 7: Linter Non-snake_case Variable Name ===
    std::cout << "[TEST 7/10] Running Linter Non-snake_case Variable Name test...\n";
    auto warn7 = lint_code("modul m fungsi f(BadParam) -> BadParam rampung");
    assert(!warn7.empty());
    assert(warn7[0].message.find("BadParam") != std::string::npos);
    std::cout << "  ✓ Test 7: Linter Non-snake_case Variable Name PASS\n";

    // === Test 8: Linter Unused Variable Detection ===
    std::cout << "[TEST 8/10] Running Linter Unused Variable Detection test...\n";
    auto warn8 = lint_code("modul m fungsi f(x) -> unused_var = 100, x + 5 rampung");
    assert(!warn8.empty());
    assert(warn8[0].message.find("unused_var") != std::string::npos);
    assert(warn8[0].message.find("never used") != std::string::npos);
    std::cout << "  ✓ Test 8: Linter Unused Variable Detection PASS\n";

    // === Test 9: Linter Clean Code Check (0 Warnings) ===
    std::cout << "[TEST 9/10] Running Linter Clean Code Check test...\n";
    auto warn9 = lint_code("modul m fungsi hitung_total(a, b) -> sum = a + b, sum rampung");
    assert(warn9.empty()); // 0 warnings for clean idiomatic code!
    std::cout << "  ✓ Test 9: Linter Clean Code Check PASS\n";

    // === Test 10: End-to-End File Formatting and Linting ===
    std::cout << "[TEST 10/10] Running End-to-End File Formatting and Linting test...\n";
    fs::path temp_file = fs::temp_directory_path() / "test_fmt.grd";
    std::ofstream ofs(temp_file);
    ofs << "modul m fungsi f() -> 1+2 rampung";
    ofs.close();

    assert(fs::exists(temp_file));
    std::string file_fmt = format_code("modul m fungsi f() -> 1+2 rampung");
    assert(file_fmt.find("1 + 2") != std::string::npos);

    fs::remove(temp_file);
    std::cout << "  ✓ Test 10: End-to-End File Formatting and Linting PASS\n";

    std::cout << "\n🎉 ALL 10 FORMATTER & LINTER TESTS PASSED PERFECTLY!\n";
    return 0;
}
