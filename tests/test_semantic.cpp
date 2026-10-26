#include "../include/garuda/compiler/lexer.hpp"
#include "../include/garuda/compiler/parser.hpp"
#include "../include/garuda/compiler/semantic.hpp"
#include <cassert>
#include <iostream>

using namespace garuda::compiler;

std::vector<SemanticError> analyze_code(const std::string& src) {
    Lexer lexer(src);
    Parser parser(lexer.tokenize());
    auto module = parser.parse_program();

    SemanticAnalyzer analyzer;
    return analyzer.analyze(module);
}

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA COMPILER — SPRINT A.1 SEMANTIC ANALYZER TEST SUITE (20 TESTS)\n";
    std::cout << "=======================================================\n\n";

    // === Category 1: Unbound Variables (5 Tests) ===
    std::cout << "[TEST 1/20] Running Unbound Variable 1 test...\n";
    auto err1 = analyze_code("modul m fungsi f() -> X rampung");
    assert(!err1.empty() && err1[0].message.find("Unbound variable 'X'") != std::string::npos);
    std::cout << "  ✓ Test 1: Unbound variable 'X' detected PASS\n";

    std::cout << "[TEST 2/20] Running Unbound Variable 2 test...\n";
    auto err2 = analyze_code("modul m fungsi f(A) -> A + Y rampung");
    assert(!err2.empty() && err2[0].message.find("Unbound variable 'Y'") != std::string::npos);
    std::cout << "  ✓ Test 2: Unbound variable 'Y' detected PASS\n";

    std::cout << "[TEST 3/20] Running Unbound Variable 3 test...\n";
    auto err3 = analyze_code("modul m fungsi f() -> {1, Z} rampung");
    assert(!err3.empty() && err3[0].message.find("Unbound variable 'Z'") != std::string::npos);
    std::cout << "  ✓ Test 3: Unbound variable 'Z' in tuple detected PASS\n";

    std::cout << "[TEST 4/20] Running Unbound Variable 4 test...\n";
    auto err4 = analyze_code("modul m fungsi f() -> yen Cond -> 1 lain -> 2 rampung rampung");
    assert(!err4.empty() && err4[0].message.find("Unbound variable 'Cond'") != std::string::npos);
    std::cout << "  ✓ Test 4: Unbound variable in 'if' condition detected PASS\n";

    std::cout << "[TEST 5/20] Running Unbound Variable 5 test...\n";
    auto err5 = analyze_code("modul m fungsi g(A) -> A rampung fungsi f() -> g(UnboundVar) rampung");
    assert(!err5.empty() && err5[0].message.find("Unbound variable 'UnboundVar'") != std::string::npos);
    std::cout << "  ✓ Test 5: Unbound variable in function argument detected PASS\n";

    // === Category 2: Arity Mismatch (5 Tests) ===
    std::cout << "[TEST 6/20] Running Arity Mismatch 1 test...\n";
    auto err6 = analyze_code("modul m fungsi tambah(A, B) -> A + B rampung fungsi f() -> tambah(1) rampung");
    assert(!err6.empty() && err6[0].message.find("Arity mismatch") != std::string::npos);
    std::cout << "  ✓ Test 6: Arity mismatch (expected 2, got 1) detected PASS\n";

    std::cout << "[TEST 7/20] Running Arity Mismatch 2 test...\n";
    auto err7 = analyze_code("modul m fungsi hitung(X, Y) -> X + Y rampung fungsi f() -> hitung(1, 2, 3) rampung");
    assert(!err7.empty() && err7[0].message.find("Arity mismatch") != std::string::npos);
    std::cout << "  ✓ Test 7: Arity mismatch (expected 2, got 3) detected PASS\n";

    std::cout << "[TEST 8/20] Running Arity Mismatch 3 test...\n";
    auto err8 = analyze_code("modul m fungsi cetak_salam(Nama) -> Nama rampung fungsi f() -> cetak_salam() rampung");
    assert(!err8.empty() && err8[0].message.find("Arity mismatch") != std::string::npos);
    std::cout << "  ✓ Test 8: Arity mismatch (expected 1, got 0) detected PASS\n";

    std::cout << "[TEST 9/20] Running Arity Mismatch 4 test...\n";
    auto err9 = analyze_code("modul m fungsi g() -> 42 rampung fungsi f() -> g(1, 2) rampung");
    assert(!err9.empty() && err9[0].message.find("Arity mismatch") != std::string::npos);
    std::cout << "  ✓ Test 9: Arity mismatch on zero-arg function detected PASS\n";

    std::cout << "[TEST 10/20] Running Arity Mismatch 5 test...\n";
    auto err10 = analyze_code("modul m fungsi add(A, B) -> A + B rampung fungsi f() -> add(add(1), 2) rampung");
    assert(!err10.empty() && err10[0].message.find("Arity mismatch") != std::string::npos);
    std::cout << "  ✓ Test 10: Nested function arity mismatch detected PASS\n";

    // === Category 3: Type Mismatch / Operator Misuse (5 Tests) ===
    std::cout << "[TEST 11/20] Running Type Mismatch 1 test...\n";
    auto err11 = analyze_code("modul m fungsi f() -> \"hello\" + 42 rampung");
    assert(!err11.empty() && err11[0].message.find("Type mismatch") != std::string::npos);
    std::cout << "  ✓ Test 11: Arithmetic on String (\"hello\" + 42) detected PASS\n";

    std::cout << "[TEST 12/20] Running Type Mismatch 2 test...\n";
    auto err12 = analyze_code("modul m fungsi f() -> 100 - \"dunia\" rampung");
    assert(!err12.empty() && err12[0].message.find("Type mismatch") != std::string::npos);
    std::cout << "  ✓ Test 12: Subtraction on String (100 - \"dunia\") detected PASS\n";

    std::cout << "[TEST 13/20] Running Type Mismatch 3 test...\n";
    auto err13 = analyze_code("modul m fungsi f() -> 10 ++ 20 rampung");
    assert(!err13.empty() && err13[0].message.find("Type mismatch") != std::string::npos);
    std::cout << "  ✓ Test 13: String Concatenation '++' on Int detected PASS\n";

    std::cout << "[TEST 14/20] Running Type Mismatch 4 test...\n";
    auto err14 = analyze_code("modul m fungsi f() -> \"a\" * \"b\" rampung");
    assert(!err14.empty() && err14[0].message.find("Type mismatch") != std::string::npos);
    std::cout << "  ✓ Test 14: Multiplication on String detected PASS\n";

    std::cout << "[TEST 15/20] Running Duplicate Parameter test...\n";
    auto err15 = analyze_code("modul m fungsi bad(X, X) -> X rampung");
    assert(!err15.empty() && err15[0].message.find("Duplicate parameter name 'X'") != std::string::npos);
    std::cout << "  ✓ Test 15: Duplicate parameter name 'X' detected PASS\n";

    // === Category 4: Valid Programs (5 Tests) ===
    std::cout << "[TEST 16/20] Running Valid Program 1 test...\n";
    auto err16 = analyze_code("modul m fungsi f() -> 5 + 3 rampung");
    assert(err16.empty());
    std::cout << "  ✓ Test 16: Valid simple arithmetic PASS\n";

    std::cout << "[TEST 17/20] Running Valid Program 2 test...\n";
    auto err17 = analyze_code("modul m fungsi f() -> X = 42, X + 10 rampung");
    if (!err17.empty()) {
        for (const auto& e : err17) {
            std::cout << "  DEBUG ERR17: " << e.message << "\n";
        }
    }
    assert(err17.empty());
    std::cout << "  ✓ Test 17: Valid variable binding (X = 42, X + 10) PASS\n";

    std::cout << "[TEST 18/20] Running Valid Program 3 test...\n";
    auto err18 = analyze_code("modul m fungsi tambah(A, B) -> A + B rampung fungsi f() -> tambah(1, 2) rampung");
    assert(err18.empty());
    std::cout << "  ✓ Test 18: Valid function call tambah(1, 2) PASS\n";

    std::cout << "[TEST 19/20] Running Valid Program 4 test...\n";
    auto err19 = analyze_code("modul m fungsi f() -> tampa {pesan, Val} -> Val rampung rampung");
    assert(err19.empty());
    std::cout << "  ✓ Test 19: Valid receive pattern binding PASS\n";

    std::cout << "[TEST 20/20] Running Valid Program 5 test...\n";
    auto err20 = analyze_code("modul m fungsi f(A) -> yen A > 0 -> 100 lain -> 200 rampung rampung");
    assert(err20.empty());
    std::cout << "  ✓ Test 20: Valid 'yen'...'lain' conditional logic PASS\n";

    std::cout << "\n🎉 ALL 20 SEMANTIC ANALYZER TESTS PASSED PERFECTLY!\n";
    return 0;
}
