#pragma once
// Garuda Language Compiler: Production Lexer & Tokenizer (`lexer.hpp`)
// Menguraikan berkas kode sumber Garuda (.grd) kata kunci Nusantara & Elixir,
// operator, literal, escape sequence, komentar, serta nomor baris & kolom.
#include <cctype>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace garuda::compiler {

enum class TokenType {
    // Keywords Nusantara & BEAM/Elixir
    DefModule, DefFunc, DefMacro, End, Spawn, Receive, After,
    Mili, Detik, Case, In, If, Else, When,
    Self, Print, Send, Destroy, Supervisor, Child, Strategy,
    Try, Catch, Import,

    // Operators
    Arrow, SendOp, Eq, Neq, Lt, Gt, Lte, Gte, Concat,
    Plus, Minus, Multiply, Divide, Assign, Pipe, Question,

    // Literals & Symbols
    Atom, Number, Float, String, Boolean, Nil, Identifier,
    LParen, RParen, LBrace, RBrace, LBracket, RBracket, Comma, Colon,

    // File boundary
    EOF_Token, Unknown
};

struct Token {
    TokenType type = TokenType::Unknown;
    std::string text;
    std::size_t line = 1;
    std::size_t column = 1;

    std::string type_name() const {
        switch (type) {
            case TokenType::DefModule: return "DefModule";
            case TokenType::DefFunc: return "DefFunc";
            case TokenType::End: return "End";
            case TokenType::Spawn: return "Spawn";
            case TokenType::Receive: return "Receive";
            case TokenType::After: return "After";
            case TokenType::If: return "If";
            case TokenType::Else: return "Else";
            case TokenType::Case: return "Case";
            case TokenType::When: return "When";
            case TokenType::Try: return "Try";
            case TokenType::Catch: return "Catch";
            case TokenType::Import: return "Import";
            case TokenType::Print: return "Print";
            case TokenType::SendOp: return "SendOp";
            case TokenType::Arrow: return "Arrow";
            case TokenType::Atom: return "Atom";
            case TokenType::Number: return "Number";
            case TokenType::Float: return "Float";
            case TokenType::String: return "String";
            case TokenType::Boolean: return "Boolean";
            case TokenType::Nil: return "Nil";
            case TokenType::Identifier: return "Identifier";
            case TokenType::EOF_Token: return "EOF";
            default: return "Token";
        }
    }
};

class Lexer {
public:
    explicit Lexer(std::string source) : source_(std::move(source)) {}

