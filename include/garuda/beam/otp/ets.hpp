#pragma once
// ETS-lite: tabel in-memory berkunci Term (ordered_set dan bag), single-thread.
#include "../core/term.hpp"

#include <functional>
#include <map>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace garuda::beam {

class OrderedSet {
public:
    void insert(Term key, Term value) { m_[std::move(key)] = std::move(value); }
    std::optional<Term> lookup(const Term& key) const {
        auto it = m_.find(key);
        if (it == m_.end()) return std::nullopt;
        return it->second;
    }
    bool erase(const Term& key) { return m_.erase(key) > 0; }
    bool contains(const Term& key) const { return m_.count(key) > 0; }
    std::size_t size() const { return m_.size(); }
    void clear() { m_.clear(); }
    std::int64_t update_counter(const Term& key, std::int64_t delta, std::int64_t dflt = 0) {
        auto it = m_.find(key);
        std::int64_t cur = dflt;
        if (it != m_.end()) { if (auto* i = it->second.get_if<std::int64_t>()) cur = *i; }
        cur += delta;
        m_[key] = Term(cur);
        return cur;
    }
    std::optional<Term> first() const { return m_.empty() ? std::nullopt : std::optional<Term>(m_.begin()->first); }
    std::optional<Term> next(const Term& key) const {
        auto it = m_.upper_bound(key);
        return it == m_.end() ? std::nullopt : std::optional<Term>(it->first);
    }
    std::vector<std::pair<Term, Term>> select(const std::function<bool(const Term&, const Term&)>& pred) const {
        std::vector<std::pair<Term, Term>> out;
        for (auto& [k, v] : m_) if (pred(k, v)) out.emplace_back(k, v);
        return out;
    }
private:
    std::map<Term, Term, TermLess> m_;
};

class Bag {
public:
    void insert(Term key, Term value) { m_.emplace(std::move(key), std::move(value)); }
    std::vector<Term> lookup(const Term& key) const {
        std::vector<Term> out;
        auto r = m_.equal_range(key);
        for (auto it = r.first; it != r.second; ++it) out.push_back(it->second);
        return out;
    }
    std::size_t erase(const Term& key) { return m_.erase(key); }
    std::size_t size() const { return m_.size(); }
private:
    std::multimap<Term, Term, TermLess> m_;
};

class Ets {
public:
    OrderedSet& ordered_set(const std::string& name) { return sets_[name]; }
    Bag& bag(const std::string& name) { return bags_[name]; }
    bool drop(const std::string& name) { return sets_.erase(name) + bags_.erase(name) > 0; }
private:
    std::unordered_map<std::string, OrderedSet> sets_;
    std::unordered_map<std::string, Bag> bags_;
};

}  // namespace garuda::beam
