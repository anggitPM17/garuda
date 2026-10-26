#pragma once
// Garuda BEAM-Core: Hot Code Reloading & Dynamic Code Server (`code_server.hpp`)
// Mengimplementasikan fitur DEWA Erlang BEAM: memperbarui kode module secara live tanpa mematikan proses / memutuskan WebSocket!
#include "runtime.hpp"
#include "term.hpp"

#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace garuda::beam {

struct ModuleVersion {
    std::string module_name;
    std::uint32_t version = 1;
    std::map<std::string, Body> functions;
};

class CodeServer {
public:
    static CodeServer& instance() {
        static CodeServer inst;
        return inst;
    }

    void load_module(std::string module_name, std::map<std::string, Body> funcs) {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = current_versions_.find(module_name);
        if (it != current_versions_.end()) {
            // Simpan versi lama ke old_versions_ untuk soft_purge
            old_versions_[module_name] = it->second;
            std::uint32_t new_ver = it->second.version + 1;
            current_versions_[module_name] = ModuleVersion{module_name, new_ver, std::move(funcs)};
            std::cout << "🔥 [GARUDA HOT CODE RELOAD] Module '" << module_name << "' upgraded live to version v" << new_ver << " (Zero Downtime)!" << std::endl;
        } else {
            current_versions_[module_name] = ModuleVersion{module_name, 1, std::move(funcs)};
            std::cout << "📦 [GARUDA CODE SERVER] Module '" << module_name << "' v1 loaded." << std::endl;
        }
    }

    std::optional<Body> find_function(const std::string& module_name, const std::string& func_name) {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = current_versions_.find(module_name);
        if (it == current_versions_.end()) return std::nullopt;

        auto f_it = it->second.functions.find(func_name);
        if (f_it == it->second.functions.end()) return std::nullopt;
        return f_it->second;
    }

    bool purge_old_version(const std::string& module_name) {
        std::lock_guard<std::mutex> lock(mu_);
        bool erased = old_versions_.erase(module_name) > 0;
        if (erased) {
            std::cout << "🧹 [GARUDA CODE SERVER] Purged old code version for module: " << module_name << std::endl;
        }
        return erased;
    }

    std::uint32_t get_version(const std::string& module_name) {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = current_versions_.find(module_name);
        if (it != current_versions_.end()) return it->second.version;
        return 0;
    }

private:
    CodeServer() = default;
    std::mutex mu_;
    std::map<std::string, ModuleVersion> current_versions_;
    std::map<std::string, ModuleVersion> old_versions_;
};

} // namespace garuda::beam
