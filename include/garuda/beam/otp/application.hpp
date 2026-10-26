#pragma once
// Garuda BEAM-Core: OTP Application Behaviour (`application.hpp`)
// Mengatur siklus hidup aplikasi OTP, env config, dan bootstrapper pohon supervisi.
#include "../core/runtime.hpp"
#include "supervisor.hpp"

#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <string>

namespace garuda::beam {

class Application {
public:
    virtual ~Application() = default;

    virtual Pid start(Runtime& rt) = 0;
    virtual void stop(Runtime& rt) { (void)rt; }

    static void set_env(const std::string& app, const std::string& key, Term value) {
        std::lock_guard<std::mutex> lock(mu_);
        env_[app][key] = std::move(value);
    }

    static std::optional<Term> get_env(const std::string& app, const std::string& key) {
        std::lock_guard<std::mutex> lock(mu_);
        auto it_app = env_.find(app);
        if (it_app == env_.end()) return std::nullopt;
        auto it_key = it_app->second.find(key);
        if (it_key == it_app->second.end()) return std::nullopt;
        return it_key->second;
    }

private:
    static inline std::mutex mu_;
    static inline std::map<std::string, std::map<std::string, Term>> env_;
};

} // namespace garuda::beam
