#pragma once
// Garuda Language Compiler: Semantic Analysis & Gradual Type System Pass (`semantic.hpp`)
// Mengimplementasikan Symbol Table, Scope Tracking, Arity Checker, Type Environment, & Static Type Checker.
#include "ast.hpp"

#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace garuda::compiler {

struct SemanticError {
    std::string message;
    std::string function_name;
    std::size_t line = 0;
    std::size_t col = 0;
};

enum class TypeKind { Unknown, Int, Float, String, Atom, Bool, Tuple, List };

struct FunctionSignature {
    std::vector<TypeKind> param_types;
    TypeKind return_type = TypeKind::Unknown;
};

class SemanticAnalyzer {
public:
    SemanticAnalyzer() = default;

    static TypeKind parse_type_kind(const std::string& t_str) {
        if (t_str == "Int" || t_str == "integer") return TypeKind::Int;
        if (t_str == "Float" || t_str == "float") return TypeKind::Float;
        if (t_str == "String" || t_str == "string") return TypeKind::String;
        if (t_str == "Atom" || t_str == "atom") return TypeKind::Atom;
        if (t_str == "Bool" || t_str == "boolean") return TypeKind::Bool;
        if (t_str == "List" || t_str == "list") return TypeKind::List;
        if (t_str == "Tuple" || t_str == "tuple") return TypeKind::Tuple;
        return TypeKind::Unknown; // Any / Unknown
    }

    static std::string type_kind_to_string(TypeKind kind) {
        switch (kind) {
            case TypeKind::Int: return "Int";
            case TypeKind::Float: return "Float";
            case TypeKind::String: return "String";
            case TypeKind::Atom: return "Atom";
            case TypeKind::Bool: return "Bool";
            case TypeKind::Tuple: return "Tuple";
            case TypeKind::List: return "List";
            default: return "Any";
        }
    }

    std::vector<SemanticError> analyze(std::shared_ptr<ModuleNode> module) {
        errors_.clear();
        if (!module) return errors_;

        module_name_ = module->name();
        functions_table_.clear();
        function_sigs_.clear();

        // Pass 1: Build Global Function Symbol Table & Type Signatures
        for (const auto& fn : module->functions()) {
            std::size_t arity = fn->params().size();
            functions_table_[fn->name()] = arity;

            FunctionSignature sig;
            for (const auto& pt : fn->param_types()) {
                sig.param_types.push_back(parse_type_kind(pt));
            }
            sig.return_type = parse_type_kind(fn->return_type());
            function_sigs_[fn->name()] = sig;
        }

        // Pass 2: Analyze Function Bodies, Parameters, Scope, Arity, and Types
        for (const auto& fn : module->functions()) {
            analyze_function(fn);
        }

        return errors_;
    }

private:
    void analyze_function(std::shared_ptr<FunctionDeclNode> fn) {
        current_fn_name_ = fn->name();
        scoped_vars_.clear();
        var_types_.clear();

        // 1. Duplicate Parameter Check & Type Binding
        std::unordered_set<std::string> param_set;
        for (std::size_t i = 0; i < fn->params().size(); ++i) {
            const std::string& param = fn->params()[i];
            if (!param_set.insert(param).second) {
                errors_.push_back({
                    "Duplicate parameter name '" + param + "' in function '" + fn->name() + "'",
                    fn->name(), fn->line(), fn->col()
                });
            } else {
                scoped_vars_.insert(param);
                if (i < fn->param_types().size()) {
                    var_types_[param] = parse_type_kind(fn->param_types()[i]);
                }
            }
        }

        // 2. Analyze Function Statements & Return Type
        TypeKind actual_return_type = TypeKind::Unknown;
        for (const auto& stmt : fn->body()) {
            actual_return_type = analyze_expr(stmt);
        }

        // 3. Return Type Annotation Enforcement
        TypeKind expected_return_type = parse_type_kind(fn->return_type());
        if (expected_return_type != TypeKind::Unknown && actual_return_type != TypeKind::Unknown) {
            if (expected_return_type != actual_return_type) {
                errors_.push_back({
                    "Type mismatch: Function '" + fn->name() + "' expected return type '" +
                    type_kind_to_string(expected_return_type) + "', but got '" +
                    type_kind_to_string(actual_return_type) + "'",
                    fn->name(), fn->line(), fn->col()
                });
            }
        }
    }

