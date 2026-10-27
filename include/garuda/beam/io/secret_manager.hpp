#pragma once
// Garuda Production Infrastructure: Secret Management & Environment Config Loader (`secret_manager.hpp`)
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <mutex>
#include <sstream>
#include <string>

namespace garuda::beam {

class SecretManager {
public:
    static SecretManager& instance() {
        static SecretManager inst;
        return inst;
    }

    std::string get_secret(const std::string& key, const std::string& default_val = "") const {
        std::lock_guard<std::mutex> lock(mu_);

        // 1. Check internal file-loaded secrets map
        auto it = file_secrets_.find(key);
        if (it != file_secrets_.end() && !it->second.empty()) {
            return it->second;
        }

        // 2. Check OS Environment Variables
        const char* env_val = std::getenv(key.c_str());
        if (env_val && std::string(env_val).length() > 0) {
            return std::string(env_val);
        }

        // 3. Fallback to default
        return default_val;
    }

    bool has_secret(const std::string& key) const {
        std::lock_guard<std::mutex> lock(mu_);
        if (file_secrets_.count(key) > 0) return true;
        const char* env_val = std::getenv(key.c_str());
        return (env_val != nullptr);
    }

    static std::string redact(const std::string& secret) {
        if (secret.empty()) return "";
        return "****";
    }

    bool load_from_file(const std::string& filepath) {
        std::lock_guard<std::mutex> lock(mu_);
        std::ifstream ifs(filepath);
        if (!ifs.is_open()) return false;

        std::string line;
        while (std::getline(ifs, line)) {
            // Trim whitespace
            trim(line);
            if (line.empty() || line.front() == '#') continue;

            auto eq_pos = line.find('=');
            if (eq_pos != std::string::npos) {
                std::string k = line.substr(0, eq_pos);
                std::string v = line.substr(eq_pos + 1);

                trim(k);
                trim(v);
                strip_quotes(v);

                if (!k.empty()) {
                    file_secrets_[k] = v;
                }
            }
        }

        return true;
    }

    void clear_file_secrets() {
        std::lock_guard<std::mutex> lock(mu_);
        file_secrets_.clear();
    }

private:
    SecretManager() = default;

    static void trim(std::string& s) {
        while (!s.empty() && (s.front() == ' ' || s.front() == '\t' || s.front() == '\r' || s.front() == '\n')) {
            s.erase(0, 1);
        }
        while (!s.empty() && (s.back() == ' ' || s.back() == '\t' || s.back() == '\r' || s.back() == '\n')) {
            s.pop_back();
        }
    }

    static void strip_quotes(std::string& s) {
        if (s.length() >= 2) {
            if ((s.front() == '"' && s.back() == '"') || (s.front() == '\'' && s.back() == '\'')) {
                s = s.substr(1, s.length() - 2);
            }
        }
    }

    mutable std::mutex mu_;
    std::map<std::string, std::string> file_secrets_;
};

} // namespace garuda::beam
