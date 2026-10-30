#pragma once
// Garuda Language Compiler: Abstract Syntax Tree (AST) Node Hierarchy (`ast.hpp`)
// Pohon sintaksis abstrak lengkap dengan Operator Precedence, Symbol Table, dan Tail Call Optimization (TCO).
#include "../beam/core/term.hpp"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace garuda::compiler {

using namespace garuda::beam;

enum class AstNodeType {
    ModuleDecl, FunctionDecl, BinaryExpr, LiteralExpr, VariableExpr,
    UnaryExpr, CallExpr, TupleExpr, ListExpr, AtomExpr, BoolExpr, NilExpr,
    SelfExpr, CetakExpr, SpawnExpr, AnonFunExpr, SendExpr, ReceiveExpr,
    IfExpr, CaseExpr, AssignmentExpr,
    PatternWildcard, PatternVar, PatternLiteral, PatternTuple, PatternListHeadTail, PatternGuard,
    MacroDef, MacroCall,
    TryCatchExpr, TryOperatorExpr, ImportDecl
};

class AstNode {
public:
    virtual ~AstNode() = default;
    virtual AstNodeType type() const = 0;
    virtual std::string to_string() const = 0;

    std::size_t line() const { return line_; }
    std::size_t col() const { return col_; }
    void set_loc(std::size_t line, std::size_t col) { line_ = line; col_ = col; }

protected:
    std::size_t line_ = 1;
    std::size_t col_ = 1;
};

class LiteralNode : public AstNode {
public:
    explicit LiteralNode(Term val) : val_(std::move(val)) {}
    AstNodeType type() const override { return AstNodeType::LiteralExpr; }
    const Term& value() const { return val_; }
    std::string to_string() const override { return garuda::beam::to_string(val_); }
private:
    Term val_;
};

class VariableNode : public AstNode {
public:
    explicit VariableNode(std::string name, std::string type_annotation = "Any")
        : name_(std::move(name)), type_annotation_(std::move(type_annotation)) {}
    AstNodeType type() const override { return AstNodeType::VariableExpr; }
    const std::string& name() const { return name_; }
    const std::string& type_annotation() const { return type_annotation_; }
    std::string to_string() const override { return name_; }
private:
    std::string name_;
    std::string type_annotation_ = "Any";
};

class BinaryExprNode : public AstNode {
public:
    BinaryExprNode(std::string op, std::shared_ptr<AstNode> left, std::shared_ptr<AstNode> right)
        : op_(std::move(op)), left_(std::move(left)), right_(std::move(right)) {}
    AstNodeType type() const override { return AstNodeType::BinaryExpr; }
    const std::string& op() const { return op_; }
    const AstNode& left() const { return *left_; }
    const AstNode& right() const { return *right_; }
    std::shared_ptr<AstNode> left_ptr() const { return left_; }
    std::shared_ptr<AstNode> right_ptr() const { return right_; }
    std::string to_string() const override {
        return "(" + left_->to_string() + " " + op_ + " " + right_->to_string() + ")";
    }
private:
    std::string op_;
    std::shared_ptr<AstNode> left_;
    std::shared_ptr<AstNode> right_;
};

class UnaryExprNode : public AstNode {
public:
    UnaryExprNode(std::string op, std::shared_ptr<AstNode> operand)
        : op_(std::move(op)), operand_(std::move(operand)) {}
    AstNodeType type() const override { return AstNodeType::UnaryExpr; }
    const std::string& op() const { return op_; }
    const AstNode& operand() const { return *operand_; }
    std::shared_ptr<AstNode> operand_ptr() const { return operand_; }
    std::string to_string() const override {
        return "(" + op_ + operand_->to_string() + ")";
    }
private:
    std::string op_;
    std::shared_ptr<AstNode> operand_;
};