    TypeKind analyze_expr(std::shared_ptr<AstNode> node) {
        if (!node) return TypeKind::Unknown;

        switch (node->type()) {
            case AstNodeType::LiteralExpr: {
                auto* lit = static_cast<LiteralNode*>(node.get());
                if (lit->value().get_if<std::int64_t>()) return TypeKind::Int;
                if (lit->value().get_if<double>()) return TypeKind::Float;
                if (lit->value().get_if<std::string>()) return TypeKind::String;
                if (lit->value().get_if<Atom>()) return TypeKind::Atom;
                return TypeKind::Unknown;
            }
            case AstNodeType::AtomExpr: return TypeKind::Atom;
            case AstNodeType::BoolExpr: return TypeKind::Bool;
            case AstNodeType::NilExpr: return TypeKind::Atom;

            case AstNodeType::VariableExpr: {
                auto* var = static_cast<VariableNode*>(node.get());
                // Anonymous wildcard '_' is always bound
                if (var->name() != "_" && scoped_vars_.find(var->name()) == scoped_vars_.end()) {
                    errors_.push_back({
                        "Unbound variable '" + var->name() + "' in function '" + current_fn_name_ + "'",
                        current_fn_name_, var->line(), var->col()
                    });
                }
                auto vt_it = var_types_.find(var->name());
                if (vt_it != var_types_.end()) return vt_it->second;

                if (var->type_annotation() != "Any") {
                    return parse_type_kind(var->type_annotation());
                }
                return TypeKind::Unknown;
            }

            case AstNodeType::AssignmentExpr: {
                auto* assign = static_cast<AssignmentExprNode*>(node.get());
                TypeKind val_type = analyze_expr(assign->value_ptr());
                bind_pattern_vars(assign->target_ptr(), val_type);
                return val_type;
            }

            case AstNodeType::BinaryExpr: {
                auto* bin = static_cast<BinaryExprNode*>(node.get());
                TypeKind left_type = analyze_expr(bin->left_ptr());
                TypeKind right_type = analyze_expr(bin->right_ptr());

                std::string op = bin->op();
                if (op == "+" || op == "-" || op == "*" || op == "/") {
                    if (left_type == TypeKind::String || right_type == TypeKind::String) {
                        errors_.push_back({
                            "Type mismatch: Cannot perform arithmetic '" + op + "' on String in function '" + current_fn_name_ + "'",
                            current_fn_name_, bin->line(), bin->col()
                        });
                    }
                    return (left_type == TypeKind::Float || right_type == TypeKind::Float) ? TypeKind::Float : TypeKind::Int;
                } else if (op == "++") {
                    if (left_type == TypeKind::Int || right_type == TypeKind::Int) {
                        errors_.push_back({
                            "Type mismatch: Cannot perform concatenation '++' on Int in function '" + current_fn_name_ + "'",
                            current_fn_name_, bin->line(), bin->col()
                        });
                    }
                    return TypeKind::String;
                }
                return TypeKind::Bool;
            }

            case AstNodeType::CallExpr: {
                auto* call = static_cast<CallExprNode*>(node.get());
                std::size_t arg_count = call->args().size();

                std::vector<TypeKind> arg_types;
                for (const auto& arg : call->args()) {
                    arg_types.push_back(analyze_expr(arg));
                }

                auto it = functions_table_.find(call->name());
                if (it != functions_table_.end()) {
                    if (it->second != arg_count) {
                        errors_.push_back({
                            "Arity mismatch: Function '" + call->name() + "' expects " +
                            std::to_string(it->second) + " argument(s), but got " + std::to_string(arg_count) +
                            " in function '" + current_fn_name_ + "'",
                            current_fn_name_, call->line(), call->col()
                        });
                    }

                    // Static Parameter Type Signature Enforcement
                    auto sig_it = function_sigs_.find(call->name());
                    if (sig_it != function_sigs_.end()) {
                        const auto& sig = sig_it->second;
                        for (std::size_t i = 0; i < sig.param_types.size() && i < arg_types.size(); ++i) {
                            if (sig.param_types[i] != TypeKind::Unknown && arg_types[i] != TypeKind::Unknown) {
                                if (sig.param_types[i] != arg_types[i]) {
                                    errors_.push_back({
                                        "Type mismatch: Argument " + std::to_string(i + 1) + " of '" + call->name() +
                                        "' expected '" + type_kind_to_string(sig.param_types[i]) + "', but got '" +
                                        type_kind_to_string(arg_types[i]) + "' in function '" + current_fn_name_ + "'",
                                        current_fn_name_, call->line(), call->col()
                                    });
                                }
                            }
                        }
                        return sig.return_type;
                    }
                }
                return TypeKind::Unknown;
            }

            case AstNodeType::SendExpr: {
                auto* send_node = static_cast<SendExprNode*>(node.get());
                analyze_expr(send_node->target_ptr());
                analyze_expr(send_node->message_ptr());
                return TypeKind::Unknown;
            }

            case AstNodeType::SpawnExpr: {
                auto* spawn_node = static_cast<SpawnExprNode*>(node.get());
                analyze_expr(spawn_node->fun_ptr());
                return TypeKind::Unknown;
            }

            case AstNodeType::ReceiveExpr: {
                auto* rec = static_cast<ReceiveExprNode*>(node.get());
                for (const auto& clause : rec->clauses()) {
                    bind_pattern_vars(clause.pattern, TypeKind::Unknown);
                    for (const auto& stmt : clause.body) {
                        analyze_expr(stmt);
                    }
                }
                return TypeKind::Unknown;
            }

            case AstNodeType::IfExpr: {
                auto* if_node = static_cast<IfExprNode*>(node.get());
                for (const auto& clause : if_node->clauses()) {
                    analyze_expr(clause.condition);
                    for (const auto& stmt : clause.body) {
                        analyze_expr(stmt);
                    }
                }
                for (const auto& stmt : if_node->else_body()) {
                    analyze_expr(stmt);
                }
                return TypeKind::Unknown;
            }

            case AstNodeType::CetakExpr: {
                auto* pr = static_cast<CetakExprNode*>(node.get());
                analyze_expr(pr->value_ptr());
                return TypeKind::Unknown;
            }

            case AstNodeType::TupleExpr: {
                auto* tup = static_cast<TupleExprNode*>(node.get());
                for (const auto& item : tup->items()) {
                    analyze_expr(item);
                }
                return TypeKind::Tuple;
            }

            case AstNodeType::ListExpr: {
                auto* lst = static_cast<ListExprNode*>(node.get());
                for (const auto& item : lst->items()) {
                    analyze_expr(item);
                }
                return TypeKind::List;
            }

            default:
                return TypeKind::Unknown;
        }
    }

