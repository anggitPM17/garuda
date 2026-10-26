#pragma once
// Garuda Standard Library: Native JSON Parser & Serializer (`json.hpp`)
// Mengimplementasikan JSON Encoder & Parser tanpa dependensi eksternal.
#include "../core/term.hpp"

#include <cctype>
#include <cstdint>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace garuda::beam::stdlib {

class Json {
public:
    // ---- ENCODER ----
    static std::string encode(const Term& term) {
        if (auto* i = term.get_if<std::int64_t>()) {
            return std::to_string(*i);
        }
        if (auto* d = term.get_if<double>()) {
            std::ostringstream ss;
            ss << *d;
            return ss.str();
        }
        if (auto* s = term.get_if<std::string>()) {
            return escape_string(*s);
        }
        if (auto* a = term.get_if<Atom>()) {
            if (a->name == "true" || a->name == "benar") return "true";
            if (a->name == "false" || a->name == "salah") return "false";
            if (a->name == "nil" || a->name == "null") return "null";
            return escape_string(a->name);
        }
        if (auto* lst = term.get_if<garuda::beam::List>()) {
            const auto& items = seq_items(lst->items);
            bool is_proplist = !items.empty();
            for (const auto& item : items) {
                if (auto* tup = item.get_if<garuda::beam::Tuple>()) {
                    if (seq_items(tup->items).size() != 2) {
                        is_proplist = false;
                        break;
                    }
                } else {
                    is_proplist = false;
                    break;
                }
            }

            if (is_proplist) {
                std::ostringstream ss;
                ss << "{";
                for (std::size_t idx = 0; idx < items.size(); ++idx) {
                    if (idx > 0) ss << ", ";
                    const auto& pair = seq_items(items[idx].get_if<garuda::beam::Tuple>()->items);
                    std::string key = pair[0].get_if<std::string>() ? *pair[0].get_if<std::string>() : to_string(pair[0]);
                    ss << escape_string(key) << ": " << encode(pair[1]);
                }
                ss << "}";
                return ss.str();
            } else {
                std::ostringstream ss;
                ss << "[";
                for (std::size_t idx = 0; idx < items.size(); ++idx) {
                    if (idx > 0) ss << ", ";
                    ss << encode(items[idx]);
                }
                ss << "]";
                return ss.str();
            }
        }
        if (auto* tup = term.get_if<garuda::beam::Tuple>()) {
            const auto& items = seq_items(tup->items);
            std::ostringstream ss;
            ss << "[";
            for (std::size_t idx = 0; idx < items.size(); ++idx) {
                if (idx > 0) ss << ", ";
                ss << encode(items[idx]);
            }
            ss << "]";
            return ss.str();
        }
        return "null";
    }

    // ---- PARSER ----
    static std::optional<Term> decode(const std::string& json_str) {
        std::size_t idx = 0;
        skip_whitespace(json_str, idx);
        if (idx >= json_str.length()) return std::nullopt;

        try {
            Term result = parse_value(json_str, idx);
            skip_whitespace(json_str, idx);
            if (idx != json_str.length()) return std::nullopt; // Unparsed trailing tokens!
            return result;
        } catch (...) {
            return std::nullopt;
        }
    }

private:
    static std::string escape_string(const std::string& s) {
        std::ostringstream ss;
        ss << '"';
        for (char c : s) {
            switch (c) {
                case '"': ss << "\\\""; break;
                case '\\': ss << "\\\\"; break;
                case '\b': ss << "\\b"; break;
                case '\f': ss << "\\f"; break;
                case '\n': ss << "\\n"; break;
                case '\r': ss << "\\r"; break;
                case '\t': ss << "\\t"; break;
                default:
                    if (static_cast<unsigned char>(c) < 0x20) {
                        ss << "\\u00" << (static_cast<int>(c) < 16 ? "0" : "") << std::hex << static_cast<int>(c);
                    } else {
                        ss << c;
                    }
            }
        }
        ss << '"';
        return ss.str();
    }

    static void skip_whitespace(const std::string& s, std::size_t& idx) {
        while (idx < s.length() && (s[idx] == ' ' || s[idx] == '\t' || s[idx] == '\n' || s[idx] == '\r')) {
            idx++;
        }
    }

    static Term parse_value(const std::string& s, std::size_t& idx) {
        skip_whitespace(s, idx);
        if (idx >= s.length()) throw std::runtime_error("Unexpected EOF");

        char c = s[idx];
        if (c == '{') return parse_object(s, idx);
        if (c == '[') return parse_array(s, idx);
        if (c == '"') return parse_string(s, idx);
        if (c == 't' || c == 'f') return parse_boolean(s, idx);
        if (c == 'n') return parse_null(s, idx);
        if (c == '-' || std::isdigit(static_cast<unsigned char>(c))) return parse_number(s, idx);

        throw std::runtime_error("Invalid JSON value token");
    }

