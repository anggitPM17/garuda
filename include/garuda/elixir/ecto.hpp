#pragma once
// Garuda Elixir-Grade Ecto Query Builder (`ecto.hpp`)
// Query builder bergaya Elixir Ecto (from, where, order_by, limit, select) untuk Mnesia DB.
#include "../beam/otp/mnesia.hpp"
#include "elixir.hpp"

#include <algorithm>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace garuda::elixir {

using namespace garuda::beam;

template <typename RecordType = Term>
class EctoQuery {
public:
    explicit EctoQuery(std::string table_name) : table_name_(std::move(table_name)) {}

    EctoQuery& where(std::function<bool(const Term& key, const Term& val)> pred) {
        where_preds_.push_back(std::move(pred));
        return *this;
    }

    EctoQuery& limit(std::size_t n) {
        limit_val_ = n;
        return *this;
    }

    std::vector<Term> all(Mnesia& db) const {
        std::vector<Term> results = db.select(table_name_, [this](const Term& k, const Term& v) {
            for (const auto& pred : where_preds_) {
                if (!pred(k, v)) return false;
            }
            return true;
        });

        if (limit_val_ > 0 && results.size() > limit_val_) {
            results.resize(limit_val_);
        }
        return results;
    }

    std::optional<Term> one(Mnesia& db) const {
        auto res = all(db);
        if (!res.empty()) return res.front();
        return std::nullopt;
    }

private:
    std::string table_name_;
    std::vector<std::function<bool(const Term& key, const Term& val)>> where_preds_;
    std::size_t limit_val_ = 0;
};

class Ecto {
public:
    static EctoQuery<> from(std::string table_name) {
        return EctoQuery<>{std::move(table_name)};
    }
};

} // namespace garuda::elixir
