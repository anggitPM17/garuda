#pragma once
// Garuda Language Compiler: Static Code Style Analyzer (`linter.hpp`)
// Mengimplementasikan Deteksi Naming Convention (snake_case), Unused Variables, & Style Warnings.
#include "ast.hpp"

#include <cctype>
#include <iostream>
#include <memory>
#include <set>
#include <string>
#include <unordered_set>
#include <vector>

namespace garuda::compiler {

struct LintWarning {
    std::string message;
    std::size_t line = 1;
    std::size_t col = 1;
};

class Linter {
public:
    Linter() = default;

    std::vector<LintWarning> lint(std::shared_ptr<ModuleNode> module) {
        warnings_.clear();
        if (!module) return warnings_;

        for (const auto& fn : module->functions()) {
            lint_function(fn);
        }

        return warnings_;
    }

private:
    void lint_function(std::shared_ptr<FunctionDeclNode> fn) {
        // 1. Naming Convention Check: Functions must be snake_case
        std::string fn_name = fn->name();
        if (!is_snake_case(fn_name)) {
            warnings_.push_back({
                "Function name '" + fn_name + "' should be in snake_case",
                fn->line(), fn->col()
            });
        }

        std::unordered_set<std::string> bound_vars;
        std::unordered_set<std::string> used_vars;

        // 2. Naming Convention & Unused Vars Check
        for (const auto& param : fn->params()) {
            if (param != "_" && !is_snake_case(param)) {
                warnings_.push_back({
                    "Parameter name '" + param + "' in function '" + fn_name + "' should be in snake_case",
                    fn->line(), fn->col()
                });
            }
            bound_vars.insert(param);
        }

        for (const auto& stmt : fn->body()) {
            scan_expr(stmt, bound_vars, used_vars);
        }

        for (const auto& var : bound_vars) {
            if (var != "_" && used_vars.find(var) == used_vars.end()) {
                warnings_.push_back({
                    "Variable '" + var + "' in function '" + fn_name + "' is bound but never used",
                    fn->line(), fn->col()
                });
            }
        }
    }

    void scan_expr(
        std::shared_ptr<AstNode> node,
        std::unordered_set<std::string>& bound_vars,
        std::unordered_set<std::string>& used_vars
    ) {
        if (!node) return;

        switch (node->type()) {
            case AstNodeType::VariableExpr: {
                auto* var = static_cast<VariableNode*>(node.get());
                if (var->name() != "_") {
                    used_vars.insert(var->name());
                    if (!is_snake_case(var->name())) {
                        warnings_.push_back({
                            "Variable name '" + var->name() + "' should be in snake_case",
                            var->line(), var->col()
                        });
                    }
                }
                break;
            }

            case AstNodeType::AssignmentExpr: {
                auto* assign = static_cast<AssignmentExprNode*>(node.get());
                scan_expr(assign->value_ptr(), bound_vars, used_vars);
                bind_var(assign->target_ptr(), bound_vars);
                break;
            }

            case AstNodeType::BinaryExpr: {
                auto* bin = static_cast<BinaryExprNode*>(node.get());
                scan_expr(bin->left_ptr(), bound_vars, used_vars);
                scan_expr(bin->right_ptr(), bound_vars, used_vars);
                break;
            }

            case AstNodeType::CallExpr: {
                auto* call = static_cast<CallExprNode*>(node.get());
                for (const auto& arg : call->args()) {
                    scan_expr(arg, bound_vars, used_vars);
                }
                break;
            }

            default:
                break;
        }
    }

    void bind_var(std::shared_ptr<AstNode> target, std::unordered_set<std::string>& bound_vars) {
        if (!target) return;
        if (target->type() == AstNodeType::VariableExpr) {
            auto* var = static_cast<VariableNode*>(target.get());
            if (var->name() != "_") {
                bound_vars.insert(var->name());
                if (!is_snake_case(var->name())) {
                    warnings_.push_back({
                        "Variable name '" + var->name() + "' should be in snake_case",
                        var->line(), var->col()
                    });
                }
            }
        }
    }

    static bool is_snake_case(const std::string& name) {
        for (char c : name) {
            if (std::isupper(static_cast<unsigned char>(c))) return false;
        }
        return true;
    }

    std::vector<LintWarning> warnings_;
};

} // namespace garuda::compiler
