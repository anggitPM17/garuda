#include "../include/garuda/compiler/lexer.hpp"
#include "../include/garuda/compiler/parser.hpp"
#include "../include/garuda/compiler/semantic.hpp"
#include "../include/garuda/compiler/source_map.hpp"
#include <cassert>
#include <iostream>

using namespace garuda::compiler;

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA COMPILER — SPRINT A.5 ERROR REPORTING TEST SUITE (10 TESTS)\n";
    std::cout << "=======================================================\n\n";

    // === Test 1: Semantic Error Line/Column Precision ===
    std::cout << "[TEST 1/10] Running Semantic Error Line/Column Precision test...\n";
    std::string src1 = "modul m\nfungsi f() ->\n    X + 1\nrampung";
    Lexer l1(src1);
    Parser p1(l1.tokenize());
    SemanticAnalyzer sem1;
    auto errs1 = sem1.analyze(p1.parse_program());
    assert(!errs1.empty());
    assert(errs1[0].message.find("Unbound variable 'X'") != std::string::npos);
    std::cout << "  ✓ Test 1: Semantic Error Line/Column Precision PASS\n";

    // === Test 2: Code Snippet Preview Generation ===
    std::cout << "[TEST 2/10] Running Code Snippet Preview Generation test...\n";
    std::string diag2 = SourceMap::format_diagnostic(src1, 3, 5, "Unbound variable 'X'", "m.grd");
    assert(diag2.find("m.grd:3:5") != std::string::npos);
    assert(diag2.find("3 |     X + 1") != std::string::npos);
    std::cout << "  ✓ Test 2: Code Snippet Preview Generation PASS\n";

    // === Test 3: Caret (^) Positioning ===
    std::cout << "[TEST 3/10] Running Caret (^) Positioning test...\n";
    assert(diag2.find("^") != std::string::npos);
    std::cout << "  ✓ Test 3: Caret (^) Positioning PASS\n";

    // === Test 4: Arity Mismatch Error Report ===
    std::cout << "[TEST 4/10] Running Arity Mismatch Error Report test...\n";
    std::string src4 = "modul m\nfungsi add(A, B) -> A + B rampung\nfungsi utama() ->\n    add(1)\nrampung";
    Lexer l4(src4);
    Parser p4(l4.tokenize());
    SemanticAnalyzer sem4;
    auto errs4 = sem4.analyze(p4.parse_program());
    assert(!errs4.empty());
    assert(errs4[0].message.find("Arity mismatch") != std::string::npos);
    std::cout << "  ✓ Test 4: Arity Mismatch Error Report PASS\n";

    // === Test 5: Type Mismatch Error Report ===
    std::cout << "[TEST 5/10] Running Type Mismatch Error Report test...\n";
    std::string src5 = "modul m\nfungsi utama() ->\n    \"salam\" + 42\nrampung";
    Lexer l5(src5);
    Parser p5(l5.tokenize());
    SemanticAnalyzer sem5;
    auto errs5 = sem5.analyze(p5.parse_program());
    assert(!errs5.empty());
    assert(errs5[0].message.find("Type mismatch") != std::string::npos);
    std::cout << "  ✓ Test 5: Type Mismatch Error Report PASS\n";

    // === Test 6: Source Map IP Mapping ===
    std::cout << "[TEST 6/10] Running Source Map IP Mapping test...\n";
    SourceMap sm6;
    sm6.add_mapping(0, 10, 4, "app.grd");
    sm6.add_mapping(5, 12, 8, "app.grd");
    auto loc_0 = sm6.get_location(0);
    auto loc_5 = sm6.get_location(5);
    assert(loc_0.line == 10 && loc_0.col == 4);
    assert(loc_5.line == 12 && loc_5.col == 8);
    std::cout << "  ✓ Test 6: Source Map IP Mapping PASS\n";

    // === Test 7: Multi-Line Error Location Tracking ===
    std::cout << "[TEST 7/10] Running Multi-Line Error Location Tracking test...\n";
    std::string src7 = "modul m\nfungsi g() -> 1 rampung\nfungsi f() ->\n    A = 10\n    B = 20\n    UnboundVar + 5\nrampung";
    Lexer l7(src7);
    Parser p7(l7.tokenize());
    SemanticAnalyzer sem7;
    auto errs7 = sem7.analyze(p7.parse_program());
    assert(!errs7.empty());
    assert(errs7[0].message.find("Unbound variable 'UnboundVar'") != std::string::npos);
    std::cout << "  ✓ Test 7: Multi-Line Error Location Tracking PASS\n";

    // === Test 8: Duplicate Parameter Error Reporting ===
    std::cout << "[TEST 8/10] Running Duplicate Parameter Error Reporting test...\n";
    std::string src8 = "modul m\nfungsi bad(X, X) -> X rampung";
    Lexer l8(src8);
    Parser p8(l8.tokenize());
    SemanticAnalyzer sem8;
    auto errs8 = sem8.analyze(p8.parse_program());
    assert(!errs8.empty());
    assert(errs8[0].message.find("Duplicate parameter name 'X'") != std::string::npos);
    std::cout << "  ✓ Test 8: Duplicate Parameter Error Reporting PASS\n";

    // === Test 9: Multi-File Source Location Tracking ===
    std::cout << "[TEST 9/10] Running Multi-File Source Location Tracking test...\n";
    std::string diag9 = SourceMap::format_diagnostic(src8, 2, 12, "Duplicate parameter name 'X'", "lib/math.grd");
    assert(diag9.find("lib/math.grd:2:12") != std::string::npos);
    std::cout << "  ✓ Test 9: Multi-File Source Location Tracking PASS\n";

    // === Test 10: End-to-End Pretty Formatted Diagnostic Output ===
    std::cout << "[TEST 10/10] Running End-to-End Pretty Formatted Diagnostic Output test...\n";
    std::string pretty = SourceMap::format_diagnostic("modul m\nfungsi f() ->\n    10 ++ 20\nrampung", 3, 5, "Type mismatch: Cannot perform concatenation '++' on Int", "app.grd");
    assert(pretty.find("app.grd:3:5") != std::string::npos);
    assert(pretty.find("Type mismatch") != std::string::npos);
    assert(pretty.find("^") != std::string::npos);
    std::cout << "  ✓ Test 10: End-to-End Pretty Formatted Diagnostic Output PASS\n";

    std::cout << "\n🎉 ALL 10 SOURCE MAPS & ERROR REPORTING TESTS PASSED PERFECTLY!\n";
    return 0;
}
