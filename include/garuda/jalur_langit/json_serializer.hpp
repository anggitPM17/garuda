#pragma once
#include <string>
#include <map>
#include <variant>
#include <vector>
#include <sstream>
#include "query_cache.hpp"

namespace garuda::jalur_langit {
    struct Row {
        std::map<std::string, std::variant<std::int64_t, std::string, bool, double>> columns;
    };

    class JsonSerializer {
    public:
        static std::string serialize_row(const Row& row) {
            std::ostringstream ss;
            ss << "{";
            bool first = true;
            for (const auto& [k, v] : row.columns) {
                if (!first) ss << ",";
                first = false;
                ss << "\"" << k << "\":";
                std::visit([&ss](auto&& arg) {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, std::string>) ss << "\"" << arg << "\"";
                    else if constexpr (std::is_same_v<T, bool>) ss << (arg ? "true" : "false");
                    else ss << arg;
                }, v);
            }
            ss << "}";
            return ss.str();
        }

        static std::string serialize_result(const QueryResult& res) {
            std::ostringstream ss;
            ss << "{\"success\":" << (res.success ? "true" : "false") << "}";
            return ss.str();
        }
    };
}
