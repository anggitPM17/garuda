#pragma once
// Garuda Language Compiler: AST Pretty-Printer & Code Formatter (`formatter.hpp`)
// Mengimplementasikan Idempotent Code Formatter dengan 4-space Indentation & Operator Spacing.
#include "ast.hpp"

#include <algorithm>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace garuda::compiler {

class Formatter {
public:
    Formatter() = default;

    std::string format(std::shared_ptr<ModuleNode> module) {
        if (!module) return "";

        std::ostringstream ss;
        ss << "modul " << module->name() << "\n\n";

        for (std::size_t i = 0; i < module->functions().size(); ++i) {
            if (i > 0) ss << "\n";
            ss << format_function(module->functions()[i]);
        }

        return ss.str();
    }

private:
    std::string format_function(std::shared_ptr<FunctionDeclNode> fn) {
        std::ostringstream ss;
        ss << "fungsi " << fn->name() << "(";
        for (std::size_t i = 0; i < fn->params().size(); ++i) {
            if (i > 0) ss << ", ";
            ss << fn->params()[i];
        }
        ss << ") ->\n";

        for (const auto& stmt : fn->body()) {
            ss << "    " << format_expr(stmt) << "\n";
        }
        ss << "rampung\n";
        return ss.str();
    }

    std::string format_expr(std::shared_ptr<AstNode> node) {
        if (!node) return "";

        switch (node->type()) {
            case AstNodeType::LiteralExpr:
                return node->to_string();
            case AstNodeType::AtomExpr:
                return node->to_string();
            case AstNodeType::BoolExpr:
                return node->to_string();
            case AstNodeType::NilExpr:
                return "nil";
            case AstNodeType::VariableExpr:
                return node->to_string();

            case AstNodeType::AssignmentExpr: {
                auto* assign = static_cast<AssignmentExprNode*>(node.get());
                return format_expr(assign->target_ptr()) + " = " + format_expr(assign->value_ptr());
            }

            case AstNodeType::BinaryExpr: {
                auto* bin = static_cast<BinaryExprNode*>(node.get());
                return format_expr(bin->left_ptr()) + " " + bin->op() + " " + format_expr(bin->right_ptr());
            }

            case AstNodeType::UnaryExpr: {
                auto* un = static_cast<UnaryExprNode*>(node.get());
                return un->op() + format_expr(un->operand_ptr());
            }

            case AstNodeType::CallExpr: {
                auto* call = static_cast<CallExprNode*>(node.get());
                std::ostringstream ss;
                ss << call->name() << "(";
                for (std::size_t i = 0; i < call->args().size(); ++i) {
                    if (i > 0) ss << ", ";
                    ss << format_expr(call->args()[i]);
                }
                ss << ")";
                return ss.str();
            }

            case AstNodeType::TupleExpr: {
                auto* tup = static_cast<TupleExprNode*>(node.get());
                std::ostringstream ss;
                ss << "{";
                for (std::size_t i = 0; i < tup->items().size(); ++i) {
                    if (i > 0) ss << ", ";
                    ss << format_expr(tup->items()[i]);
                }
                ss << "}";
                return ss.str();
            }

            case AstNodeType::ListExpr: {
                auto* lst = static_cast<ListExprNode*>(node.get());
                std::ostringstream ss;
                ss << "[";
                for (std::size_t i = 0; i < lst->items().size(); ++i) {
                    if (i > 0) ss << ", ";
                    ss << format_expr(lst->items()[i]);
                }
                ss << "]";
                return ss.str();
            }

            case AstNodeType::CetakExpr: {
                auto* pr = static_cast<CetakExprNode*>(node.get());
                return "cetak(" + format_expr(pr->value_ptr()) + ")";
            }

            case AstNodeType::SendExpr: {
                auto* send_node = static_cast<SendExprNode*>(node.get());
                return format_expr(send_node->target_ptr()) + " <- " + format_expr(send_node->message_ptr());
            }

            default:
                return node->to_string();
        }
    }
};

} // namespace garuda::compiler
