#pragma once
// Garuda Language Compiler: Macro Expander Pass (`macro_expander.hpp`)
// Mengimplementasikan Metaprogramming & Compile-Time AST Transformation.
#include "ast.hpp"

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace garuda::compiler {

class MacroExpander {
public:
    MacroExpander() = default;

    std::shared_ptr<ModuleNode> expand(std::shared_ptr<ModuleNode> module) {
        if (!module) return module;

        macro_table_.clear();
        auto expanded_module = std::make_shared<ModuleNode>(module->name());

        // Pass 1: Collect macro definitions
        for (const auto& fn : module->functions()) {
            if (fn->type() == AstNodeType::MacroDef) {
                auto macro_def = std::static_pointer_cast<MacroDefNode>(fn);
                macro_table_[macro_def->name()] = macro_def;
            }
        }

        // Pass 2: Expand macros in function bodies
        for (const auto& fn : module->functions()) {
            if (fn->type() == AstNodeType::FunctionDecl) {
                std::vector<std::shared_ptr<AstNode>> expanded_body;
                for (const auto& stmt : fn->body()) {
                    expanded_body.push_back(expand_expr(stmt, {}));
                }
                auto expanded_fn = std::make_shared<FunctionDeclNode>(
                    fn->name(), fn->params(), std::move(expanded_body)
                );
                expanded_module->add_function(expanded_fn);
            }
        }

        return expanded_module;
    }

private:
    std::shared_ptr<AstNode> expand_expr(
        std::shared_ptr<AstNode> node,
        const std::unordered_map<std::string, std::shared_ptr<AstNode>>& bindings
    ) {
        if (!node) return node;

        switch (node->type()) {
            case AstNodeType::VariableExpr: {
                auto* var = static_cast<VariableNode*>(node.get());
                auto it = bindings.find(var->name());
                if (it != bindings.end()) {
                    return clone_ast(it->second);
                }
                return node;
            }

            case AstNodeType::PatternVar: {
                auto* pvar = static_cast<PatternVarNode*>(node.get());
                auto it = bindings.find(pvar->name());
                if (it != bindings.end()) {
                    return clone_ast(it->second);
                }
                return node;
            }

            case AstNodeType::MacroCall: {
                auto* call = static_cast<MacroCallNode*>(node.get());
                auto it = macro_table_.find(call->name());
                if (it != macro_table_.end()) {
                    return expand_macro_definition(it->second, call->args(), bindings);
                }
                return node;
            }

            case AstNodeType::CallExpr: {
                auto* call = static_cast<CallExprNode*>(node.get());
                auto it = macro_table_.find(call->name());
                if (it != macro_table_.end()) {
                    return expand_macro_definition(it->second, call->args(), bindings);
                }

                std::vector<std::shared_ptr<AstNode>> expanded_args;
                for (const auto& arg : call->args()) {
                    expanded_args.push_back(expand_expr(arg, bindings));
                }
                return std::make_shared<CallExprNode>(call->name(), std::move(expanded_args));
            }

            case AstNodeType::BinaryExpr: {
                auto* bin = static_cast<BinaryExprNode*>(node.get());
                return std::make_shared<BinaryExprNode>(
                    bin->op(),
                    expand_expr(bin->left_ptr(), bindings),
                    expand_expr(bin->right_ptr(), bindings)
                );
            }

            case AstNodeType::UnaryExpr: {
                auto* un = static_cast<UnaryExprNode*>(node.get());
                return std::make_shared<UnaryExprNode>(
                    un->op(),
                    expand_expr(un->operand_ptr(), bindings)
                );
            }

            case AstNodeType::AssignmentExpr: {
                auto* assign = static_cast<AssignmentExprNode*>(node.get());
                return std::make_shared<AssignmentExprNode>(
                    expand_expr(assign->target_ptr(), bindings),
                    expand_expr(assign->value_ptr(), bindings)
                );
            }

            case AstNodeType::TupleExpr: {
                auto* tup = static_cast<TupleExprNode*>(node.get());
                std::vector<std::shared_ptr<AstNode>> expanded_items;
                for (const auto& item : tup->items()) {
                    expanded_items.push_back(expand_expr(item, bindings));
                }
                return std::make_shared<TupleExprNode>(std::move(expanded_items));
            }

            case AstNodeType::ListExpr: {
                auto* lst = static_cast<ListExprNode*>(node.get());
                std::vector<std::shared_ptr<AstNode>> expanded_items;
                for (const auto& item : lst->items()) {
                    expanded_items.push_back(expand_expr(item, bindings));
                }
                return std::make_shared<ListExprNode>(std::move(expanded_items));
            }

            case AstNodeType::IfExpr: {
                auto* if_node = static_cast<IfExprNode*>(node.get());
                std::vector<IfClause> expanded_clauses;
                for (const auto& clause : if_node->clauses()) {
                    std::vector<std::shared_ptr<AstNode>> body;
                    for (const auto& stmt : clause.body) {
                        body.push_back(expand_expr(stmt, bindings));
                    }
                    expanded_clauses.push_back({expand_expr(clause.condition, bindings), std::move(body)});
                }
                std::vector<std::shared_ptr<AstNode>> expanded_else;
                for (const auto& stmt : if_node->else_body()) {
                    expanded_else.push_back(expand_expr(stmt, bindings));
                }
                return std::make_shared<IfExprNode>(std::move(expanded_clauses), std::move(expanded_else));
            }

            default:
                return node;
        }
    }

    std::shared_ptr<AstNode> expand_macro_definition(
        std::shared_ptr<MacroDefNode> macro_def,
        const std::vector<std::shared_ptr<AstNode>>& args,
        const std::unordered_map<std::string, std::shared_ptr<AstNode>>& outer_bindings
    ) {
        std::unordered_map<std::string, std::shared_ptr<AstNode>> local_bindings = outer_bindings;
        const auto& params = macro_def->params();

        for (std::size_t i = 0; i < params.size() && i < args.size(); ++i) {
            local_bindings[params[i]] = expand_expr(args[i], outer_bindings);
        }

        std::shared_ptr<AstNode> last_expanded = nullptr;
        for (const auto& stmt : macro_def->body()) {
            last_expanded = expand_expr(stmt, local_bindings);
        }

        return last_expanded;
    }

    std::shared_ptr<AstNode> clone_ast(std::shared_ptr<AstNode> node) {
        if (!node) return nullptr;
        return expand_expr(node, {});
    }

    std::unordered_map<std::string, std::shared_ptr<MacroDefNode>> macro_table_;
};

} // namespace garuda::compiler
