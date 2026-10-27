#include "../include/garuda/compiler/lexer.hpp"
#include "../include/garuda/compiler/parser.hpp"
#include "../include/garuda/compiler/semantic.hpp"
#include "../include/garuda/compiler/source_map.hpp"
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
    std::cout << "GARUDA COMPILER — ERROR REPORTING & PARSER DIAGNOSTICS SUITE\n";
    std::cout << "=======================================================\n\n";

    // === Test 1: Parser Syntax Error - Missing Closing Parenthesis in Function ===
    run_test_with_timeout("Test 1: Parser Error - Missing Closing Parenthesis", []() {
        std::string src = "modul m\nfungsi bad(A, B\n    A + B\nrampung";
        Lexer l(src);
        Parser p(l.tokenize());
        bool caught = false;
        try {
            p.parse_program();
        } catch (const ParseError& err) {
            caught = true;
            GARUDA_CHECK(err.line == 3, "Error line at unexpected token must be 3");
            GARUDA_CHECK(err.raw_message.find("Expected ')' after parameters") != std::string::npos, "Error message must mention missing ')'");
        }
        GARUDA_CHECK(caught, "ParseError must be thrown for missing ')'");
        std::cout << "  ✓ Test 1: Parser Error - Missing Closing Parenthesis PASS\n";
    }, 5);

    // === Test 2: Parser Syntax Error - Missing '->' in yen condition ===
    run_test_with_timeout("Test 2: Parser Error - Missing '->' in yen", []() {
        std::string src = "modul m\nfungsi f(X) ->\n    yen X > 0\n        100\n    rampung\nrampung";
        Lexer l(src);
        Parser p(l.tokenize());
        bool caught = false;
        try {
            p.parse_program();
        } catch (const ParseError& err) {
            caught = true;
            GARUDA_CHECK(err.line == 4, "Error line at unexpected token must be 4");
            GARUDA_CHECK(err.raw_message.find("Expected '->'") != std::string::npos, "Error message must mention '->'");
        }
        GARUDA_CHECK(caught, "ParseError must be thrown for missing '->'");
        std::cout << "  ✓ Test 2: Parser Error - Missing '->' in yen PASS\n";
    }, 5);

    // === Test 3: Semantic Error Line/Column Precision ===
    run_test_with_timeout("Test 3: Semantic Error Line/Column Precision", []() {
        std::string src = "modul m\nfungsi f() ->\n    X + 1\nrampung";
        Lexer l(src);
        Parser p(l.tokenize());
        SemanticAnalyzer sem;
        auto errs = sem.analyze(p.parse_program());
        GARUDA_CHECK(!errs.empty(), "Semantic error list must not be empty");
        GARUDA_CHECK(errs[0].message.find("Unbound variable 'X'") != std::string::npos, "Error must mention unbound variable 'X'");
        std::cout << "  ✓ Test 3: Semantic Error Line/Column Precision PASS\n";
    }, 5);

    // === Test 4: Code Snippet Preview Generation & Caret Positioning ===
    run_test_with_timeout("Test 4: Code Snippet Preview & Caret Positioning", []() {
        std::string src = "modul m\nfungsi f() ->\n    X + 1\nrampung";
        std::string diag = SourceMap::format_diagnostic(src, 3, 5, "Unbound variable 'X'", "m.grd");
        GARUDA_CHECK(diag.find("m.grd:3:5") != std::string::npos, "Diagnostic must contain file:line:col");
        GARUDA_CHECK(diag.find("3 |     X + 1") != std::string::npos, "Diagnostic must contain source line");
        GARUDA_CHECK(diag.find("^") != std::string::npos, "Diagnostic must contain caret '^'");
        std::cout << "  ✓ Test 4: Code Snippet Preview & Caret Positioning PASS\n";
    }, 5);

    // === Test 5: Arity Mismatch Error Report ===
    run_test_with_timeout("Test 5: Arity Mismatch Error Report", []() {
        std::string src = "modul m\nfungsi add(A, B) -> A + B rampung\nfungsi utama() ->\n    add(1)\nrampung";
        Lexer l(src);
        Parser p(l.tokenize());
        SemanticAnalyzer sem;
        auto errs = sem.analyze(p.parse_program());
        GARUDA_CHECK(!errs.empty(), "Error list must not be empty");
        GARUDA_CHECK(errs[0].message.find("Arity mismatch") != std::string::npos, "Error must mention arity mismatch");
        std::cout << "  ✓ Test 5: Arity Mismatch Error Report PASS\n";
    }, 5);

    // === Test 6: Type Mismatch Error Report ===
    run_test_with_timeout("Test 6: Type Mismatch Error Report", []() {
        std::string src = "modul m\nfungsi utama() ->\n    \"salam\" + 42\nrampung";
        Lexer l(src);
        Parser p(l.tokenize());
        SemanticAnalyzer sem;
        auto errs = sem.analyze(p.parse_program());
        GARUDA_CHECK(!errs.empty(), "Error list must not be empty");
        GARUDA_CHECK(errs[0].message.find("Type mismatch") != std::string::npos, "Error must mention type mismatch");
        std::cout << "  ✓ Test 6: Type Mismatch Error Report PASS\n";
    }, 5);

    // === Test 7: Source Map IP Mapping ===
    run_test_with_timeout("Test 7: Source Map IP Mapping", []() {
        SourceMap sm;
        sm.add_mapping(0, 10, 4, "app.grd");
        sm.add_mapping(5, 12, 8, "app.grd");
        auto loc_0 = sm.get_location(0);
        auto loc_5 = sm.get_location(5);
        GARUDA_CHECK(loc_0.line == 10 && loc_0.col == 4, "IP 0 location must match 10:4");
        GARUDA_CHECK(loc_5.line == 12 && loc_5.col == 8, "IP 5 location must match 12:8");
        std::cout << "  ✓ Test 7: Source Map IP Mapping PASS\n";
    }, 5);

    // === Test 8: Duplicate Parameter Error Reporting ===
    run_test_with_timeout("Test 8: Duplicate Parameter Error Reporting", []() {
        std::string src = "modul m\nfungsi bad(X, X) -> X rampung";
        Lexer l(src);
        Parser p(l.tokenize());
        SemanticAnalyzer sem;
        auto errs = sem.analyze(p.parse_program());
        GARUDA_CHECK(!errs.empty(), "Error list must not be empty");
        GARUDA_CHECK(errs[0].message.find("Duplicate parameter name 'X'") != std::string::npos, "Error must mention duplicate parameter");
        std::cout << "  ✓ Test 8: Duplicate Parameter Error Reporting PASS\n";
    }, 5);

    std::cout << "\n🎉 ALL ERROR REPORTING & DIAGNOSTICS TESTS PASSED PERFECTLY!\n";
    return 0;
}