class CallExprNode : public AstNode {
public:
    CallExprNode(std::string name, std::vector<std::shared_ptr<AstNode>> args)
        : name_(std::move(name)), args_(std::move(args)) {}
    AstNodeType type() const override { return AstNodeType::CallExpr; }
    const std::string& name() const { return name_; }
    const std::vector<std::shared_ptr<AstNode>>& args() const { return args_; }
    std::string to_string() const override {
        std::string res = name_ + "(";
        for (std::size_t i = 0; i < args_.size(); ++i) {
            if (i > 0) res += ", ";
            res += args_[i]->to_string();
        }
        return res + ")";
    }
private:
    std::string name_;
    std::vector<std::shared_ptr<AstNode>> args_;
};

class TupleExprNode : public AstNode {
public:
    explicit TupleExprNode(std::vector<std::shared_ptr<AstNode>> items)
        : items_(std::move(items)) {}
    AstNodeType type() const override { return AstNodeType::TupleExpr; }
    const std::vector<std::shared_ptr<AstNode>>& items() const { return items_; }
    std::string to_string() const override {
        std::string res = "{";
        for (std::size_t i = 0; i < items_.size(); ++i) {
            if (i > 0) res += ", ";
            res += items_[i]->to_string();
        }
        return res + "}";
    }
private:
    std::vector<std::shared_ptr<AstNode>> items_;
};

class ListExprNode : public AstNode {
public:
    explicit ListExprNode(std::vector<std::shared_ptr<AstNode>> items)
        : items_(std::move(items)) {}
    AstNodeType type() const override { return AstNodeType::ListExpr; }
    const std::vector<std::shared_ptr<AstNode>>& items() const { return items_; }
    std::string to_string() const override {
        std::string res = "[";
        for (std::size_t i = 0; i < items_.size(); ++i) {
            if (i > 0) res += ", ";
            res += items_[i]->to_string();
        }
        return res + "]";
    }
private:
    std::vector<std::shared_ptr<AstNode>> items_;
};

class AtomExprNode : public AstNode {
public:
    explicit AtomExprNode(std::string name) : name_(std::move(name)) {}
    AstNodeType type() const override { return AstNodeType::AtomExpr; }
    const std::string& name() const { return name_; }
    std::string to_string() const override { return ":" + name_; }
private:
    std::string name_;
};

class BoolExprNode : public AstNode {
public:
    explicit BoolExprNode(bool val) : val_(val) {}
    AstNodeType type() const override { return AstNodeType::BoolExpr; }
    bool value() const { return val_; }
    std::string to_string() const override { return val_ ? "benar" : "salah"; }
private:
    bool val_;
};

class NilExprNode : public AstNode {
public:
    AstNodeType type() const override { return AstNodeType::NilExpr; }
    std::string to_string() const override { return "nil"; }
};

class SelfExprNode : public AstNode {
public:
    AstNodeType type() const override { return AstNodeType::SelfExpr; }
    std::string to_string() const override { return "diri"; }
};

class CetakExprNode : public AstNode {
public:
    explicit CetakExprNode(std::shared_ptr<AstNode> val) : val_(std::move(val)) {}
    AstNodeType type() const override { return AstNodeType::CetakExpr; }
    const AstNode& value() const { return *val_; }
    std::shared_ptr<AstNode> value_ptr() const { return val_; }
    std::string to_string() const override { return "cetak(" + val_->to_string() + ")"; }
private:
    std::shared_ptr<AstNode> val_;
};

class SpawnExprNode : public AstNode {
public:
    explicit SpawnExprNode(std::shared_ptr<AstNode> fun) : fun_(std::move(fun)) {}
    AstNodeType type() const override { return AstNodeType::SpawnExpr; }
    const AstNode& fun() const { return *fun_; }
    std::shared_ptr<AstNode> fun_ptr() const { return fun_; }
    std::string to_string() const override { return "cakra(" + fun_->to_string() + ")"; }
private:
    std::shared_ptr<AstNode> fun_;
};

class AnonFunExprNode : public AstNode {
public:
    AnonFunExprNode(std::vector<std::string> params, std::vector<std::shared_ptr<AstNode>> body)
        : params_(std::move(params)), body_(std::move(body)) {}
    AstNodeType type() const override { return AstNodeType::AnonFunExpr; }
    const std::vector<std::string>& params() const { return params_; }
    const std::vector<std::shared_ptr<AstNode>>& body() const { return body_; }
    std::string to_string() const override {
        std::string res = "fungsi(";
        for (std::size_t i = 0; i < params_.size(); ++i) {
            if (i > 0) res += ", ";
            res += params_[i];
        }
        res += ") -> ... rampung";
        return res;
    }
private:
    std::vector<std::string> params_;
    std::vector<std::shared_ptr<AstNode>> body_;
};

