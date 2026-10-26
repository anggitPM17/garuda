#pragma once
// Garuda Elixir-Grade Abstraction Engine (`elixir.hpp`)
// Keyword Lists, Structs (%User{}), Enum (map, filter, reduce, reject), Pipe Operator (|>).
#include "../beam/core/term.hpp"

#include <algorithm>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <numeric>
#include <string>
#include <vector>

namespace garuda::elixir {

using namespace garuda::beam;

// Keyword List ([key: "value", atom: 123])
struct KeywordPair {
    std::string key;
    Term value;
};

class KeywordList {
public:
    KeywordList() = default;
    KeywordList(std::initializer_list<KeywordPair> pairs) : pairs_(pairs) {}

    void put(std::string key, Term val) {
        for (auto& p : pairs_) {
            if (p.key == key) { p.value = std::move(val); return; }
        }
        pairs_.push_back({std::move(key), std::move(val)});
    }

    std::optional<Term> get(const std::string& key) const {
        for (const auto& p : pairs_) {
            if (p.key == key) return p.value;
        }
        return std::nullopt;
    }

    bool has_key(const std::string& key) const {
        for (const auto& p : pairs_) {
            if (p.key == key) return true;
        }
        return false;
    }

    const std::vector<KeywordPair>& pairs() const { return pairs_; }

private:
    std::vector<KeywordPair> pairs_;
};

// Elixir Struct (%User{name: "Anggit", role: :admin})
class Struct {
public:
    explicit Struct(std::string struct_name) : name_(std::move(struct_name)) {}

    const std::string& struct_name() const { return name_; }

    void set(std::string field, Term val) {
        fields_[std::move(field)] = std::move(val);
    }

    std::optional<Term> get(const std::string& field) const {
        auto it = fields_.find(field);
        if (it != fields_.end()) return it->second;
        return std::nullopt;
    }

    std::string to_string() const {
        std::ostringstream ss;
        ss << "%" << name_ << "{";
        bool first = true;
        for (const auto& [k, v] : fields_) {
            if (!first) ss << ", ";
            ss << k << ": " << garuda::beam::to_string(v);
            first = false;
        }
        ss << "}";
        return ss.str();
    }

private:
    std::string name_;
    std::map<std::string, Term> fields_;
};

// Elixir Enum Module (Enum.map, Enum.filter, Enum.reduce)
class Enum {
public:
    template <typename T, typename Func>
    static auto map(const std::vector<T>& vec, Func fn) {
        using ResultType = decltype(fn(vec[0]));
        std::vector<ResultType> out;
        out.reserve(vec.size());
        for (const auto& item : vec) {
            out.push_back(fn(item));
        }
        return out;
    }

    template <typename T, typename Pred>
    static std::vector<T> filter(const std::vector<T>& vec, Pred pred) {
        std::vector<T> out;
        for (const auto& item : vec) {
            if (pred(item)) out.push_back(item);
        }
        return out;
    }

    template <typename T, typename Pred>
    static std::vector<T> reject(const std::vector<T>& vec, Pred pred) {
        std::vector<T> out;
        for (const auto& item : vec) {
            if (!pred(item)) out.push_back(item);
        }
        return out;
    }

    template <typename T, typename Acc, typename Func>
    static Acc reduce(const std::vector<T>& vec, Acc initial, Func fn) {
        Acc acc = initial;
        for (const auto& item : vec) {
            acc = fn(item, acc);
        }
        return acc;
    }

    static std::string join(const std::vector<std::string>& vec, const std::string& sep = "") {
        std::ostringstream ss;
        for (std::size_t i = 0; i < vec.size(); ++i) {
            if (i > 0) ss << sep;
            ss << vec[i];
        }
        return ss.str();
    }
};

// C++ Fluent Pipeline (Simulasi Elixir |> Pipe Operator)
template <typename T>
class Pipe {
public:
    explicit Pipe(T val) : val_(std::move(val)) {}

    template <typename Func>
    auto pipe(Func fn) const {
        return Pipe<decltype(fn(val_))>(fn(val_));
    }

    const T& value() const { return val_; }

private:
    T val_;
};

template <typename T>
inline Pipe<T> make_pipe(T val) {
    return Pipe<T>(std::move(val));
}

} // namespace garuda::elixir