    static std::string parse_string(const std::string& s, std::size_t& idx) {
        if (s[idx] != '"') throw std::runtime_error("Expected quote");
        idx++; // Skip opening quote
        std::string res;

        while (idx < s.length()) {
            char c = s[idx++];
            if (c == '"') return res; // Closing quote
            if (c == '\\') {
                if (idx >= s.length()) throw std::runtime_error("Unterminated escape sequence");
                char esc = s[idx++];
                switch (esc) {
                    case '"': res += '"'; break;
                    case '\\': res += '\\'; break;
                    case '/': res += '/'; break;
                    case 'b': res += '\b'; break;
                    case 'f': res += '\f'; break;
                    case 'n': res += '\n'; break;
                    case 'r': res += '\r'; break;
                    case 't': res += '\t'; break;
                    default: res += esc; break;
                }
            } else {
                res += c;
            }
        }
        throw std::runtime_error("Unterminated JSON string");
    }

    static Term parse_number(const std::string& s, std::size_t& idx) {
        std::size_t start = idx;
        if (s[idx] == '-') idx++;
        bool is_float = false;

        while (idx < s.length() && (std::isdigit(static_cast<unsigned char>(s[idx])) || s[idx] == '.' || s[idx] == 'e' || s[idx] == 'E' || s[idx] == '+' || s[idx] == '-')) {
            if (s[idx] == '.' || s[idx] == 'e' || s[idx] == 'E') is_float = true;
            idx++;
        }

        std::string num_str = s.substr(start, idx - start);
        if (is_float) {
            return Term(std::stod(num_str));
        } else {
            return Term(static_cast<std::int64_t>(std::stoll(num_str)));
        }
    }

    static Term parse_object(const std::string& s, std::size_t& idx) {
        idx++; // Skip '{'
        std::vector<Term> pairs;

        skip_whitespace(s, idx);
        if (idx < s.length() && s[idx] == '}') {
            idx++; // Empty object {}
            return list_of(pairs);
        }

        while (idx < s.length()) {
            skip_whitespace(s, idx);
            if (s[idx] != '"') throw std::runtime_error("Expected string key in object");

            std::string key = parse_string(s, idx);
            skip_whitespace(s, idx);

            if (idx >= s.length() || s[idx] != ':') throw std::runtime_error("Expected ':' after key");
            idx++; // Skip ':'

            Term val = parse_value(s, idx);
            pairs.push_back(tuple_of({Term(key), val}));

            skip_whitespace(s, idx);
            if (idx < s.length() && s[idx] == '}') {
                idx++; // Closing '}'
                return list_of(pairs);
            }
            if (idx < s.length() && s[idx] == ',') {
                idx++; // Skip ','
                continue;
            }
            throw std::runtime_error("Expected ',' or '}' in object");
        }
        throw std::runtime_error("Unterminated object");
    }

    static Term parse_array(const std::string& s, std::size_t& idx) {
        idx++; // Skip '['
        std::vector<Term> arr;

        skip_whitespace(s, idx);
        if (idx < s.length() && s[idx] == ']') {
            idx++; // Empty array []
            return list_of(arr);
        }

        while (idx < s.length()) {
            Term val = parse_value(s, idx);
            arr.push_back(val);

            skip_whitespace(s, idx);
            if (idx < s.length() && s[idx] == ']') {
                idx++; // Closing ']'
                return list_of(arr);
            }
            if (idx < s.length() && s[idx] == ',') {
                idx++; // Skip ','
                continue;
            }
            throw std::runtime_error("Expected ',' or ']' in array");
        }
        throw std::runtime_error("Unterminated array");
    }

    static Term parse_boolean(const std::string& s, std::size_t& idx) {
        if (s.compare(idx, 4, "true") == 0) {
            idx += 4;
            return Term(Atom{"true"});
        }
        if (s.compare(idx, 5, "false") == 0) {
            idx += 5;
            return Term(Atom{"false"});
        }
        throw std::runtime_error("Invalid boolean literal");
    }

    static Term parse_null(const std::string& s, std::size_t& idx) {
        if (s.compare(idx, 4, "null") == 0) {
            idx += 4;
            return Term(Atom{"nil"});
        }
        throw std::runtime_error("Invalid null literal");
    }
};

} // namespace garuda::beam::stdlib
