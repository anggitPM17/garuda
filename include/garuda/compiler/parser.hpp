#pragma once
// Garuda Language Compiler: Recursive Descent & Precedence Climbing Parser (`parser.hpp`)
// Mengubah alur Token menjadi Pohon Sintaksis Abstrak (AST) dengan presedensi operator presisi.
#include "ast.hpp"
#include "lexer.hpp"

#include <cctype>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace garuda::compiler {

class ParseError : public std::runtime_error {
public:
    ParseError(const std::string& msg, std::size_t line, std::size_t col)
        : std::runtime_error("ParseError at line " + std::to_string(line) + ", col " + std::to_string(col) + ": " + msg),
          raw_message(msg), line(line), column(col) {}

    std::string raw_message;
    std::size_t line = 0;
    std::size_t column = 0;
};

class Parser {
public:
    explicit Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)) {}

    std::shared_ptr<ModuleNode> parse_program() {
        std::string mod_name = "Utama";
        if (check(TokenType::DefModule)) {
            advance();
            const Token& name_tok = expect(TokenType::Identifier, "Expected module name after 'modul'/'defmodule'");
            mod_name = name_tok.text;
        }

        auto module_node = std::make_shared<ModuleNode>(mod_name);

        while (!is_at_end() && !check(TokenType::EOF_Token)) {
            if (check(TokenType::Import)) {
                advance();
                const Token& path_tok = expect(TokenType::String, "Expected string file path after 'impor'");
                module_node->add_import(std::make_shared<ImportNode>(path_tok.text));
            } else if (check(TokenType::DefFunc)) {
                module_node->add_function(parse_function());
            } else if (check(TokenType::DefMacro)) {
                module_node->add_function(parse_macro());
            } else {
                advance();
            }
        }
        return module_node;
    }

    std::shared_ptr<AstNode> parse_expression_standalone() {
        return parse_expr();
    }

