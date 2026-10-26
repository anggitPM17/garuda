#include "../include/garuda/compiler/parser.hpp"
#include <cassert>
#include <iostream>

using namespace garuda::compiler;

// Helper
std::shared_ptr<ModuleNode> parse_src(const std::string& src) {
    Lexer lex(src);
    Parser p(lex.tokenize());
    return p.parse_program();
}

std::shared_ptr<AstNode> parse_expr_src(const std::string& src) {
    auto module = parse_src("modul test fungsi f() -> " + src + " rampung");
    return module->functions()[0]->body()[0];
}

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
    std::cout << "GARUDA COMPILER — SPRINT 1.2 PARSER TEST SUITE (23 ASSERTIONS)\n";
    std::cout << "=======================================================\n\n";

    // === Test 1: Modul kosong ===
    auto m1 = parse_src("modul halo");
    TEST_ASSERT(m1->name() == "halo", "m1 name must be 'halo'");
    TEST_ASSERT(m1->functions().size() == 0, "m1 functions count must be 0");
    std::cout << "  ✓ Test 1: modul kosong PASS\n";

    // === Test 2: Fungsi tanpa parameter ===
    auto m2 = parse_src("modul m fungsi f() -> 42 rampung");
    TEST_ASSERT(m2->functions().size() == 1, "m2 functions count must be 1");
    TEST_ASSERT(m2->functions()[0]->name() == "f", "m2 function name must be 'f'");
    TEST_ASSERT(m2->functions()[0]->params().size() == 0, "m2 params count must be 0");
    std::cout << "  ✓ Test 2: fungsi tanpa parameter PASS\n";

    // === Test 3: Fungsi dengan parameter ===
    auto m3 = parse_src("modul m fungsi tambah(A, B) -> A + B rampung");
    assert(m3->functions()[0]->params().size() == 2);
    assert(m3->functions()[0]->params()[0] == "A");
    assert(m3->functions()[0]->params()[1] == "B");
    std::cout << "  ✓ Test 3: fungsi dengan parameter PASS\n";

    // === Test 4: Presedensi operator (krusial!) ===
    auto e4 = parse_expr_src("1 + 2 * 3");
    // HARUS jadi (1 + (2 * 3)), bukan ((1 + 2) * 3)
    assert(e4->to_string() == "(1 + (2 * 3))");
    std::cout << "  ✓ Test 4: presedensi 1 + 2 * 3 = " << e4->to_string() << " PASS\n";

    // === Test 5: Presedensi dengan kurung ===
    auto e5 = parse_expr_src("(1 + 2) * 3");
    assert(e5->to_string() == "((1 + 2) * 3)");
    std::cout << "  ✓ Test 5: presedensi (1 + 2) * 3 = " << e5->to_string() << " PASS\n";

    // === Test 6: Literal & variable ===
    auto e6 = parse_expr_src("X");
    assert(e6->type() == AstNodeType::VariableExpr);
    std::cout << "  ✓ Test 6: variable X PASS\n";

    // === Test 7: Atom ===
    auto e7 = parse_expr_src(":ok");
    assert(e7->type() == AstNodeType::AtomExpr);
    std::cout << "  ✓ Test 7: atom :ok PASS\n";

    // === Test 8: Boolean & nil ===
    assert(parse_expr_src("benar")->type() == AstNodeType::BoolExpr);
    assert(parse_expr_src("salah")->type() == AstNodeType::BoolExpr);
    assert(parse_expr_src("nil")->type() == AstNodeType::NilExpr);
    std::cout << "  ✓ Test 8: boolean & nil PASS\n";

    // === Test 9: Tuple literal ===
    auto e9 = parse_expr_src("{1, 2, :ok}");
    assert(e9->type() == AstNodeType::TupleExpr);
    assert(e9->to_string() == "{1, 2, :ok}");
    std::cout << "  ✓ Test 9: tuple {1, 2, :ok} PASS\n";

    // === Test 10: List literal ===
    auto e10 = parse_expr_src("[1, 2, 3]");
    assert(e10->type() == AstNodeType::ListExpr);
    assert(e10->to_string() == "[1, 2, 3]");
    std::cout << "  ✓ Test 10: list [1, 2, 3] PASS\n";

    // === Test 11: Assignment ===
    auto e11 = parse_expr_src("X = 42");
    assert(e11->type() == AstNodeType::AssignmentExpr);
    std::cout << "  ✓ Test 11: assignment X = 42 PASS\n";

    // === Test 12: Call ===
    auto e12 = parse_expr_src("tambah(1, 2)");
    assert(e12->type() == AstNodeType::CallExpr);
    std::cout << "  ✓ Test 12: call tambah(1, 2) PASS\n";

    // === Test 13: Cetak ===
    auto e13 = parse_expr_src("cetak(42)");
    assert(e13->type() == AstNodeType::CetakExpr);
    std::cout << "  ✓ Test 13: cetak(42) PASS\n";

    // === Test 14: Send (Pid <- msg) ===
    auto e14 = parse_expr_src("Pid <- {halo}");
    assert(e14->type() == AstNodeType::SendExpr);
    std::cout << "  ✓ Test 14: send Pid <- {halo} PASS\n";

    // === Test 15: Spawn ===
    auto e15 = parse_expr_src("cakra(fungsi() -> 1 rampung)");
    assert(e15->type() == AstNodeType::SpawnExpr);
    std::cout << "  ✓ Test 15: spawn cakra(fungsi() -> 1 rampung) PASS\n";

    // === Test 16: Anonymous fun ===
    auto e16 = parse_expr_src("fungsi(X) -> X + 1 rampung");
    assert(e16->type() == AstNodeType::AnonFunExpr);
    std::cout << "  ✓ Test 16: anon fungsi(X) -> X + 1 rampung PASS\n";

    // === Test 17: If yen ===
    auto m17 = parse_src(R"(
        modul m
        fungsi f(X) ->
            yen X > 0 ->
                1
            lain ->
                0
            rampung
        rampung
    )");
    auto e17 = m17->functions()[0]->body()[0];
    assert(e17->type() == AstNodeType::IfExpr);
    std::cout << "  ✓ Test 17: if yen...lain...rampung PASS\n";

    // === Test 18: Case tinjau ===
    auto m18 = parse_src(R"(
        modul m
        fungsi f(X) ->
            tinjau X ing
                1 -> :satu
                _ -> :lain
            rampung
        rampung
    )");
    auto e18 = m18->functions()[0]->body()[0];
    assert(e18->type() == AstNodeType::CaseExpr);
    std::cout << "  ✓ Test 18: case tinjau...ing...rampung PASS\n";

    // === Test 19: Receive tampa ===
    auto m19 = parse_src(R"(
        modul m
        fungsi f() ->
            tampa
                {halo, Dari} -> Dari
            rampung
        rampung
    )");
    auto e19 = m19->functions()[0]->body()[0];
    assert(e19->type() == AstNodeType::ReceiveExpr);
    std::cout << "  ✓ Test 19: receive tampa...rampung PASS\n";

    // === Test 20: Unary operator ===
    auto e20 = parse_expr_src("-5");
    assert(e20->type() == AstNodeType::UnaryExpr);
    assert(e20->to_string() == "(-5)");
    std::cout << "  ✓ Test 20: unary -5 PASS\n";

    // === Test 21: Logical bukan ===
    auto e21 = parse_expr_src("bukan benar");
    assert(e21->type() == AstNodeType::UnaryExpr);
    std::cout << "  ✓ Test 21: bukan benar PASS\n";

    // === Test 22: Concat ++ ===
    auto e22 = parse_expr_src("\"a\" ++ \"b\" ++ \"c\"");
    assert(e22->type() == AstNodeType::BinaryExpr);
    assert(e22->to_string() == "((\"a\" ++ \"b\") ++ \"c\")");  // left-assoc
    std::cout << "  ✓ Test 22: concat \"a\" ++ \"b\" ++ \"c\" PASS\n";

    // === Test 23: Nested tuple ===
    auto e23 = parse_expr_src("{a, {b, c}}");
    assert(e23->type() == AstNodeType::TupleExpr);
    std::cout << "  ✓ Test 23: nested tuple {a, {b, c}} PASS\n";

    std::cout << "\n🎉 ALL 23 PARSER ASSERTIONS PASSED PERFECTLY!\n";
    return 0;
}
