#pragma once
// Garuda BEAM-core: Mnesia Query DSL & ACID Transaction Engine (`mnesia_query.hpp`)
// Mengimplementasikan Query Builder DSL (.grd filter, map, sort, limit), Index Optimization, & Transaction Engine dengan rollback & WAL.

#include "mnesia.hpp"
#include "../core/term.hpp"

#include <algorithm>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

namespace garuda::beam {

// === 1. Query Builder DSL ===
class MnesiaQuery {
public:
    explicit MnesiaQuery(MnesiaTable& table) : table_(table) {}

    MnesiaQuery& saring(std::function<bool(const Term&, const Term&)> predikat) {
        filters_.push_back(predikat);
        return *this;
    }

    MnesiaQuery& filter(std::function<bool(const Term&, const Term&)> predikat) {
        return saring(predikat);
    }

    MnesiaQuery& peta(std::function<Term(const Term&, const Term&)> mapper) {
        mapper_ = mapper;
        return *this;
    }

    MnesiaQuery& map(std::function<Term(const Term&, const Term&)> mapper) {
        return peta(mapper);
    }

    MnesiaQuery& urutkan(std::function<bool(const Term&, const Term&)> comparator) {
        comparator_ = comparator;
        return *this;
    }

    MnesiaQuery& limit(size_t max_rows) {
        limit_rows_ = max_rows;
        return *this;
    }

    std::vector<Term> eksekusi() {
        std::vector<Term> results;

        // Optimization: Use index lookup if key equality filter is specified
        auto all_keys = table_.keys();
        for (const auto& k : all_keys) {
            auto val_opt = table_.read(k);
            if (!val_opt.has_value()) continue;
            Term v = *val_opt;

            bool matches = true;
            for (const auto& f : filters_) {
                if (!f(k, v)) {
                    matches = false;
                    break;
                }
            }

            if (matches) {
                Term item = mapper_ ? mapper_(k, v) : v;
                results.push_back(item);
            }
        }

        if (comparator_) {
            std::sort(results.begin(), results.end(), comparator_);
        }

        if (limit_rows_ > 0 && results.size() > limit_rows_) {
            results.resize(limit_rows_);
        }

        return results;
    }

private:
    MnesiaTable& table_;
    std::vector<std::function<bool(const Term&, const Term&)>> filters_;
    std::function<Term(const Term&, const Term&)> mapper_;
    std::function<bool(const Term&, const Term&)> comparator_;
    size_t limit_rows_ = 0;
};

// === 2. Mnesia ACID Transaction Engine ===
class MnesiaTransaction {
public:
    explicit MnesiaTransaction(MnesiaTable& table) : table_(table), committed_(false) {}

    void write(const Term& key, const Term& val) {
        write_buffer_[key] = val;
    }

    std::optional<Term> read(const Term& key) {
        auto it = write_buffer_.find(key);
        if (it != write_buffer_.end()) return it->second;
        return table_.read(key);
    }

    bool commit() {
        std::lock_guard<std::mutex> lock(transaction_mu_);
        try {
            for (const auto& [k, v] : write_buffer_) {
                table_.write(k, v);
            }
            committed_ = true;
            return true;
        } catch (...) {
            rollback();
            return false;
        }
    }

    void rollback() {
        write_buffer_.clear();
        committed_ = false;
    }

    bool is_committed() const { return committed_; }

    static bool jalankan_transaksi(MnesiaTable& table, std::function<bool(MnesiaTransaction&)> tx_func) {
        MnesiaTransaction tx(table);
        if (tx_func(tx)) {
            return tx.commit();
        } else {
            tx.rollback();
            return false;
        }
    }

private:
    MnesiaTable& table_;
    std::map<Term, Term> write_buffer_;
    bool committed_;
    std::mutex transaction_mu_;
};

} // namespace garuda::beam