private:
    const Token& peek(std::size_t offset = 0) const {
        if (pos_ + offset < tokens_.size()) return tokens_[pos_ + offset];
        return tokens_.back();
    }

    const Token& advance() {
        if (!is_at_end()) pos_++;
        return tokens_[pos_ - 1];
    }

    bool check(TokenType type) const {
        if (is_at_end()) return false;
        return peek().type == type;
    }

    bool match(TokenType type) {
        if (check(type)) {
            advance();
            return true;
        }
        return false;
    }

    const Token& expect(TokenType type, const std::string& msg) {
        if (check(type)) return advance();
        const auto& tok = peek();
        throw ParseError(msg, tok.line, tok.column);
    }

    bool is_at_end() const {
        return pos_ >= tokens_.size() || peek().type == TokenType::EOF_Token;
    }

    // Function declaration parser
    std::shared_ptr<FunctionDeclNode> parse_function() {
        expect(TokenType::DefFunc, "Expected 'fungsi'/'def'");
        const Token& name_tok = expect(TokenType::Identifier, "Expected function name");
        expect(TokenType::LParen, "Expected '(' after function name");

        std::vector<std::string> params;
        if (!check(TokenType::RParen)) {
            do {
                const Token& p_tok = expect(TokenType::Identifier, "Expected parameter name");
                params.push_back(p_tok.text);
            } while (match(TokenType::Comma));
        }
        expect(TokenType::RParen, "Expected ')' after parameters");
        expect(TokenType::Arrow, "Expected '->' before function body");

        std::vector<std::shared_ptr<AstNode>> body;
        while (!check(TokenType::End) && !is_at_end()) {
            body.push_back(parse_expr());
            match(TokenType::Comma);
        }
        expect(TokenType::End, "Expected 'rampung'/'end' to close function declaration");

        return std::make_shared<FunctionDeclNode>(name_tok.text, std::move(params), std::move(body));
    }

    std::shared_ptr<MacroDefNode> parse_macro() {
        expect(TokenType::DefMacro, "Expected 'makro'/'macro'");
        const Token& name_tok = expect(TokenType::Identifier, "Expected macro name");
        expect(TokenType::LParen, "Expected '(' after macro name");

        std::vector<std::string> params;
        if (!check(TokenType::RParen)) {
            do {
                const Token& p_tok = expect(TokenType::Identifier, "Expected parameter name");
                params.push_back(p_tok.text);
            } while (match(TokenType::Comma));
        }
        expect(TokenType::RParen, "Expected ')' after parameters");
        expect(TokenType::Arrow, "Expected '->' before macro body");

        std::vector<std::shared_ptr<AstNode>> body;
        while (!check(TokenType::End) && !is_at_end()) {
            body.push_back(parse_expr());
            match(TokenType::Comma);
        }
        expect(TokenType::End, "Expected 'rampung'/'end' to close macro declaration");

        return std::make_shared<MacroDefNode>(name_tok.text, std::move(params), std::move(body));
    }

    // Expression Precedence Climbing:
    // 1. Assign (=)
    // 2. Send (<-)
    // 3. Or (atau)
    // 4. And (dan)
    // 5. Not (bukan)
    // 6. Cmp (==, !=, <, >, <=, >=)
    // 7. Concat (++)
    // 8. Add/Sub (+, -)
    // 9. Mul/Div (*, /)
    // 10. Unary (-)
    // 11. Primary

    std::shared_ptr<AstNode> parse_expr() {
        auto node = parse_assign();
        if (match(TokenType::Question)) {
            return std::make_shared<TryOperatorNode>(node);
        }
        return node;
    }

    std::shared_ptr<TryCatchNode> parse_try_catch() {
        expect(TokenType::Try, "Expected 'coba'/'try'");

        std::vector<std::shared_ptr<AstNode>> try_body;
        while (!check(TokenType::Catch) && !check(TokenType::End) && !is_at_end()) {
            try_body.push_back(parse_expr());
            match(TokenType::Comma);
        }

        expect(TokenType::Catch, "Expected 'tangkap'/'catch' after 'coba' body");
        auto catch_pattern = parse_expr();
        expect(TokenType::Arrow, "Expected '->' after catch pattern");

        std::vector<std::shared_ptr<AstNode>> catch_body;
        while (!check(TokenType::End) && !is_at_end()) {
            catch_body.push_back(parse_expr());
            match(TokenType::Comma);
        }
        expect(TokenType::End, "Expected 'rampung'/'end' to close try/catch block");

        return std::make_shared<TryCatchNode>(std::move(try_body), catch_pattern, std::move(catch_body));
    }

    std::shared_ptr<AstNode> parse_assign() {
        auto left = parse_pipe();
        if (check(TokenType::Assign)) {
            advance();
            auto right = parse_assign();
            return std::make_shared<AssignmentExprNode>(left, right);
        }
        return left;
    }

    std::shared_ptr<AstNode> parse_pipe() {
        auto left = parse_send();
        while (check(TokenType::Pipe)) {
            advance();
            auto right = parse_send();
            left = std::make_shared<BinaryExprNode>("|>", left, right);
        }
        return left;
    }

    std::shared_ptr<AstNode> parse_send() {
        auto left = parse_or();
        while (check(TokenType::SendOp)) {
            advance();
            auto right = parse_or();
            left = std::make_shared<SendExprNode>(left, right);
        }
        return left;
    }

    std::shared_ptr<AstNode> parse_or() {
        auto left = parse_and();
        while (check(TokenType::Identifier) && peek().text == "atau") {
            advance();
            auto right = parse_and();
            left = std::make_shared<BinaryExprNode>("atau", left, right);
        }
        return left;
    }

    std::shared_ptr<AstNode> parse_and() {
        auto left = parse_not();
        while (check(TokenType::Identifier) && peek().text == "dan") {
            advance();
            auto right = parse_not();
            left = std::make_shared<BinaryExprNode>("dan", left, right);
        }
        return left;
    }

    std::shared_ptr<AstNode> parse_not() {
        if (check(TokenType::Identifier) && peek().text == "bukan") {
            advance();
            auto operand = parse_not();
            return std::make_shared<UnaryExprNode>("bukan ", operand);
        }
        return parse_cmp();
    }

    std::shared_ptr<AstNode> parse_cmp() {
        auto left = parse_concat();
        while (check(TokenType::Eq) || check(TokenType::Neq) ||
               check(TokenType::Lt) || check(TokenType::Gt) ||
               check(TokenType::Lte) || check(TokenType::Gte)) {
            std::string op = advance().text;
            auto right = parse_concat();
            left = std::make_shared<BinaryExprNode>(op, left, right);
        }
        return left;
    }

    std::shared_ptr<AstNode> parse_concat() {
        auto left = parse_add();
        while (check(TokenType::Concat)) {
            advance();
            auto right = parse_add();
            left = std::make_shared<BinaryExprNode>("++", left, right);
        }
        return left;
    }

    std::shared_ptr<AstNode> parse_add() {
        auto left = parse_mul();
        while (check(TokenType::Plus) || check(TokenType::Minus)) {
            std::string op = advance().text;
            auto right = parse_mul();
            left = std::make_shared<BinaryExprNode>(op, left, right);
        }
        return left;
    }

    std::shared_ptr<AstNode> parse_mul() {
        auto left = parse_unary();
        while (check(TokenType::Multiply) || check(TokenType::Divide)) {
            std::string op = advance().text;
            auto right = parse_unary();
            left = std::make_shared<BinaryExprNode>(op, left, right);
        }
        return left;
    }

    std::shared_ptr<AstNode> parse_unary() {
        if (check(TokenType::Minus)) {
            advance();
            auto operand = parse_primary();
            return std::make_shared<UnaryExprNode>("-", operand);
        }
        return parse_primary();
    }

    std::shared_ptr<AstNode> parse_primary() {
        if (check(TokenType::Number)) {
            std::int64_t v = std::stoll(advance().text);
            return std::make_shared<LiteralNode>(Term(v));
        }
        if (check(TokenType::Float)) {
            double v = std::stod(advance().text);
            return std::make_shared<LiteralNode>(Term(v));
        }
        if (check(TokenType::String)) {
            return std::make_shared<LiteralNode>(Term(advance().text));
        }
        if (check(TokenType::Atom)) {
            return std::make_shared<AtomExprNode>(advance().text);
        }
        if (check(TokenType::Boolean)) {
            std::string b_str = advance().text;
            return std::make_shared<BoolExprNode>(b_str == "benar" || b_str == "true");
        }
        if (check(TokenType::Nil)) {
            advance();
            return std::make_shared<NilExprNode>();
        }
        if (check(TokenType::Self)) {
            advance();
            return std::make_shared<SelfExprNode>();
        }

        // cetak(...)
        if (check(TokenType::Print)) {
            advance();
            expect(TokenType::LParen, "Expected '(' after 'cetak'");
            auto val = parse_expr();
            expect(TokenType::RParen, "Expected ')' after 'cetak' argument");
            return std::make_shared<CetakExprNode>(val);
        }

        // cakra(...)
        if (check(TokenType::Spawn)) {
            advance();
            expect(TokenType::LParen, "Expected '(' after 'cakra'");
            auto fun = parse_expr();
            expect(TokenType::RParen, "Expected ')' after 'cakra' argument");
            return std::make_shared<SpawnExprNode>(fun);
        }

        // fungsi(...) -> ... rampung (Anonymous function)
        if (check(TokenType::DefFunc)) {
            advance();
            expect(TokenType::LParen, "Expected '(' for anonymous function");
            std::vector<std::string> params;
            if (!check(TokenType::RParen)) {
                do {
                    params.push_back(expect(TokenType::Identifier, "Expected parameter").text);
                } while (match(TokenType::Comma));
            }
            expect(TokenType::RParen, "Expected ')'");
            expect(TokenType::Arrow, "Expected '->'");
            std::vector<std::shared_ptr<AstNode>> body;
            while (!check(TokenType::End) && !is_at_end()) {
                body.push_back(parse_expr());
            }
            expect(TokenType::End, "Expected 'rampung'/'end'");
            return std::make_shared<AnonFunExprNode>(std::move(params), std::move(body));
        }

        // coba ... tangkap ... rampung (Try Catch expression)
        if (check(TokenType::Try)) {
            return parse_try_catch();
        }

        // yen ... lain ... rampung (If expression)
        if (check(TokenType::If)) {
            advance();
            auto cond = parse_expr();
            expect(TokenType::Arrow, "Expected '->' after if condition");
            std::vector<std::shared_ptr<AstNode>> then_body;
            std::vector<std::shared_ptr<AstNode>> else_body;

            while (!check(TokenType::Else) && !check(TokenType::End) && !is_at_end()) {
                then_body.push_back(parse_expr());
            }

            if (match(TokenType::Else)) {
                if (check(TokenType::Arrow)) advance();
                while (!check(TokenType::End) && !is_at_end()) {
                    else_body.push_back(parse_expr());
                }
            }

            expect(TokenType::End, "Expected 'rampung'/'end' to close if");
            std::vector<IfClause> clauses = {{cond, then_body}};
            return std::make_shared<IfExprNode>(clauses, else_body);
        }

        // tinjau ... ing ... rampung (Case expression)
        if (check(TokenType::Case)) {
            advance();
            auto subject = parse_expr();
            expect(TokenType::In, "Expected 'ing'/'in' after case subject");

            std::vector<CaseClause> clauses;
            while (!check(TokenType::End) && !is_at_end()) {
                auto pattern = parse_pattern();
                expect(TokenType::Arrow, "Expected '->' after pattern");
                std::vector<std::shared_ptr<AstNode>> body = {parse_expr()};
                clauses.push_back({pattern, nullptr, body});
            }

            expect(TokenType::End, "Expected 'rampung'/'end' to close case");
            return std::make_shared<CaseExprNode>(subject, clauses);
        }

        // tampa ... rampung (Receive expression)
        if (check(TokenType::Receive)) {
            advance();
            std::vector<ReceiveClause> clauses;
            while (!check(TokenType::End) && !check(TokenType::After) && !is_at_end()) {
                auto pattern = parse_pattern();
                expect(TokenType::Arrow, "Expected '->' after receive pattern");
                std::vector<std::shared_ptr<AstNode>> body = {parse_expr()};
                clauses.push_back({pattern, nullptr, body});
            }
            expect(TokenType::End, "Expected 'rampung'/'end' to close receive");
            return std::make_shared<ReceiveExprNode>(clauses);
        }

        // Tuple {...}
        if (check(TokenType::LBrace)) {
            advance();
            std::vector<std::shared_ptr<AstNode>> items;
            if (!check(TokenType::RBrace)) {
                do {
                    items.push_back(parse_expr());
                } while (match(TokenType::Comma));
            }
            expect(TokenType::RBrace, "Expected '}'");
            return std::make_shared<TupleExprNode>(items);
        }

        // List [...] or List cons [H | T]
        if (check(TokenType::LBracket)) {
            advance();
            std::vector<std::shared_ptr<AstNode>> items;
            if (!check(TokenType::RBracket)) {
                do {
                    items.push_back(parse_expr());
                } while (match(TokenType::Comma));
            }
            if (match(TokenType::Pipe)) {
                auto tail = parse_expr();
                expect(TokenType::RBracket, "Expected ']'");
                return std::make_shared<ListConsExprNode>(items, tail);
            }
            expect(TokenType::RBracket, "Expected ']'");
            return std::make_shared<ListExprNode>(items);
        }

        // Parentheses (expr)
        if (check(TokenType::LParen)) {
            advance();
            auto inner = parse_expr();
            expect(TokenType::RParen, "Expected ')'");
            return inner;
        }

        // Identifier or Function Call
        if (check(TokenType::Identifier)) {
            std::string name = advance().text;
            if (check(TokenType::LParen)) {
                advance(); // (
                std::vector<std::shared_ptr<AstNode>> args;
                if (!check(TokenType::RParen)) {
                    do {
                        args.push_back(parse_expr());
                    } while (match(TokenType::Comma));
                }
                expect(TokenType::RParen, "Expected ')'");
                return std::make_shared<CallExprNode>(name, args);
            }
            return std::make_shared<VariableNode>(name);
        }

        const auto& tok = peek();
        throw ParseError("Unexpected token '" + tok.text + "'", tok.line, tok.column);
    }

    // Pattern Parser
    std::shared_ptr<AstNode> parse_pattern() {
        if (check(TokenType::Identifier)) {
            std::string name = advance().text;
            if (name == "_") return std::make_shared<PatternWildcardNode>();
            if (!name.empty() && std::isupper(name[0])) return std::make_shared<PatternVarNode>(name);
            return std::make_shared<PatternLiteralNode>(Term(Atom{name}));
        }
        if (check(TokenType::Atom)) {
            return std::make_shared<PatternLiteralNode>(Term(Atom{advance().text}));
        }
        if (check(TokenType::Number)) {
            return std::make_shared<PatternLiteralNode>(Term(std::stoll(advance().text)));
        }
        if (check(TokenType::String)) {
            return std::make_shared<PatternLiteralNode>(Term(advance().text));
        }
        if (check(TokenType::LBrace)) {
            advance();
            std::vector<std::shared_ptr<AstNode>> items;
            if (!check(TokenType::RBrace)) {
                do {
                    items.push_back(parse_pattern());
                } while (match(TokenType::Comma));
            }
            expect(TokenType::RBrace, "Expected '}' in pattern tuple");
            return std::make_shared<PatternTupleNode>(items);
        }
        if (check(TokenType::LBracket)) {
            advance();
            if (check(TokenType::RBracket)) {
                advance();
                return std::make_shared<ListExprNode>(std::vector<std::shared_ptr<AstNode>>{});
            }
            auto head = parse_pattern();
            if (match(TokenType::Pipe)) {
                auto tail = parse_pattern();
                expect(TokenType::RBracket, "Expected ']' in list pattern [H | T]");
                return std::make_shared<PatternListHeadTailNode>(head, tail);
            }
            std::vector<std::shared_ptr<AstNode>> items = {head};
            while (match(TokenType::Comma)) {
                items.push_back(parse_pattern());
            }
            expect(TokenType::RBracket, "Expected ']' in list pattern");
            return std::make_shared<PatternTupleNode>(items);
        }
        return parse_expr();
    }

    std::vector<Token> tokens_;
    std::size_t pos_ = 0;
};

} // namespace garuda::compiler
