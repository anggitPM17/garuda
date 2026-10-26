#include "../include/garuda/compiler/lexer.hpp"
#include "../include/garuda/compiler/parser.hpp"
#include "../include/garuda/compiler/semantic.hpp"
#include "../include/garuda/compiler/codegen.hpp"
#include "../include/garuda/beam/gvm/interpreter.hpp"
#include <cassert>
#include <iostream>

using namespace garuda::compiler;
using namespace garuda::beam;

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA COMPILER — SPRINT A.2 EXTENDED PATTERN MATCHING TEST SUITE (15 TESTS)\n";
    std::cout << "=======================================================\n\n";

    // === Category 1: Nested Tuple Patterns (5 Tests) ===
    std::cout << "[TEST 1/15] Running Nested Tuple Pattern {A, {B, C}} test...\n";
    auto t1 = std::make_shared<PatternTupleNode>(std::vector<std::shared_ptr<AstNode>>{
        std::make_shared<PatternVarNode>("A"),
        std::make_shared<PatternTupleNode>(std::vector<std::shared_ptr<AstNode>>{
            std::make_shared<PatternVarNode>("B"),
            std::make_shared<PatternVarNode>("C")
        })
    });
    assert(t1->type() == AstNodeType::PatternTuple);
    assert(t1->to_string() == "{A, {B, C}}");
    std::cout << "  ✓ Test 1: Nested Tuple Pattern {A, {B, C}} PASS\n";

    std::cout << "[TEST 2/15] Running Deep Nested Tuple Pattern test...\n";
    auto t2 = std::make_shared<PatternTupleNode>(std::vector<std::shared_ptr<AstNode>>{
        std::make_shared<PatternLiteralNode>(Term(1LL)),
        std::make_shared<PatternTupleNode>(std::vector<std::shared_ptr<AstNode>>{
            std::make_shared<PatternLiteralNode>(Term(2LL)),
            std::make_shared<PatternTupleNode>(std::vector<std::shared_ptr<AstNode>>{
                std::make_shared<PatternLiteralNode>(Term(3LL)),
                std::make_shared<PatternVarNode>("X")
            })
        })
    });
    assert(t2->to_string() == "{1, {2, {3, X}}}");
    std::cout << "  ✓ Test 2: Deep Nested Tuple Pattern PASS\n";

    std::cout << "[TEST 3/15] Running Nested Tuple with Literal Match test...\n";
    auto t3 = std::make_shared<PatternTupleNode>(std::vector<std::shared_ptr<AstNode>>{
        std::make_shared<PatternLiteralNode>(Term(Atom{"ok"})),
        std::make_shared<PatternTupleNode>(std::vector<std::shared_ptr<AstNode>>{
            std::make_shared<PatternLiteralNode>(Term(200LL)),
            std::make_shared<PatternVarNode>("Data")
        })
    });
    assert(t3->to_string() == "{:ok, {200, Data}}");
    std::cout << "  ✓ Test 3: Nested Tuple with Literal Match PASS\n";

    std::cout << "[TEST 4/15] Running Pattern Match Assignment test...\n";
    auto assign_node = std::make_shared<AssignmentExprNode>(
        std::make_shared<PatternTupleNode>(std::vector<std::shared_ptr<AstNode>>{
            std::make_shared<PatternVarNode>("X"),
            std::make_shared<PatternVarNode>("Y")
        }),
        std::make_shared<TupleExprNode>(std::vector<std::shared_ptr<AstNode>>{
            std::make_shared<LiteralNode>(Term(10LL)),
            std::make_shared<LiteralNode>(Term(20LL))
        })
    );
    assert(assign_node->type() == AstNodeType::AssignmentExpr);
    std::cout << "  ✓ Test 4: Pattern Match Assignment PASS\n";

    std::cout << "[TEST 5/15] Running Pattern Match Tuple Wildcard test...\n";
    auto t5 = std::make_shared<PatternTupleNode>(std::vector<std::shared_ptr<AstNode>>{
        std::make_shared<PatternVarNode>("X"),
        std::make_shared<PatternWildcardNode>()
    });
    assert(t5->to_string() == "{X, _}");
    std::cout << "  ✓ Test 5: Pattern Match Tuple Wildcard PASS\n";

    // === Category 2: List Head-Tail Pattern [H | T] (5 Tests) ===
    std::cout << "[TEST 6/15] Running List Head-Tail Pattern [H | T] test...\n";
    auto ht6 = std::make_shared<PatternListHeadTailNode>(
        std::make_shared<PatternVarNode>("H"),
        std::make_shared<PatternVarNode>("T")
    );
    assert(ht6->type() == AstNodeType::PatternListHeadTail);
    assert(ht6->to_string() == "[H | T]");
    std::cout << "  ✓ Test 6: List Head-Tail Pattern [H | T] PASS\n";

    std::cout << "[TEST 7/15] Running List Head Extract [H | _] test...\n";
    auto ht7 = std::make_shared<PatternListHeadTailNode>(
        std::make_shared<PatternVarNode>("H"),
        std::make_shared<PatternWildcardNode>()
    );
    assert(ht7->to_string() == "[H | _]");
    std::cout << "  ✓ Test 7: List Head Extract [H | _] PASS\n";

    std::cout << "[TEST 8/15] Running List Tail Extract [_ | T] test...\n";
    auto ht8 = std::make_shared<PatternListHeadTailNode>(
        std::make_shared<PatternWildcardNode>(),
        std::make_shared<PatternVarNode>("T")
    );
    assert(ht8->to_string() == "[_ | T]");
    std::cout << "  ✓ Test 8: List Tail Extract [_ | T] PASS\n";

    std::cout << "[TEST 9/15] Running List Head-Tail Semantic Binding test...\n";
    SemanticAnalyzer sem_analyzer;
    auto mod9 = std::make_shared<ModuleNode>("m");
    auto fn9 = std::make_shared<FunctionDeclNode>("f", std::vector<std::string>{}, std::vector<std::shared_ptr<AstNode>>{
        std::make_shared<AssignmentExprNode>(
            ht6,
            std::make_shared<LiteralNode>(Term(100LL))
        ),
        std::make_shared<BinaryExprNode>("+", std::make_shared<VariableNode>("H"), std::make_shared<LiteralNode>(Term(1LL)))
    });
    mod9->add_function(fn9);
    auto err9 = sem_analyzer.analyze(mod9);
    assert(err9.empty()); // H and T bound in scope!
    std::cout << "  ✓ Test 9: List Head-Tail Semantic Binding PASS\n";

    std::cout << "[TEST 10/15] Running Nested List Head-Tail Pattern test...\n";
    auto ht10 = std::make_shared<PatternListHeadTailNode>(
        std::make_shared<PatternTupleNode>(std::vector<std::shared_ptr<AstNode>>{
            std::make_shared<PatternVarNode>("K"),
            std::make_shared<PatternVarNode>("V")
        }),
        std::make_shared<PatternVarNode>("Rest")
    );
    assert(ht10->to_string() == "[{K, V} | Rest]");
    std::cout << "  ✓ Test 10: Nested List Head-Tail Pattern [{K, V} | Rest] PASS\n";

    // === Category 3: Pattern Guard Expressions (when/ketika) (5 Tests) ===
    std::cout << "[TEST 11/15] Running Pattern Guard Expression ketika X > 0 test...\n";
    auto guard11 = std::make_shared<PatternGuardNode>(
        std::make_shared<PatternVarNode>("X"),
        std::make_shared<BinaryExprNode>(">", std::make_shared<VariableNode>("X"), std::make_shared<LiteralNode>(Term(0LL)))
    );
    assert(guard11->type() == AstNodeType::PatternGuard);
    assert(guard11->to_string() == "X ketika X > 0");
    std::cout << "  ✓ Test 11: Pattern Guard Expression X ketika X > 0 PASS\n";

    std::cout << "[TEST 12/15] Running Pattern Guard Equality ketika X == 100 test...\n";
    auto guard12 = std::make_shared<PatternGuardNode>(
        std::make_shared<PatternVarNode>("X"),
        std::make_shared<BinaryExprNode>("==", std::make_shared<VariableNode>("X"), std::make_shared<LiteralNode>(Term(100LL)))
    );
    assert(guard12->to_string() == "X ketika X == 100");
    std::cout << "  ✓ Test 12: Pattern Guard Equality X ketika X == 100 PASS\n";

    std::cout << "[TEST 13/15] Running Pattern Guard Semantic Analysis test...\n";
    auto mod13 = std::make_shared<ModuleNode>("m");
    auto fn13 = std::make_shared<FunctionDeclNode>("f", std::vector<std::string>{}, std::vector<std::shared_ptr<AstNode>>{
        std::make_shared<AssignmentExprNode>(
            guard11,
            std::make_shared<LiteralNode>(Term(50LL))
        ),
        std::make_shared<VariableNode>("X")
    });
    mod13->add_function(fn13);
    auto err13 = sem_analyzer.analyze(mod13);
    assert(err13.empty()); // X bound and used in guard!
    std::cout << "  ✓ Test 13: Pattern Guard Semantic Analysis PASS\n";

    std::cout << "[TEST 14/15] Running Multiple Guard Condition Expression test...\n";
    auto guard14 = std::make_shared<PatternGuardNode>(
        std::make_shared<PatternTupleNode>(std::vector<std::shared_ptr<AstNode>>{
            std::make_shared<PatternVarNode>("X"),
            std::make_shared<PatternVarNode>("Y")
        }),
        std::make_shared<BinaryExprNode>(">", std::make_shared<VariableNode>("X"), std::make_shared<VariableNode>("Y"))
    );
    assert(guard14->to_string() == "{X, Y} ketika X > Y");
    std::cout << "  ✓ Test 14: Multiple Guard Condition Expression PASS\n";

    std::cout << "[TEST 15/15] Running Combined Nested Pattern + Guard Clause test...\n";
    auto guard15 = std::make_shared<PatternGuardNode>(
        std::make_shared<PatternListHeadTailNode>(
            std::make_shared<PatternVarNode>("Head"),
            std::make_shared<PatternVarNode>("Tail")
        ),
        std::make_shared<BinaryExprNode>("!=", std::make_shared<VariableNode>("Head"), std::make_shared<LiteralNode>(Term(0LL)))
    );
    assert(guard15->to_string() == "[Head | Tail] ketika Head != 0");
    std::cout << "  ✓ Test 15: Combined Nested Pattern + Guard Clause PASS\n";

    std::cout << "\n🎉 ALL 15 EXTENDED PATTERN MATCHING TESTS PASSED PERFECTLY!\n";
    return 0;
}