class SendExprNode : public AstNode {
public:
    SendExprNode(std::shared_ptr<AstNode> target, std::shared_ptr<AstNode> message)
        : target_(std::move(target)), message_(std::move(message)) {}
    AstNodeType type() const override { return AstNodeType::SendExpr; }
    const AstNode& target() const { return *target_; }
    const AstNode& message() const { return *message_; }
    std::shared_ptr<AstNode> target_ptr() const { return target_; }
    std::shared_ptr<AstNode> message_ptr() const { return message_; }
    std::string to_string() const override {
        return target_->to_string() + " <- " + message_->to_string();
    }
private:
    std::shared_ptr<AstNode> target_;
    std::shared_ptr<AstNode> message_;
};

struct ReceiveClause {
    std::shared_ptr<AstNode> pattern;
    std::shared_ptr<AstNode> guard;
    std::vector<std::shared_ptr<AstNode>> body;
};

class ReceiveExprNode : public AstNode {
public:
    explicit ReceiveExprNode(std::vector<ReceiveClause> clauses)
        : clauses_(std::move(clauses)) {}
    AstNodeType type() const override { return AstNodeType::ReceiveExpr; }
    const std::vector<ReceiveClause>& clauses() const { return clauses_; }
    std::string to_string() const override { return "tampa ... rampung"; }
private:
    std::vector<ReceiveClause> clauses_;
};

struct IfClause {
    std::shared_ptr<AstNode> condition;
    std::vector<std::shared_ptr<AstNode>> body;
};

class IfExprNode : public AstNode {
public:
    IfExprNode(std::vector<IfClause> clauses, std::vector<std::shared_ptr<AstNode>> else_body)
        : clauses_(std::move(clauses)), else_body_(std::move(else_body)) {}
    AstNodeType type() const override { return AstNodeType::IfExpr; }
    const std::vector<IfClause>& clauses() const { return clauses_; }
    const std::vector<std::shared_ptr<AstNode>>& else_body() const { return else_body_; }
    std::string to_string() const override { return "yen ... lain ... rampung"; }
private:
    std::vector<IfClause> clauses_;
    std::vector<std::shared_ptr<AstNode>> else_body_;
};

struct CaseClause {
    std::shared_ptr<AstNode> pattern;
    std::shared_ptr<AstNode> guard;
    std::vector<std::shared_ptr<AstNode>> body;
};

class CaseExprNode : public AstNode {
public:
    CaseExprNode(std::shared_ptr<AstNode> subject, std::vector<CaseClause> clauses)
        : subject_(std::move(subject)), clauses_(std::move(clauses)) {}
    AstNodeType type() const override { return AstNodeType::CaseExpr; }
    const AstNode& subject() const { return *subject_; }
    std::shared_ptr<AstNode> subject_ptr() const { return subject_; }
    const std::vector<CaseClause>& clauses() const { return clauses_; }
    std::string to_string() const override { return "tinjau ... ing ... rampung"; }
private:
    std::shared_ptr<AstNode> subject_;
    std::vector<CaseClause> clauses_;
};

class AssignmentExprNode : public AstNode {
public:
    AssignmentExprNode(std::shared_ptr<AstNode> target, std::shared_ptr<AstNode> value)
        : target_(std::move(target)), value_(std::move(value)) {}
    AstNodeType type() const override { return AstNodeType::AssignmentExpr; }
    const AstNode& target() const { return *target_; }
    const AstNode& value() const { return *value_; }
    std::shared_ptr<AstNode> target_ptr() const { return target_; }
    std::shared_ptr<AstNode> value_ptr() const { return value_; }
    std::string to_string() const override {
        return target_->to_string() + " = " + value_->to_string();
    }
private:
    std::shared_ptr<AstNode> target_;
    std::shared_ptr<AstNode> value_;
};

