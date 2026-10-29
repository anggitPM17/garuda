#pragma once
// =====================================================================
// FASE D.2.4 - DSL: panggil_c keyword untuk Garuda script
// =====================================================================
#include "registry.hpp"
#include <string>
#include <vector>
#include <sstream>
#include <memory>
#include <optional>

namespace garuda::ffi {

class Dsl {
private:
    std::shared_ptr<CRegistry> registry_;

public:
    explicit Dsl(std::shared_ptr<CRegistry> registry)
        : registry_(std::move(registry)) {}

    std::optional<std::string> execute(const std::string& line) {
        if (line.empty() || !registry_) return std::nullopt;

        std::stringstream ss(line);
        std::string keyword;
        ss >> keyword;

        if (keyword != "panggil_c") return std::nullopt;

        std::string alias;
        ss >> alias;
        if (alias.empty()) return std::nullopt;

        auto fn_ref = registry_->get(alias);
        if (!fn_ref.has_value()) return std::nullopt;

        // Execute function via registry
        auto res = registry_->call_int(alias);
        if (res.has_value()) {
            return std::to_string(*res);
        }

        return std::nullopt;
    }

    std::vector<std::string> execute_script(const std::string& script) {
        std::vector<std::string> results;
        std::stringstream ss(script);
        std::string line;

        while (std::getline(ss, line)) {
            auto res = execute(line);
            if (res.has_value()) {
                results.push_back(*res);
            }
        }
        return results;
    }
};

} // namespace garuda::ffi
