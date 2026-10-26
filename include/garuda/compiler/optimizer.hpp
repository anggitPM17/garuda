#pragma once
// Garuda Language Compiler: AST & Bytecode Optimizer Pass (`optimizer.hpp`)
// Mengimplementasikan Constant Folding, Dead Code Elimination, & Peephole Bytecode Compaction.
#include "ast.hpp"
#include "../beam/gvm/interpreter.hpp"

#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace garuda::compiler {

using namespace garuda::beam;

class Optimizer {
public:
    Optimizer() = default;

    std::shared_ptr<ModuleNode> optimize_ast(std::shared_ptr<ModuleNode> module) {
        if (!module) return module;

        auto opt_module = std::make_shared<ModuleNode>(module->name());

        for (const auto& fn : module->functions()) {
            std::vector<std::shared_ptr<AstNode>> opt_body;
            for (const auto& stmt : fn->body()) {
                opt_body.push_back(fold_constants(stmt));
            }
            auto opt_fn = std::make_shared<FunctionDeclNode>(
                fn->name(), fn->params(), std::move(opt_body)
            );
            opt_module->add_function(opt_fn);
        }

        return opt_module;
    }

    std::vector<Instruction> optimize_bytecode(const std::vector<Instruction>& code) {
        std::vector<Instruction> opt_code;
        opt_code.reserve(code.size());

        for (std::size_t i = 0; i < code.size(); ++i) {
            const auto& inst = code[i];

            // Peephole 1: Eliminate redundant self-moves `OpMove X[a] -> X[a]`
            if (inst.op == Opcode::OpMove && inst.args.size() == 2) {
                if (inst.args[0].type == RegType::X && inst.args[1].type == RegType::X &&
                    inst.args[0].index == inst.args[1].index) {
                    continue; // Skip redundant move!
                }
            }

            opt_code.push_back(inst);
        }

        return opt_code;
    }

private:
    std::shared_ptr<AstNode> fold_constants(std::shared_ptr<AstNode> node) {
        if (!node) return nullptr;

        switch (node->type()) {
            case AstNodeType::BinaryExpr: {
                auto* bin = static_cast<BinaryExprNode*>(node.get());
                auto left = fold_constants(bin->left_ptr());
                auto right = fold_constants(bin->right_ptr());

                // Constant folding for literals
                if (left->type() == AstNodeType::LiteralExpr && right->type() == AstNodeType::LiteralExpr) {
                    auto* l_lit = static_cast<LiteralNode*>(left.get());
                    auto* r_lit = static_cast<LiteralNode*>(right.get());

                    auto* l_int = l_lit->value().get_if<std::int64_t>();
                    auto* r_int = r_lit->value().get_if<std::int64_t>();

                    std::string op = bin->op();
                    if (l_int && r_int) {
                        if (op == "+") return std::make_shared<LiteralNode>(Term(*l_int + *r_int));
                        if (op == "-") return std::make_shared<LiteralNode>(Term(*l_int - *r_int));
                        if (op == "*") return std::make_shared<LiteralNode>(Term(*l_int * *r_int));
                        if (op == "/") return std::make_shared<LiteralNode>(Term(*r_int != 0 ? *l_int / *r_int : 0LL));
                        if (op == "==") return std::make_shared<BoolExprNode>(*l_int == *r_int);
                        if (op == "!=") return std::make_shared<BoolExprNode>(*l_int != *r_int);
                        if (op == "<") return std::make_shared<BoolExprNode>(*l_int < *r_int);
                        if (op == ">") return std::make_shared<BoolExprNode>(*l_int > *r_int);
                        if (op == "<=") return std::make_shared<BoolExprNode>(*l_int <= *r_int);
                        if (op == ">=") return std::make_shared<BoolExprNode>(*l_int >= *r_int);
                    }

                    auto* l_str = l_lit->value().get_if<std::string>();
                    auto* r_str = r_lit->value().get_if<std::string>();
                    if (l_str && r_str && op == "++") {
                        return std::make_shared<LiteralNode>(Term(*l_str + *r_str));
                    }
                }
                return std::make_shared<BinaryExprNode>(bin->op(), left, right);
            }

            case AstNodeType::IfExpr: {
                auto* if_node = static_cast<IfExprNode*>(node.get());
                if (!if_node->clauses().empty()) {
                    const auto& clause = if_node->clauses()[0];
                    auto cond = fold_constants(clause.condition);

                    // Dead Code Elimination for boolean literal condition
                    if (cond->type() == AstNodeType::BoolExpr) {
                        auto* b_cond = static_cast<BoolExprNode*>(cond.get());
                        if (b_cond->value()) {
                            // Condition is ALWAYS true -> prune else branch!
                            if (!clause.body.empty()) {
                                return fold_constants(clause.body.back());
                            }
                        } else {
                            // Condition is ALWAYS false -> prune then branch!
                            if (!if_node->else_body().empty()) {
                                return fold_constants(if_node->else_body().back());
                            }
                        }
                    }
                }
                return node;
            }

            case AstNodeType::AssignmentExpr: {
                auto* assign = static_cast<AssignmentExprNode*>(node.get());
                return std::make_shared<AssignmentExprNode>(
                    assign->target_ptr(),
                    fold_constants(assign->value_ptr())
                );
            }

            case AstNodeType::TupleExpr: {
                auto* tup = static_cast<TupleExprNode*>(node.get());
                std::vector<std::shared_ptr<AstNode>> opt_items;
                for (const auto& item : tup->items()) {
                    opt_items.push_back(fold_constants(item));
                }
                return std::make_shared<TupleExprNode>(std::move(opt_items));
            }

            case AstNodeType::ListExpr: {
                auto* lst = static_cast<ListExprNode*>(node.get());
                std::vector<std::shared_ptr<AstNode>> opt_items;
                for (const auto& item : lst->items()) {
                    opt_items.push_back(fold_constants(item));
                }
                return std::make_shared<ListExprNode>(std::move(opt_items));
            }

            default:
                return node;
        }
    }
};

} // namespace garuda::compiler