// Pattern Nodes
class PatternWildcardNode : public AstNode {
public:
    AstNodeType type() const override { return AstNodeType::PatternWildcard; }
    std::string to_string() const override { return "_"; }
};

class PatternVarNode : public AstNode {
public:
    explicit PatternVarNode(std::string name) : name_(std::move(name)) {}
    AstNodeType type() const override { return AstNodeType::PatternVar; }
    const std::string& name() const { return name_; }
    std::string to_string() const override { return name_; }
private:
    std::string name_;
};

class PatternLiteralNode : public AstNode {
public:
    explicit PatternLiteralNode(Term val) : val_(std::move(val)) {}
    AstNodeType type() const override { return AstNodeType::PatternLiteral; }
    const Term& value() const { return val_; }
    std::string to_string() const override { return garuda::beam::to_string(val_); }
private:
    Term val_;
};

class PatternTupleNode : public AstNode {
public:
    explicit PatternTupleNode(std::vector<std::shared_ptr<AstNode>> items)
        : items_(std::move(items)) {}
    AstNodeType type() const override { return AstNodeType::PatternTuple; }
    const std::vector<std::shared_ptr<AstNode>>& items() const { return items_; }
    std::string to_string() const override {
        std::string res = "{";
        for (std::size_t i = 0; i < items_.size(); ++i) {
            if (i > 0) res += ", ";
            res += items_[i]->to_string();
        }
        return res + "}";
    }
private:
    std::vector<std::shared_ptr<AstNode>> items_;
};

class PatternListHeadTailNode : public AstNode {
public:
    PatternListHeadTailNode(std::shared_ptr<AstNode> head, std::shared_ptr<AstNode> tail)
        : head_(std::move(head)), tail_(std::move(tail)) {}
    AstNodeType type() const override { return AstNodeType::PatternListHeadTail; }
    const AstNode& head() const { return *head_; }
    const AstNode& tail() const { return *tail_; }
    std::shared_ptr<AstNode> head_ptr() const { return head_; }
    std::shared_ptr<AstNode> tail_ptr() const { return tail_; }
    std::string to_string() const override {
        return "[" + head_->to_string() + " | " + tail_->to_string() + "]";
    }
private:
    std::shared_ptr<AstNode> head_;
    std::shared_ptr<AstNode> tail_;
};

class PatternGuardNode : public AstNode {
public:
    PatternGuardNode(std::shared_ptr<AstNode> pattern, std::shared_ptr<AstNode> guard)
        : pattern_(std::move(pattern)), guard_(std::move(guard)) {}
    AstNodeType type() const override { return AstNodeType::PatternGuard; }
    const AstNode& pattern() const { return *pattern_; }
    const AstNode& guard() const { return *guard_; }
    std::shared_ptr<AstNode> pattern_ptr() const { return pattern_; }
    std::shared_ptr<AstNode> guard_ptr() const { return guard_; }
    std::string to_string() const override {
        return pattern_->to_string() + " ketika " + guard_->to_string();
    }
private:
    std::shared_ptr<AstNode> pattern_;
    std::shared_ptr<AstNode> guard_;
};

class FunctionDeclNode : public AstNode {
public:
    FunctionDeclNode(
        std::string name,
        std::vector<std::string> params,
        std::vector<std::shared_ptr<AstNode>> body,
        std::vector<std::string> param_types = {},
        std::string return_type = "Any"
    ) : name_(std::move(name)),
        params_(std::move(params)),
        body_(std::move(body)),
        param_types_(std::move(param_types)),
        return_type_(std::move(return_type)) {}

    AstNodeType type() const override { return AstNodeType::FunctionDecl; }
    const std::string& name() const { return name_; }
    const std::vector<std::string>& params() const { return params_; }
    const std::vector<std::shared_ptr<AstNode>>& body() const { return body_; }
    const std::vector<std::string>& param_types() const { return param_types_; }
    const std::string& return_type() const { return return_type_; }

    std::string to_string() const override {
        return "def " + name_ + "(" + std::to_string(params_.size()) + " args) : " + return_type_;
    }
private:
    std::string name_;
    std::vector<std::string> params_;
    std::vector<std::shared_ptr<AstNode>> body_;
    std::vector<std::string> param_types_;
    std::string return_type_ = "Any";
};

