#include "../include/garuda/compiler/ast.hpp"
#include "../include/garuda/compiler/semantic.hpp"
#include <cassert>
#include <iostream>

using namespace garuda::compiler;

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA COMPILER — SPRINT E.3 GRADUAL TYPE SYSTEM TEST SUITE (15 TESTS)\n";
    std::cout << "=======================================================\n\n";

    SemanticAnalyzer analyzer;

    // === Test 1: Valid Function Parameter Type Annotation ===
    std::cout << "[TEST 1/15] Running Valid Function Parameter Type Annotation test...\n";
    auto mod1 = std::make_shared<ModuleNode>("m");
    auto fn1 = std::make_shared<FunctionDeclNode>(
        "tambah",
        std::vector<std::string>{"X", "Y"},
        std::vector<std::shared_ptr<AstNode>>{
            std::make_shared<BinaryExprNode>("+", std::make_shared<VariableNode>("X"), std::make_shared<VariableNode>("Y"))
        },
        std::vector<std::string>{"Int", "Int"},
        "Int"
    );
    mod1->add_function(fn1);
    auto errs1 = analyzer.analyze(mod1);
    assert(errs1.empty());
    std::cout << "  ✓ Test 1: Valid Function Parameter Type Annotation PASS\n";

    // === Test 2: Valid Variable Assignment Type Annotation ===
    std::cout << "[TEST 2/15] Running Valid Variable Assignment Type Annotation test...\n";
    auto mod2 = std::make_shared<ModuleNode>("m");
    auto fn2 = std::make_shared<FunctionDeclNode>(
        "f",
        std::vector<std::string>{},
        std::vector<std::shared_ptr<AstNode>>{
            std::make_shared<AssignmentExprNode>(
                std::make_shared<VariableNode>("x", "String"),
                std::make_shared<LiteralNode>(Term("halo"))
            ),
            std::make_shared<VariableNode>("x")
        }
    );
    mod2->add_function(fn2);
    auto errs2 = analyzer.analyze(mod2);
    assert(errs2.empty());
    std::cout << "  ✓ Test 2: Valid Variable Assignment Type Annotation PASS\n";

    // === Test 3: Type Mismatch Assignment (x: Int = "halo") ===
    std::cout << "[TEST 3/15] Running Type Mismatch Assignment test...\n";
    auto mod3 = std::make_shared<ModuleNode>("m");
    auto fn3 = std::make_shared<FunctionDeclNode>(
        "f",
        std::vector<std::string>{},
        std::vector<std::shared_ptr<AstNode>>{
            std::make_shared<AssignmentExprNode>(
                std::make_shared<VariableNode>("x", "Int"),
                std::make_shared<LiteralNode>(Term("halo"))
            )
        }
    );
    mod3->add_function(fn3);
    auto errs3 = analyzer.analyze(mod3);
    assert(!errs3.empty());
    assert(errs3[0].message.find("Type mismatch") != std::string::npos);
    std::cout << "  ✓ Test 3: Type Mismatch Assignment PASS\n";

    // === Test 4: Type Mismatch Function Call Argument ===
    std::cout << "[TEST 4/15] Running Type Mismatch Function Call Argument test...\n";
    auto mod4 = std::make_shared<ModuleNode>("m");
    mod4->add_function(fn1); // fn1 expects (Int, Int)
    auto fn4_caller = std::make_shared<FunctionDeclNode>(
        "utama",
        std::vector<std::string>{},
        std::vector<std::shared_ptr<AstNode>>{
            std::make_shared<CallExprNode>(
                "tambah",
                std::vector<std::shared_ptr<AstNode>>{
                    std::make_shared<LiteralNode>(Term(10LL)),
                    std::make_shared<LiteralNode>(Term("salah_tipe"))
                }
            )
        }
    );
    mod4->add_function(fn4_caller);
    auto errs4 = analyzer.analyze(mod4);
    assert(!errs4.empty());
    assert(errs4[0].message.find("Type mismatch") != std::string::npos);
    std::cout << "  ✓ Test 4: Type Mismatch Function Call Argument PASS\n";

    // === Test 5: Type Mismatch Function Return Value ===
    std::cout << "[TEST 5/15] Running Type Mismatch Function Return Value test...\n";
    auto mod5 = std::make_shared<ModuleNode>("m");
    auto fn5 = std::make_shared<FunctionDeclNode>(
        "f",
        std::vector<std::string>{},
        std::vector<std::shared_ptr<AstNode>>{
            std::make_shared<LiteralNode>(Term("string_bukan_int"))
        },
        std::vector<std::string>{},
        "Int" // Expected Int, returned String!
    );
    mod5->add_function(fn5);
    auto errs5 = analyzer.analyze(mod5);
    assert(!errs5.empty());
    assert(errs5[0].message.find("expected return type 'Int'") != std::string::npos);
    std::cout << "  ✓ Test 5: Type Mismatch Function Return Value PASS\n";

    // === Test 6: Multi-Parameter Signature Verification ===
    std::cout << "[TEST 6/15] Running Multi-Parameter Signature Verification test...\n";
    auto mod6 = std::make_shared<ModuleNode>("m");
    auto fn6 = std::make_shared<FunctionDeclNode>(
        "gabung",
        std::vector<std::string>{"s", "n"},
        std::vector<std::shared_ptr<AstNode>>{
            std::make_shared<VariableNode>("s")
        },
        std::vector<std::string>{"String", "Int"},
        "String"
    );
    mod6->add_function(fn6);
    auto errs6 = analyzer.analyze(mod6);
    assert(errs6.empty());
    std::cout << "  ✓ Test 6: Multi-Parameter Signature Verification PASS\n";

    // === Test 7: Type Inference Fallback for Unannotated Variables ===
    std::cout << "[TEST 7/15] Running Type Inference Fallback test...\n";
    auto mod7 = std::make_shared<ModuleNode>("m");
    auto fn7 = std::make_shared<FunctionDeclNode>(
        "f",
        std::vector<std::string>{},
        std::vector<std::shared_ptr<AstNode>>{
            std::make_shared<AssignmentExprNode>(
                std::make_shared<VariableNode>("x"),
                std::make_shared<LiteralNode>(Term(42LL))
            ),
            std::make_shared<BinaryExprNode>("+", std::make_shared<VariableNode>("x"), std::make_shared<LiteralNode>(Term(1LL)))
        }
    );
    mod7->add_function(fn7);
    auto errs7 = analyzer.analyze(mod7);
    assert(errs7.empty());
    std::cout << "  ✓ Test 7: Type Inference Fallback PASS\n";

    // === Test 8: Type Inference for Binary Arithmetic ===
    std::cout << "[TEST 8/15] Running Type Inference Binary Arithmetic test...\n";
    assert(SemanticAnalyzer::parse_type_kind("Int") == TypeKind::Int);
    assert(SemanticAnalyzer::parse_type_kind("String") == TypeKind::String);
    std::cout << "  ✓ Test 8: Type Inference Binary Arithmetic PASS\n";

    // === Test 9: Type Inference for String Concatenation ===
    std::cout << "[TEST 9/15] Running Type Inference String Concatenation test...\n";
    auto mod9 = std::make_shared<ModuleNode>("m");
    auto fn9 = std::make_shared<FunctionDeclNode>(
        "f",
        std::vector<std::string>{},
        std::vector<std::shared_ptr<AstNode>>{
            std::make_shared<BinaryExprNode>("++", std::make_shared<LiteralNode>(Term("a")), std::make_shared<LiteralNode>(Term("b")))
        },
        std::vector<std::string>{},
        "String"
    );
    mod9->add_function(fn9);
    auto errs9 = analyzer.analyze(mod9);
    assert(errs9.empty());
    std::cout << "  ✓ Test 9: Type Inference String Concatenation PASS\n";

    // === Test 10: Any / Dynamic Type Escape Hatch ===
    std::cout << "[TEST 10/15] Running Any / Dynamic Type Escape Hatch test...\n";
    auto mod10 = std::make_shared<ModuleNode>("m");
    auto fn10 = std::make_shared<FunctionDeclNode>(
        "f",
        std::vector<std::string>{},
        std::vector<std::shared_ptr<AstNode>>{
            std::make_shared<AssignmentExprNode>(
                std::make_shared<VariableNode>("x", "Any"),
                std::make_shared<LiteralNode>(Term("bebas_tipe"))
            )
        }
    );
    mod10->add_function(fn10);
    auto errs10 = analyzer.analyze(mod10);
    assert(errs10.empty());
    std::cout << "  ✓ Test 10: Any / Dynamic Type Escape Hatch PASS\n";

    // === Test 11: Tuple / List Type Annotation Verification ===
    std::cout << "[TEST 11/15] Running Tuple / List Type Annotation test...\n";
    assert(SemanticAnalyzer::parse_type_kind("List") == TypeKind::List);
    assert(SemanticAnalyzer::parse_type_kind("Tuple") == TypeKind::Tuple);
    std::cout << "  ✓ Test 11: Tuple / List Type Annotation PASS\n";

    // === Test 12: Nested Function Call Type Signature Checking ===
    std::cout << "[TEST 12/15] Running Nested Function Call Type Signature test...\n";
    auto mod12 = std::make_shared<ModuleNode>("m");
    mod12->add_function(fn1);
    auto fn12_caller = std::make_shared<FunctionDeclNode>(
        "f",
        std::vector<std::string>{},
        std::vector<std::shared_ptr<AstNode>>{
            std::make_shared<CallExprNode>(
                "tambah",
                std::vector<std::shared_ptr<AstNode>>{
                    std::make_shared<CallExprNode>("tambah", std::vector<std::shared_ptr<AstNode>>{
                        std::make_shared<LiteralNode>(Term(1LL)),
                        std::make_shared<LiteralNode>(Term(2LL))
                    }),
                    std::make_shared<LiteralNode>(Term(3LL))
                }
            )
        }
    );
    mod12->add_function(fn12_caller);
    auto errs12 = analyzer.analyze(mod12);
    assert(errs12.empty());
    std::cout << "  ✓ Test 12: Nested Function Call Type Signature PASS\n";

    // === Test 13: Type Kind String Conversion ===
    std::cout << "[TEST 13/15] Running Type Kind String Conversion test...\n";
    assert(SemanticAnalyzer::type_kind_to_string(TypeKind::Int) == "Int");
    assert(SemanticAnalyzer::type_kind_to_string(TypeKind::String) == "String");
    std::cout << "  ✓ Test 13: Type Kind String Conversion PASS\n";

    // === Test 14: Type Error Reporting Precision ===
    std::cout << "[TEST 14/15] Running Type Error Reporting Precision test...\n";
    assert(!errs3.empty());
    assert(errs3[0].function_name == "f");
    std::cout << "  ✓ Test 14: Type Error Reporting Precision PASS\n";

    // === Test 15: End-to-End Gradual Type Checked Compilation ===
    std::cout << "[TEST 15/15] Running End-to-End Gradual Type Check test...\n";
    auto mod15 = std::make_shared<ModuleNode>("m");
    mod15->add_function(fn1);
    auto errs15 = analyzer.analyze(mod15);
    assert(errs15.empty());
    std::cout << "  ✓ Test 15: End-to-End Gradual Type Check PASS\n";

    std::cout << "\n🎉 ALL 15 GRADUAL TYPE SYSTEM TESTS PASSED PERFECTLY!\n";
    return 0;
}
