#pragma once

#include "../core/term.hpp"
#include <iostream>
#include <string>
#include <unordered_map>
#include <shared_mutex>
#include <mutex>
#include <memory>
#include <vector>
#include <atomic>

namespace garuda::beam::stdlib {

class EtsTable {
public:
    EtsTable(std::int64_t id) : id_(id) {}

    std::int64_t id() const { return id_; }

    void insert(const std::string& key, const Term& val) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        table_[key] = val;
    }

    Term lookup(const std::string& key) const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        auto it = table_.find(key);
        if (it != table_.end()) {
            return it->second;
        }
        return atom("nil");
    }

    bool erase(const std::string& key) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        return table_.erase(key) > 0;
    }

    Term all() const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        std::vector<Term> items;
        items.reserve(table_.size());
        for (const auto& [k, v] : table_) {
            items.push_back(tuple({Term(k), v}));
        }
        return list_of(std::move(items));
    }

    size_t size() const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return table_.size();
    }

private:
    std::int64_t id_;
    std::unordered_map<std::string, Term> table_;
    mutable std::shared_mutex mutex_;
};

class EtsManager {
public:
    static EtsManager& instance() {
        static EtsManager mgr;
        return mgr;
    }

    std::int64_t create_table() {
        std::int64_t tid = next_id_++;
        std::unique_lock<std::shared_mutex> lock(mutex_);
        tables_[tid] = std::make_shared<EtsTable>(tid);
        return tid;
    }

    std::shared_ptr<EtsTable> get_table(std::int64_t tid) {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        auto it = tables_.find(tid);
        if (it != tables_.end()) {
            return it->second;
        }
        return nullptr;
    }

private:
    EtsManager() : next_id_(1) {}
    std::atomic<std::int64_t> next_id_;
    std::unordered_map<std::int64_t, std::shared_ptr<EtsTable>> tables_;
    mutable std::shared_mutex mutex_;
};

} // namespace garuda::beam::stdlib