class MacroDefNode : public FunctionDeclNode {
public:
    MacroDefNode(std::string name, std::vector<std::string> params, std::vector<std::shared_ptr<AstNode>> body)
        : FunctionDeclNode(std::move(name), std::move(params), std::move(body)) {}
    AstNodeType type() const override { return AstNodeType::MacroDef; }
    std::string to_string() const override {
        return "makro " + name() + "(" + std::to_string(params().size()) + " params)";
    }
};

class MacroCallNode : public AstNode {
public:
    MacroCallNode(std::string name, std::vector<std::shared_ptr<AstNode>> args)
        : name_(std::move(name)), args_(std::move(args)) {}
    AstNodeType type() const override { return AstNodeType::MacroCall; }
    const std::string& name() const { return name_; }
    const std::vector<std::shared_ptr<AstNode>>& args() const { return args_; }
    std::string to_string() const override {
        return "macro_call " + name_ + "()";
    }
private:
    std::string name_;
    std::vector<std::shared_ptr<AstNode>> args_;
};

class TryCatchNode : public AstNode {
public:
    TryCatchNode(
        std::vector<std::shared_ptr<AstNode>> try_body,
        std::shared_ptr<AstNode> catch_pattern,
        std::vector<std::shared_ptr<AstNode>> catch_body
    ) : try_body_(std::move(try_body)),
        catch_pattern_(std::move(catch_pattern)),
        catch_body_(std::move(catch_body)) {}

    AstNodeType type() const override { return AstNodeType::TryCatchExpr; }
    const std::vector<std::shared_ptr<AstNode>>& try_body() const { return try_body_; }
    const AstNode& catch_pattern() const { return *catch_pattern_; }
    std::shared_ptr<AstNode> catch_pattern_ptr() const { return catch_pattern_; }
    const std::vector<std::shared_ptr<AstNode>>& catch_body() const { return catch_body_; }

    std::string to_string() const override {
        return "coba ... tangkap " + catch_pattern_->to_string();
    }
private:
    std::vector<std::shared_ptr<AstNode>> try_body_;
    std::shared_ptr<AstNode> catch_pattern_;
    std::vector<std::shared_ptr<AstNode>> catch_body_;
};

class TryOperatorNode : public AstNode {
public:
    explicit TryOperatorNode(std::shared_ptr<AstNode> expr) : expr_(std::move(expr)) {}
    AstNodeType type() const override { return AstNodeType::TryOperatorExpr; }
    const AstNode& expr() const { return *expr_; }
    std::shared_ptr<AstNode> expr_ptr() const { return expr_; }
    std::string to_string() const override {
        return expr_->to_string() + "?";
    }
private:
    std::shared_ptr<AstNode> expr_;
};

class ImportNode : public AstNode {
public:
    explicit ImportNode(std::string path) : path_(std::move(path)) {}
    AstNodeType type() const override { return AstNodeType::ImportDecl; }
    const std::string& path() const { return path_; }
    std::string to_string() const override { return "impor \"" + path_ + "\""; }
private:
    std::string path_;
};

class ModuleNode : public AstNode {
public:
    explicit ModuleNode(std::string name) : name_(std::move(name)) {}
    AstNodeType type() const override { return AstNodeType::ModuleDecl; }
    const std::string& name() const { return name_; }
    void add_function(std::shared_ptr<FunctionDeclNode> fn) { functions_.push_back(std::move(fn)); }
    const std::vector<std::shared_ptr<FunctionDeclNode>>& functions() const { return functions_; }
    void add_import(std::shared_ptr<ImportNode> imp) { imports_.push_back(std::move(imp)); }
    const std::vector<std::shared_ptr<ImportNode>>& imports() const { return imports_; }
    std::string to_string() const override { return "defmodule " + name_; }
private:
    std::string name_;
    std::vector<std::shared_ptr<FunctionDeclNode>> functions_;
    std::vector<std::shared_ptr<ImportNode>> imports_;
};

} // namespace garuda::compiler
