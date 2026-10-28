#pragma once

#include "jalur_langit_ultimate.hpp"
#include <string>
#include <sstream>
#include <iomanip>
#include <variant>

namespace garuda::jalur_langit {

class JsonSerializer {
public:
    static std::string escape_string(const std::string& str) {
        std::ostringstream ss;
        for (char c : str) {
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
                        ss << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(c);
                    } else {
                        ss << c;
                    }
                    break;
            }
        }
        return ss.str();
    }

    static std::string serialize_value(const Value& val) {
        return std::visit([](auto&& arg) -> std::string {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, std::monostate>) {
                return "null";
            } else if constexpr (std::is_same_v<T, std::int64_t>) {
                return std::to_string(arg);
            } else if constexpr (std::is_same_v<T, double>) {
                std::ostringstream ss;
                ss << arg;
                return ss.str();
            } else if constexpr (std::is_same_v<T, bool>) {
                return arg ? "true" : "false";
            } else if constexpr (std::is_same_v<T, std::string>) {
                return "\"" + escape_string(arg) + "\"";
            } else {
                return "null";
            }
        }, val);
    }

    // Konversi satu Row ke JSON object string
    static std::string serialize_row(const Row& row) {
        std::ostringstream ss;
        ss << "{";
        bool first = true;
        for (const auto& [k, v] : row.columns) {
            if (!first) ss << ",";
            first = false;
            ss << "\"" << escape_string(k) << "\":" << serialize_value(v);
        }
        ss << "}";
        return ss.str();
    }

    // Konversi QueryResult ke JSON string
    static std::string serialize_result(const QueryResult& result) {
        std::ostringstream ss;
        ss << "{";
        ss << "\"success\":" << (result.success ? "true" : "false") << ",";
        ss << "\"error_message\":\"" << escape_string(result.error_message) << "\",";
        ss << "\"affected_rows\":" << result.affected_rows << ",";
        ss << "\"rows\":[";
        for (size_t i = 0; i < result.rows.size(); ++i) {
            if (i > 0) ss << ",";
            ss << serialize_row(result.rows[i]);
        }
        ss << "]}";
        return ss.str();
    }
};

} // namespace garuda::jalur_langit