    std::vector<Token> tokenize() {
        if (source_.length() >= 3 &&
            static_cast<unsigned char>(source_[0]) == 0xEF &&
            static_cast<unsigned char>(source_[1]) == 0xBB &&
            static_cast<unsigned char>(source_[2]) == 0xBF) {
            pos_ = 3; // Strip UTF-8 BOM!
        }

        std::vector<Token> tokens;
        while (pos_ < source_.length()) {
            char c = source_[pos_];

            // Whitespace handling & line/column tracking
            if (std::isspace(c)) {
                if (c == '\n') {
                    line_++;
                    col_ = 1;
                } else {
                    col_++;
                }
                pos_++;
                continue;
            }

            // Comments: `--` or `#`
            if (c == '#' || (c == '-' && peek(1) == '-')) {
                while (pos_ < source_.length() && source_[pos_] != '\n') pos_++;
                continue;
            }

            // Identifiers & Keywords
            if (std::isalpha(c) || c == '_') {
                std::size_t start_col = col_;
                std::size_t start_pos = pos_;
                while (pos_ < source_.length() && (std::isalnum(source_[pos_]) || source_[pos_] == '_' || source_[pos_] == '.')) {
                    pos_++;
                    col_++;
                }
                std::string word = source_.substr(start_pos, pos_ - start_pos);

                TokenType type = TokenType::Identifier;
                if (word == "modul" || word == "defmodule") type = TokenType::DefModule;
                else if (word == "fungsi" || word == "def") type = TokenType::DefFunc;
                else if (word == "makro" || word == "macro") type = TokenType::DefMacro;
                else if (word == "rampung" || word == "end") type = TokenType::End;
                else if (word == "cakra" || word == "spawn") type = TokenType::Spawn;
                else if (word == "tampa" || word == "receive") type = TokenType::Receive;
                else if (word == "setelah" || word == "after") type = TokenType::After;
                else if (word == "mili") type = TokenType::Mili;
                else if (word == "detik") type = TokenType::Detik;
                else if (word == "tinjau" || word == "case") type = TokenType::Case;
                else if (word == "ing" || word == "in") type = TokenType::In;
                else if (word == "yen" || word == "if") type = TokenType::If;
                else if (word == "lain" || word == "else") type = TokenType::Else;
                else if (word == "ketika" || word == "when") type = TokenType::When;
                else if (word == "diri" || word == "self") type = TokenType::Self;
                else if (word == "cetak" || word == "puts") type = TokenType::Print;
                else if (word == "kirim" || word == "send") type = TokenType::Send;
                else if (word == "musnah" || word == "kill") type = TokenType::Destroy;
                else if (word == "pengawas" || word == "supervisor") type = TokenType::Supervisor;
                else if (word == "anak" || word == "child") type = TokenType::Child;
                else if (word == "strategi" || word == "strategy") type = TokenType::Strategy;
                else if (word == "coba" || word == "try") type = TokenType::Try;
                else if (word == "tangkap" || word == "catch") type = TokenType::Catch;
                else if (word == "impor" || word == "import") type = TokenType::Import;
                else if (word == "benar" || word == "salah" || word == "true" || word == "false") type = TokenType::Boolean;
                else if (word == "nil") type = TokenType::Nil;

                tokens.push_back({type, word, line_, start_col});
                continue;
            }

            // Atoms: `:ok`, `:error`
            if (c == ':') {
                std::size_t start_col = col_;
                pos_++; col_++;
                std::size_t start_pos = pos_;
                while (pos_ < source_.length() && (std::isalnum(source_[pos_]) || source_[pos_] == '_')) {
                    pos_++; col_++;
                }
                tokens.push_back({TokenType::Atom, source_.substr(start_pos, pos_ - start_pos), line_, start_col});
                continue;
            }

            // Numbers: Hex (`0x1F`), Biner (`0b1010`), Float (`3.14`), Int (`42`)
            if (std::isdigit(c)) {
                std::size_t start_col = col_;
                std::size_t start_pos = pos_;

                if (c == '0' && (peek(1) == 'x' || peek(1) == 'X')) { // Hex
                    pos_ += 2; col_ += 2;
                    while (pos_ < source_.length() && std::isxdigit(source_[pos_])) { pos_++; col_++; }
                } else if (c == '0' && (peek(1) == 'b' || peek(1) == 'B')) { // Biner
                    pos_ += 2; col_ += 2;
                    while (pos_ < source_.length() && (source_[pos_] == '0' || source_[pos_] == '1')) { pos_++; col_++; }
                } else { // Desimal atau Float
                    bool is_float = false;
                    while (pos_ < source_.length() && (std::isdigit(source_[pos_]) || source_[pos_] == '.')) {
                        if (source_[pos_] == '.') {
                            if (is_float) break;
                            is_float = true;
                        }
                        pos_++; col_++;
                    }
                    tokens.push_back({is_float ? TokenType::Float : TokenType::Number, source_.substr(start_pos, pos_ - start_pos), line_, start_col});
                    continue;
                }

                tokens.push_back({TokenType::Number, source_.substr(start_pos, pos_ - start_pos), line_, start_col});
                continue;
            }

            // Strings dengan Escape Sequence (\n, \t, \")
            if (c == '"') {
                std::size_t start_col = col_;
                pos_++; col_++;
                std::string str_val;
                while (pos_ < source_.length() && source_[pos_] != '"') {
                    if (source_[pos_] == '\\' && pos_ + 1 < source_.length()) {
                        char next_c = source_[pos_ + 1];
                        if (next_c == 'n') str_val += '\n';
                        else if (next_c == 't') str_val += '\t';
                        else if (next_c == '"') str_val += '"';
                        else if (next_c == '\\') str_val += '\\';
                        else str_val += next_c;
                        pos_ += 2; col_ += 2;
                    } else {
                        if (source_[pos_] == '\n') { line_++; col_ = 1; } else { col_++; }
                        str_val += source_[pos_++];
                    }
                }
                if (pos_ < source_.length()) { pos_++; col_++; } // Close "
                tokens.push_back({TokenType::String, str_val, line_, start_col});
                continue;
            }

            // Multi-char Operators: `->`, `<-`, `==`, `!=`, `<=`, `>=`, `++`
            std::size_t start_col = col_;
            if (c == '-' && peek(1) == '>') { tokens.push_back({TokenType::Arrow, "->", line_, start_col}); pos_ += 2; col_ += 2; continue; }
            if (c == '<' && peek(1) == '-') { tokens.push_back({TokenType::SendOp, "<-", line_, start_col}); pos_ += 2; col_ += 2; continue; }
            if (c == '=' && peek(1) == '=') { tokens.push_back({TokenType::Eq, "==", line_, start_col}); pos_ += 2; col_ += 2; continue; }
            if (c == '!' && peek(1) == '=') { tokens.push_back({TokenType::Neq, "!=", line_, start_col}); pos_ += 2; col_ += 2; continue; }
            if (c == '<' && peek(1) == '=') { tokens.push_back({TokenType::Lte, "<=", line_, start_col}); pos_ += 2; col_ += 2; continue; }
            if (c == '>' && peek(1) == '=') { tokens.push_back({TokenType::Gte, ">=", line_, start_col}); pos_ += 2; col_ += 2; continue; }
            if (c == '+' && peek(1) == '+') { tokens.push_back({TokenType::Concat, "++", line_, start_col}); pos_ += 2; col_ += 2; continue; }

            // Single-char Symbols & Operators
            if (c == '+') { tokens.push_back({TokenType::Plus, "+", line_, start_col}); pos_++; col_++; continue; }
            if (c == '-') { tokens.push_back({TokenType::Minus, "-", line_, start_col}); pos_++; col_++; continue; }
            if (c == '*') { tokens.push_back({TokenType::Multiply, "*", line_, start_col}); pos_++; col_++; continue; }
            if (c == '/') { tokens.push_back({TokenType::Divide, "/", line_, start_col}); pos_++; col_++; continue; }
            if (c == '?') { tokens.push_back({TokenType::Question, "?", line_, start_col}); pos_++; col_++; continue; }
            if (c == '=') { tokens.push_back({TokenType::Assign, "=", line_, start_col}); pos_++; col_++; continue; }
            if (c == '|') { tokens.push_back({TokenType::Pipe, "|", line_, start_col}); pos_++; col_++; continue; }
            if (c == '<') { tokens.push_back({TokenType::Lt, "<", line_, start_col}); pos_++; col_++; continue; }
            if (c == '>') { tokens.push_back({TokenType::Gt, ">", line_, start_col}); pos_++; col_++; continue; }
            if (c == '(') { tokens.push_back({TokenType::LParen, "(", line_, start_col}); pos_++; col_++; continue; }
            if (c == ')') { tokens.push_back({TokenType::RParen, ")", line_, start_col}); pos_++; col_++; continue; }
            if (c == '{') { tokens.push_back({TokenType::LBrace, "{", line_, start_col}); pos_++; col_++; continue; }
            if (c == '}') { tokens.push_back({TokenType::RBrace, "}", line_, start_col}); pos_++; col_++; continue; }
            if (c == '[') { tokens.push_back({TokenType::LBracket, "[", line_, start_col}); pos_++; col_++; continue; }
            if (c == ']') { tokens.push_back({TokenType::RBracket, "]", line_, start_col}); pos_++; col_++; continue; }
            if (c == ',') { tokens.push_back({TokenType::Comma, ",", line_, start_col}); pos_++; col_++; continue; }

            pos_++; col_++;
        }
        tokens.push_back({TokenType::EOF_Token, "", line_, col_});
        return tokens;
    }

private:
    char peek(std::size_t offset) const {
        if (pos_ + offset < source_.length()) return source_[pos_ + offset];
        return '\0';
    }

    std::string source_;
    std::size_t pos_ = 0;
    std::size_t line_ = 1;
    std::size_t col_ = 1;
};

} // namespace garuda::compiler