    void bind_pattern_vars(std::shared_ptr<AstNode> pattern, TypeKind val_type) {
        if (!pattern) return;
        if (pattern->type() == AstNodeType::PatternVar) {
            auto* pvar = static_cast<PatternVarNode*>(pattern.get());
            if (pvar->name() != "_") {
                scoped_vars_.insert(pvar->name());
                if (val_type != TypeKind::Unknown) var_types_[pvar->name()] = val_type;
            }
        } else if (pattern->type() == AstNodeType::VariableExpr) {
            auto* var = static_cast<VariableNode*>(pattern.get());
            if (var->name() != "_") {
                scoped_vars_.insert(var->name());

                if (var->type_annotation() != "Any") {
                    TypeKind annotated_type = parse_type_kind(var->type_annotation());
                    if (val_type != TypeKind::Unknown && annotated_type != val_type) {
                        errors_.push_back({
                            "Type mismatch: Variable '" + var->name() + "' annotated as '" +
                            type_kind_to_string(annotated_type) + "', but assigned value of type '" +
                            type_kind_to_string(val_type) + "'",
                            current_fn_name_, var->line(), var->col()
                        });
                    }
                    var_types_[var->name()] = annotated_type;
                } else if (val_type != TypeKind::Unknown) {
                    var_types_[var->name()] = val_type;
                }
            }
        } else if (pattern->type() == AstNodeType::PatternTuple) {
            auto* ptup = static_cast<PatternTupleNode*>(pattern.get());
            for (const auto& item : ptup->items()) {
                bind_pattern_vars(item, TypeKind::Unknown);
            }
        } else if (pattern->type() == AstNodeType::TupleExpr) {
            auto* tup = static_cast<TupleExprNode*>(pattern.get());
            for (const auto& item : tup->items()) {
                bind_pattern_vars(item, TypeKind::Unknown);
            }
        } else if (pattern->type() == AstNodeType::PatternListHeadTail) {
            auto* pht = static_cast<PatternListHeadTailNode*>(pattern.get());
            bind_pattern_vars(pht->head_ptr(), TypeKind::Unknown);
            bind_pattern_vars(pht->tail_ptr(), TypeKind::List);
        } else if (pattern->type() == AstNodeType::PatternGuard) {
            auto* pg = static_cast<PatternGuardNode*>(pattern.get());
            bind_pattern_vars(pg->pattern_ptr(), val_type);
            analyze_expr(pg->guard_ptr());
        }
    }

    std::string module_name_;
    std::string current_fn_name_;
    std::unordered_map<std::string, std::size_t> functions_table_;
    std::unordered_map<std::string, FunctionSignature> function_sigs_;
    std::unordered_map<std::string, TypeKind> var_types_;
    std::unordered_set<std::string> scoped_vars_;
    std::vector<SemanticError> errors_;
};

} // namespace garuda::compiler
