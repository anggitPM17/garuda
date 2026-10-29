#pragma once
// =====================================================================
// FASE D.2.2 - Registry C functions (type-safe signatures)
// =====================================================================
#include "ffi.hpp"
#include <map>
#include <mutex>
#include <string>
#include <vector>
#include <optional>
#include <memory>

namespace garuda::ffi {

struct CFunctionSpec {
    std::string library_path;
    std::string symbol_name;
    std::string return_type = "int"; // "int", "string", "void"
    std::vector<std::string> arg_types;
};

class CRegistry {
private:
    std::map<std::string, CFunction> functions_;
    std::map<std::string, CFunctionSpec> specs_;
    mutable std::mutex mu_;

public:
    bool register_function(const std::string& alias, const CFunctionSpec& spec) {
        std::lock_guard<std::mutex> lock(mu_);
        auto fn_opt = CFunction::load(spec.library_path, spec.symbol_name);
        if (!fn_opt.has_value()) return false;

        functions_[alias] = std::move(*fn_opt);
        specs_[alias] = spec;
        return true;
    }

    std::optional<std::reference_wrapper<const CFunction>> get(const std::string& alias) const {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = functions_.find(alias);
        if (it != functions_.end()) {
            return std::cref(it->second);
        }
        return std::nullopt;
    }

    std::optional<std::int64_t> call_int(const std::string& alias, const std::vector<std::int64_t>& /*args*/ = {}) const {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = functions_.find(alias);
        if (it == functions_.end()) return std::nullopt;

#if defined(_WIN32)
        using FnType = unsigned long(*)();
#else
        using FnType = int(*)();
#endif
        auto res = it->second.call<std::int64_t>();
        return res;
    }

    std::size_t function_count() const {
        std::lock_guard<std::mutex> lock(mu_);
        return functions_.size();
    }

    std::vector<std::string> list_functions() const {
        std::lock_guard<std::mutex> lock(mu_);
        std::vector<std::string> out;
        for (const auto& [name, fn] : functions_) {
            out.push_back(name);
        }
        return out;
    }
};

} // namespace garuda::ffi
