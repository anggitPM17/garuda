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
                if (check(TokenType::Colon) || (check(TokenType::Atom) && peek().text.empty())) {
                    advance(); // skip ':'
                    if (check(TokenType::Identifier)) {
                        advance(); // skip type
                        if (check(TokenType::Lt)) {
                            advance();
                            while (!check(TokenType::Gt) && !is_at_end()) advance();
                            if (check(TokenType::Gt)) advance();
                        }
                    }
                }
            } while (match(TokenType::Comma));
        }
        expect(TokenType::RParen, "Expected ')' after parameters");

        if (check(TokenType::Arrow)) {
            std::size_t save_pos = pos_;
            advance(); // skip '->'
            if (check(TokenType::Identifier)) {
                advance(); // skip return type
                if (check(TokenType::Lt)) {
                    advance();
                    while (!check(TokenType::Gt) && !is_at_end()) advance();
                    if (check(TokenType::Gt)) advance();
                }
                if (!check(TokenType::Arrow)) {
                    pos_ = save_pos;
                }
            } else {
                pos_ = save_pos;
            }
        }

        std::shared_ptr<AstNode> guard_cond = nullptr;
        if (check(TokenType::When) || (check(TokenType::Identifier) && peek().text == "nalika")) {
            advance();
            guard_cond = parse_expr();
        }

        expect(TokenType::Arrow, "Expected '->' before function body");

        std::vector<std::shared_ptr<AstNode>> body;
        while (!check(TokenType::End) && !is_at_end()) {
            body.push_back(parse_expr());
            match(TokenType::Comma);
        }
        expect(TokenType::End, "Expected 'rampung'/'end' to close function declaration");

        if (guard_cond) {
            std::vector<std::shared_ptr<AstNode>> else_body = {std::make_shared<NilExprNode>()};
            std::vector<IfClause> clauses = {{guard_cond, body}};
            auto if_guard = std::make_shared<IfExprNode>(clauses, else_body);
            std::vector<std::shared_ptr<AstNode>> guarded_body = {if_guard};
            return std::make_shared<FunctionDeclNode>(name_tok.text, std::move(params), std::move(guarded_body));
        }

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
        if (check(TokenType::Comma)) {
            std::size_t save_pos = pos_;
            std::vector<std::shared_ptr<AstNode>> targets = {left};
            while (match(TokenType::Comma)) {
                targets.push_back(parse_pipe());
            }
            if (check(TokenType::Assign) && peek(1).type != TokenType::Gt) {
                advance();
                std::vector<std::shared_ptr<AstNode>> values;
                values.push_back(parse_pipe());
                while (match(TokenType::Comma)) {
                    values.push_back(parse_pipe());
                }
                return std::make_shared<MultiAssignNode>(targets, values);
            }
            pos_ = save_pos;
        }
        if (check(TokenType::Assign) && peek(1).type != TokenType::Gt) {
            advance();
            auto right = parse_assign();
            return std::make_shared<AssignmentExprNode>(left, right);
        }
        return left;
    }

    std::shared_ptr<AstNode> parse_pipe() {
        auto left = parse_send();
        while (check(TokenType::Pipe) && peek().text == "|>") {
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

    std::shared_ptr<AstNode> parse_string_interpolation(const std::string& str) {
        std::size_t i = 0;
        std::shared_ptr<AstNode> result = nullptr;

        while (i < str.length()) {
            std::size_t start = str.find("#{", i);
            if (start == std::string::npos) {
                std::string literal_part = str.substr(i);
                if (!literal_part.empty()) {
                    auto lit_node = std::make_shared<LiteralNode>(Term(literal_part));
                    result = result ? std::make_shared<BinaryExprNode>("++", result, lit_node) : std::static_pointer_cast<AstNode>(lit_node);
                }
                break;
            }

            std::string literal_part = str.substr(i, start - i);
            if (!literal_part.empty()) {
                auto lit_node = std::make_shared<LiteralNode>(Term(literal_part));
                result = result ? std::make_shared<BinaryExprNode>("++", result, lit_node) : std::static_pointer_cast<AstNode>(lit_node);
            }

            std::size_t end = str.find('}', start + 2);
            if (end == std::string::npos) {
                std::string remaining_part = str.substr(start);
                auto lit_node = std::make_shared<LiteralNode>(Term(remaining_part));
                result = result ? std::make_shared<BinaryExprNode>("++", result, lit_node) : std::static_pointer_cast<AstNode>(lit_node);
                break;
            }

            std::string expr_str = str.substr(start + 2, end - (start + 2));
            std::string fmt_spec = "";
            std::size_t colon_pos = expr_str.find(':');
            if (colon_pos != std::string::npos) {
                fmt_spec = expr_str.substr(colon_pos + 1);
                expr_str = expr_str.substr(0, colon_pos);
            }

            Lexer expr_lexer(expr_str);
            auto expr_tokens = expr_lexer.tokenize();
            if (!expr_tokens.empty()) {
                Parser expr_parser(expr_tokens);
                auto expr_ast = expr_parser.parse_expr();
                if (expr_ast) {
                    if (!fmt_spec.empty()) {
                        expr_ast = std::make_shared<CallExprNode>("String.format", std::vector<std::shared_ptr<AstNode>>{
                            expr_ast, std::make_shared<LiteralNode>(Term(fmt_spec))
                        });
                    }
                    result = result ? std::make_shared<BinaryExprNode>("++", result, expr_ast) : expr_ast;
                }
            }

            i = end + 1;
        }

        if (!result) {
            result = std::make_shared<LiteralNode>(Term(""));
        }
        return result;
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
            std::string str_val = advance().text;
            if (str_val.find("#{") != std::string::npos) {
                return parse_string_interpolation(str_val);
            }
            return std::make_shared<LiteralNode>(Term(str_val));
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
            if (match(TokenType::LParen)) {
                expect(TokenType::RParen, "Expected ')' after 'diri'");
            }
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
                std::shared_ptr<AstNode> guard = nullptr;
                if (check(TokenType::When) || (check(TokenType::Identifier) && (peek().text == "nalika" || peek().text == "when"))) {
                    advance();
                    guard = parse_expr();
                }
                expect(TokenType::Arrow, "Expected '->' after pattern");
                std::vector<std::shared_ptr<AstNode>> body = {parse_expr()};
                clauses.push_back({pattern, guard, body});
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

        // Map %{...} or Map Comprehension %{k => v untuk pattern ing source}
        if (check(TokenType::Percent) || (check(TokenType::Unknown) && peek().text == "%")) {
            advance();
            expect(TokenType::LBrace, "Expected '{' after '%' in map expression");
            if (check(TokenType::RBrace)) {
                advance();
                return std::make_shared<MapExprNode>(std::vector<MapEntry>{});
            }

            auto first_key = parse_expr();
            if (check(TokenType::Arrow)) {
                advance();
            } else if (check(TokenType::Assign)) {
                advance();
                if (check(TokenType::Gt)) advance();
            } else if (check(TokenType::Colon)) {
                advance();
            }
            auto first_val = parse_expr();

            if (check(TokenType::Identifier) && (peek().text == "untuk" || peek().text == "for")) {
                advance();

                std::shared_ptr<AstNode> pattern = nullptr;
                if (check(TokenType::Identifier) && peek(1).type == TokenType::Comma) {
                    auto k_name = advance().text;
                    advance();
                    auto v_name = expect(TokenType::Identifier, "Expected second pattern variable").text;
                    pattern = std::make_shared<PatternTupleNode>(std::vector<std::shared_ptr<AstNode>>{
                        std::make_shared<VariableNode>(k_name),
                        std::make_shared<VariableNode>(v_name)
                    });
                } else {
                    pattern = parse_pattern();
                }

                expect(TokenType::In, "Expected 'ing'/'in' after pattern in map comprehension");
                auto source_expr = parse_expr();
                std::shared_ptr<AstNode> filter_cond = nullptr;
                if (check(TokenType::If) || (check(TokenType::Identifier) && (peek().text == "yen" || peek().text == "if"))) {
                    advance();
                    filter_cond = parse_expr();
                }
                expect(TokenType::RBrace, "Expected '}' at end of map comprehension");
                return std::make_shared<MapComprehensionNode>(first_key, first_val, pattern, source_expr, filter_cond);
            }

            std::vector<MapEntry> entries = {{first_key, first_val}};
            while (match(TokenType::Comma)) {
                auto key = parse_expr();
                if (check(TokenType::Arrow)) {
                    advance();
                } else if (check(TokenType::Assign)) {
                    advance();
                    if (check(TokenType::Gt)) advance();
                } else if (check(TokenType::Colon)) {
                    advance();
                }
                auto val = parse_expr();
                entries.push_back({key, val});
            }
            expect(TokenType::RBrace, "Expected '}' at end of map expression");
            return std::make_shared<MapExprNode>(entries);
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

        // List [...] or List cons [H | T] or List Comprehension [expr untuk pattern ing list]
        if (check(TokenType::LBracket)) {
            advance();
            if (check(TokenType::RBracket)) {
                advance();
                return std::make_shared<ListExprNode>(std::vector<std::shared_ptr<AstNode>>{});
            }
            auto first_expr = parse_expr();
            if (check(TokenType::Identifier) && (peek().text == "untuk" || peek().text == "for")) {
                advance();
                auto pattern = parse_pattern();
                expect(TokenType::In, "Expected 'ing'/'in' after pattern in comprehension");
                auto list_expr = parse_expr();
                std::shared_ptr<AstNode> filter_cond = nullptr;
                if (check(TokenType::If) || (check(TokenType::Identifier) && (peek().text == "yen" || peek().text == "if"))) {
                    advance();
                    filter_cond = parse_expr();
                }
                expect(TokenType::RBracket, "Expected ']' at end of list comprehension");
                return std::make_shared<ListComprehensionNode>(first_expr, pattern, list_expr, filter_cond);
            }

            std::vector<std::shared_ptr<AstNode>> items = {first_expr};
            while (match(TokenType::Comma)) {
                items.push_back(parse_expr());
            }
            if (check(TokenType::Pipe) && peek().text == "|") {
                advance();
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
            if (check(TokenType::Pipe) && peek().text == "|") {
                advance();
                auto tail = parse_pattern();
                expect(TokenType::RBracket, "Expected ']' in list pattern [H | T]");
                return std::make_shared<PatternListHeadTailNode>(head, tail);
            }
            std::vector<std::shared_ptr<AstNode>> items = {head};
            while (match(TokenType::Comma)) {
                items.push_back(parse_pattern());
            }
            expect(TokenType::RBracket, "Expected ']' in list pattern");
            return std::make_shared<ListExprNode>(items);
        }
        return parse_expr();
    }

    std::vector<Token> tokens_;
    std::size_t pos_ = 0;
};

} // namespace garuda::compiler
