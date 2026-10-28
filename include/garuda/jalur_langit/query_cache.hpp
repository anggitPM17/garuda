#pragma once

#include "jalur_langit_ultimate.hpp"
#include <unordered_map>
#include <mutex>
#include <string>
#include <chrono>
#include <optional>

namespace garuda::jalur_langit {

struct CacheEntry {
    QueryResult result;
    std::chrono::steady_clock::time_point timestamp;
};

class QueryCache {
private:
    std::unordered_map<std::string, CacheEntry> cache_;
    mutable std::mutex mutex_;
    std::chrono::seconds ttl_;

public:
    explicit QueryCache(std::chrono::seconds ttl = std::chrono::seconds(60))
        : ttl_(ttl) {}

    // Ambil hasil dari cache (nullopt jika tidak ada / expired)
    std::optional<QueryResult> get(const std::string& query) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = cache_.find(query);
        if (it == cache_.end()) {
            return std::nullopt;
        }

        auto now = std::chrono::steady_clock::now();
        if (now - it->second.timestamp >= ttl_) {
            cache_.erase(it);
            return std::nullopt;
        }

        return it->second.result;
    }

    // Simpan hasil ke cache
    void put(const std::string& query, const QueryResult& result) {
        std::lock_guard<std::mutex> lock(mutex_);
        cache_[query] = CacheEntry{result, std::chrono::steady_clock::now()};
    }

    // Hapus semua cache
    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        cache_.clear();
    }
};

} // namespace garuda::jalur_langit
