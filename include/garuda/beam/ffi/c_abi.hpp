#pragma once
// Garuda FFI (Foreign Function Interface): C ABI & Function Registry (`c_abi.hpp`)
#include "../core/term.hpp"

#include <cstdint>
#include <functional>
#include <iostream>
#include <map>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

extern "C" {

using GrdInt = std::int64_t;
using GrdFloat = double;
using GrdString = const char*;
using GrdBool = std::uint8_t;

enum class GrdType : std::uint8_t {
    Null = 0,
    Int = 1,
    Float = 2,
    String = 3,
    Bool = 4
};

struct GrdValue {
    GrdType type = GrdType::Null;
    GrdInt int_val = 0;
    GrdFloat float_val = 0.0;
    GrdString str_val = nullptr;
    GrdBool bool_val = 0;

    static GrdValue make_null() {
        GrdValue v;
        v.type = GrdType::Null;
        return v;
    }

    static GrdValue make_int(GrdInt i) {
        GrdValue v;
        v.type = GrdType::Int;
        v.int_val = i;
        return v;
    }

    static GrdValue make_float(GrdFloat f) {
        GrdValue v;
        v.type = GrdType::Float;
        v.float_val = f;
        return v;
    }

    static GrdValue make_string(GrdString s) {
        GrdValue v;
        v.type = GrdType::String;
        v.str_val = s;
        return v;
    }

    static GrdValue make_bool(GrdBool b) {
        GrdValue v;
        v.type = GrdType::Bool;
        v.bool_val = b ? 1 : 0;
        return v;
    }
};

typedef GrdValue (*GrdCFunction)(const GrdValue* args, int argc);

} // extern "C"

namespace garuda::beam {

inline GrdValue term_to_grd_value(const Term& term) {
    if (auto* i = term.get_if<std::int64_t>()) {
        return GrdValue::make_int(*i);
    }
    if (auto* f = term.get_if<double>()) {
        return GrdValue::make_float(*f);
    }
    if (auto* s = term.get_if<std::string>()) {
        return GrdValue::make_string(s->c_str());
    }
    if (auto* a = term.get_if<Atom>()) {
        if (a->name == "true") return GrdValue::make_bool(1);
        if (a->name == "false") return GrdValue::make_bool(0);
        return GrdValue::make_string(a->name.c_str());
    }
    return GrdValue::make_null();
}

inline Term grd_value_to_term(const GrdValue& val) {
    switch (val.type) {
        case GrdType::Int:
            return Term(val.int_val);
        case GrdType::Float:
            return Term(val.float_val);
        case GrdType::String:
            return Term(val.str_val ? val.str_val : "");
        case GrdType::Bool:
            return Term(Atom{val.bool_val != 0 ? "true" : "false"});
        case GrdType::Null:
        default:
            return Term(Atom{"nil"});
    }
}

class CFfiRegistry {
public:
    static CFfiRegistry& instance() {
        static CFfiRegistry inst;
        return inst;
    }

    void register_func(const std::string& name, GrdCFunction func) {
        std::lock_guard<std::mutex> lock(mu_);
        functions_[name] = func;
    }

    bool has_func(const std::string& name) const {
        std::lock_guard<std::mutex> lock(mu_);
        return functions_.count(name) > 0;
    }

    std::optional<Term> call_func(const std::string& name, const std::vector<Term>& args) const {
        GrdCFunction fn = nullptr;
        {
            std::lock_guard<std::mutex> lock(mu_);
            auto it = functions_.find(name);
            if (it == functions_.end() || !it->second) {
                return std::nullopt;
            }
            fn = it->second;
        }

        std::vector<GrdValue> c_args;
        c_args.reserve(args.size());
        for (const auto& arg : args) {
            c_args.push_back(term_to_grd_value(arg));
        }

        GrdValue c_res = fn(c_args.data(), static_cast<int>(c_args.size()));
        return grd_value_to_term(c_res);
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mu_);
        functions_.clear();
    }

private:
    CFfiRegistry() = default;

    mutable std::mutex mu_;
    std::map<std::string, GrdCFunction> functions_;
};

} // namespace garuda::beam
