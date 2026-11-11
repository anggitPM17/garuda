#pragma once
#include <string>
#include <chrono>
#include <optional>
#include <unordered_map>
#include <vector>

namespace garuda::jalur_langit {
    struct QueryResult {
        bool success{true};
        std::int64_t affected_rows{0};
        std::vector<std::string> rows;
    };

    class QueryCache {
    public:
        explicit QueryCache(std::chrono::seconds ttl) : ttl_(ttl) {}

        void put(const std::string& query, const QueryResult& res) {
            cache_[query] = {res, std::chrono::steady_clock::now()};
        }

        std::optional<QueryResult> get(const std::string& query) {
            auto it = cache_.find(query);
            if (it == cache_.end()) return std::nullopt;
            if (std::chrono::steady_clock::now() - it->second.timestamp > ttl_) {
                cache_.erase(it);
                return std::nullopt;
            }
            return it->second.result;
        }

    private:
        struct Entry {
            QueryResult result;
            std::chrono::steady_clock::time_point timestamp;
        };
        std::chrono::seconds ttl_;
        std::unordered_map<std::string, Entry> cache_;
    };
}
