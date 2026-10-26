#include "../include/garuda/compiler/lexer.hpp"
#include <cassert>
#include <iostream>

using namespace garuda::compiler;

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
    std::cout << "GARUDA COMPILER — SPRINT 1.1 LEXER TEST SUITE (30+ ASSERTIONS)\n";
    std::cout << "=======================================================\n\n";

    // 1. Nusantara Keywords Test
    std::string source1 = "modul halo fungsi mulai() -> 42 rampung";
    Lexer lex1(source1);
    auto tok1 = lex1.tokenize();
    TEST_ASSERT(tok1.size() >= 8, "Token count must be >= 8");
    TEST_ASSERT(tok1[0].type == TokenType::DefModule, "tok1[0] must be DefModule");
    TEST_ASSERT(tok1[0].text == "modul", "tok1[0] text must be 'modul'");
    TEST_ASSERT(tok1[1].type == TokenType::Identifier, "tok1[1] must be Identifier");
    TEST_ASSERT(tok1[1].text == "halo", "tok1[1] text must be 'halo'");
    TEST_ASSERT(tok1[2].type == TokenType::DefFunc, "tok1[2] must be DefFunc");
    TEST_ASSERT(tok1[2].text == "fungsi", "tok1[2] text must be 'fungsi'");
    TEST_ASSERT(tok1[3].type == TokenType::Identifier, "tok1[3] must be Identifier");
    TEST_ASSERT(tok1[3].text == "mulai", "tok1[3] text must be 'mulai'");
    TEST_ASSERT(tok1[4].type == TokenType::LParen, "tok1[4] must be LParen");
    TEST_ASSERT(tok1[5].type == TokenType::RParen, "tok1[5] must be RParen");
    TEST_ASSERT(tok1[6].type == TokenType::Arrow, "tok1[6] must be Arrow");
    TEST_ASSERT(tok1[7].type == TokenType::Number, "tok1[7] must be Number");
    TEST_ASSERT(tok1[8].type == TokenType::End, "tok1[8] must be End");
    std::cout << "  ✓ 1. Nusantara Keywords Test PASS" << std::endl;

    // 2. Concurrency Nusantara Keywords Test (cakra, tampa, setelah, mili, diri, kirim)
    std::string source2 = "Pid = cakra(fungsi() -> tampa {halo, Dari} -> Dari <- {balasan, \"Sugeng rawung!\"} rampung rampung)";
    Lexer lex2(source2);
    auto tok2 = lex2.tokenize();
    assert(tok2[2].type == TokenType::Spawn);      // Assertion 14: cakra -> Spawn
    assert(tok2[6].type == TokenType::Receive);    // Assertion 15: tampa -> Receive
    assert(tok2[12].type == TokenType::SendOp);    // Assertion 16: <- -> SendOp
    assert(tok2[15].type == TokenType::String);    // Assertion 17: "Sugeng rawung!"
    assert(tok2[15].text == "Sugeng rawung!");     // Assertion 18
    std::cout << "  ✓ 2. Concurrency Nusantara Keywords Test PASS" << std::endl;

    // 3. Number Formats Test (Hex, Binary, Float, Int)
    std::string source3 = "42 0x1F 0b1010 3.14159";
    Lexer lex3(source3);
    auto tok3 = lex3.tokenize();
    assert(tok3[0].type == TokenType::Number && tok3[0].text == "42");       // Assertion 19
    assert(tok3[1].type == TokenType::Number && tok3[1].text == "0x1F");     // Assertion 20
    assert(tok3[2].type == TokenType::Number && tok3[2].text == "0b1010");   // Assertion 21
    assert(tok3[3].type == TokenType::Float && tok3[3].text == "3.14159");   // Assertion 22
    std::cout << "  ✓ 3. Number Formats Test PASS" << std::endl;

    // 4. String Escape Sequences Test (\n, \t, \")
    std::string source4 = "\"Baris 1\\nBaris 2\\ttabbed \\\"quoted\\\"\"";
    Lexer lex4(source4);
    auto tok4 = lex4.tokenize();
    assert(tok4[0].type == TokenType::String);                               // Assertion 23
    assert(tok4[0].text == "Baris 1\nBaris 2\ttabbed \"quoted\"");           // Assertion 24
    std::cout << "  ✓ 4. String Escape Sequences Test PASS" << std::endl;

    // 5. Atoms, Booleans & Nil Test (:ok, :error, benar, salah, nil)
    std::string source5 = ":ok :error benar salah nil";
    Lexer lex5(source5);
    auto tok5 = lex5.tokenize();
    assert(tok5[0].type == TokenType::Atom && tok5[0].text == "ok");         // Assertion 25
    assert(tok5[1].type == TokenType::Atom && tok5[1].text == "error");      // Assertion 26
    assert(tok5[2].type == TokenType::Boolean && tok5[2].text == "benar");   // Assertion 27
    assert(tok5[3].type == TokenType::Boolean && tok5[3].text == "salah");   // Assertion 28
    assert(tok5[4].type == TokenType::Nil && tok5[4].text == "nil");         // Assertion 29
    std::cout << "  ✓ 5. Atoms, Booleans & Nil Test PASS" << std::endl;

    // 6. Comments & Line/Column Tracking Test
    std::string source6 = "-- Komentar Nusantara\n# Komentar Elixir\n  modul";
    Lexer lex6(source6);
    auto tok6 = lex6.tokenize();
    assert(tok6[0].type == TokenType::DefModule);                            // Assertion 30
    assert(tok6[0].line == 3);                                               // Assertion 31 (Line 3 after 2 comments)
    assert(tok6[0].column == 3);                                             // Assertion 32 (Col 3 after 2 spaces)
    std::cout << "  ✓ 6. Comments & Line/Column Tracking Test PASS" << std::endl;

    std::cout << "\n🎉 ALL 32 ASSERTIONS IN SPRINT 1.1 LEXER TEST SUITE PASSED PERFECTLY!\n";
    return 0;
}
