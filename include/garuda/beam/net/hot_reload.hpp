#pragma once
// Garuda Distributed Engine: Hot Code Reloading & Zero-Downtime Upgrade (`hot_reload.hpp`)
// Mengimplementasikan pembaruan kode modul biner .gvm di memori secara live tanpa restart VM.
#include "../core/code_server.hpp"
#include "../gvm/interpreter.hpp"

#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

namespace garuda::beam {

struct LoadedModule {
    std::string name;
    std::uint32_t version = 1;
    std::vector<Instruction> instructions;
    std::map<std::string, std::uint64_t> exported_functions;
};

class HotReload {
public:
    HotReload() = default;

    std::uint32_t load_module(const std::string& name,
                              std::vector<Instruction> instructions,
                              std::map<std::string, std::uint64_t> exports) {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = current_modules_.find(name);
        if (it == current_modules_.end()) {
            LoadedModule mod;
            mod.name = name;
            mod.version = 1;
            mod.instructions = std::move(instructions);
            mod.exported_functions = std::move(exports);
            current_modules_[name] = mod;
            std::cout << "📦 [GARUDA HOT RELOAD] Loaded new module '" << name << "' v1 in memory." << std::endl;
            return 1;
        }
        return it->second.version;
    }

    std::optional<LoadedModule> upgrade_module(const std::string& name,
                                                std::vector<Instruction> new_instructions,
                                                std::map<std::string, std::uint64_t> new_exports) {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = current_modules_.find(name);
        if (it == current_modules_.end()) {
            // First load
            LoadedModule mod;
            mod.name = name;
            mod.version = 1;
            mod.instructions = std::move(new_instructions);
            mod.exported_functions = std::move(new_exports);
            current_modules_[name] = mod;
            return std::nullopt;
        }

        // Preserve old version in grace period
        old_modules_[name] = it->second;
        std::uint32_t new_version = it->second.version + 1;

        LoadedModule new_mod;
        new_mod.name = name;
        new_mod.version = new_version;
        new_mod.instructions = std::move(new_instructions);
        new_mod.exported_functions = std::move(new_exports);
        current_modules_[name] = new_mod;

        std::cout << "🔥 [GARUDA HOT RELOAD] Live upgraded module '" << name << "' v"
                  << (new_version - 1) << " -> v" << new_version << " (Zero Downtime)!" << std::endl;

        return old_modules_[name];
    }

    std::optional<LoadedModule> get_module(const std::string& name) const {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = current_modules_.find(name);
        if (it != current_modules_.end()) return it->second;
        return std::nullopt;
    }

    std::optional<LoadedModule> get_old_module(const std::string& name) const {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = old_modules_.find(name);
        if (it != old_modules_.end()) return it->second;
        return std::nullopt;
    }

    bool purge_old(const std::string& name) {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = old_modules_.find(name);
        if (it != old_modules_.end()) {
            old_modules_.erase(it);
            std::cout << "🧹 [GARUDA HOT RELOAD] Purged old version of module '" << name << "'" << std::endl;
            return true;
        }
        return false;
    }

    std::uint32_t get_version(const std::string& name) const {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = current_modules_.find(name);
        if (it != current_modules_.end()) return it->second.version;
        return 0;
    }

    std::vector<std::string> list_modules() const {
        std::lock_guard<std::mutex> lock(mu_);
        std::vector<std::string> list;
        for (const auto& [name, mod] : current_modules_) {
            list.push_back(name + " (v" + std::to_string(mod.version) + ")");
        }
        return list;
    }

private:
    mutable std::mutex mu_;
    std::map<std::string, LoadedModule> current_modules_;
    std::map<std::string, LoadedModule> old_modules_;
};

} // namespace garuda::beam
