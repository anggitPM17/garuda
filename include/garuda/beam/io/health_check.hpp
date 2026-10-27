#pragma once
// Garuda Production Infrastructure: Health Check & Readiness Probe Engine (`health_check.hpp`)
#include <chrono>
#include <functional>
#include <iostream>
#include <map>
#include <mutex>
#include <sstream>
#include <string>

namespace garuda::beam {

struct HealthStatus {
    bool is_healthy = true;
    std::string version = "67.0.0";
    double uptime_seconds = 0.0;
    std::map<std::string, bool> check_results;
};

class HealthChecker {
public:
    static HealthChecker& instance() {
        static HealthChecker inst;
        return inst;
    }

    void register_check(const std::string& name, std::function<bool()> check_fn) {
        std::lock_guard<std::mutex> lock(mu_);
        checks_[name] = check_fn;
    }

    void clear_checks() {
        std::lock_guard<std::mutex> lock(mu_);
        checks_.clear();
    }

    HealthStatus check_all() const {
        std::lock_guard<std::mutex> lock(mu_);
        HealthStatus status;
        status.version = version_;

        auto now = std::chrono::steady_clock::now();
        status.uptime_seconds = std::chrono::duration<double>(now - start_time_).count();

        for (const auto& [name, fn] : checks_) {
            bool ok = false;
            if (fn) {
                try {
                    ok = fn();
                } catch (...) {
                    ok = false;
                }
            }
            status.check_results[name] = ok;
            if (!ok) status.is_healthy = false;
        }

        return status;
    }

    std::string get_health_json() const {
        HealthStatus status = check_all();
        std::ostringstream ss;

        ss << "{"
           << "\"status\":\"" << (status.is_healthy ? "ok" : "error") << "\","
           << "\"version\":\"" << escape_json(status.version) << "\","
           << "\"uptime_seconds\":" << status.uptime_seconds << ","
           << "\"checks\":{";

        bool first = true;
        for (const auto& [name, ok] : status.check_results) {
            if (!first) ss << ",";
            ss << "\"" << escape_json(name) << "\":\"" << (ok ? "ok" : "failed") << "\"";
            first = false;
        }

        ss << "}}";
        return ss.str();
    }

    void set_version(const std::string& ver) {
        std::lock_guard<std::mutex> lock(mu_);
        version_ = ver;
    }

private:
    HealthChecker() : start_time_(std::chrono::steady_clock::now()) {}

    static std::string escape_json(const std::string& s) {
        std::string res;
        res.reserve(s.length() + 8);
        for (char c : s) {
            if (c == '"') res += "\\\"";
            else if (c == '\\') res += "\\\\";
            else res += c;
        }
        return res;
    }

    mutable std::mutex mu_;
    std::string version_ = "67.0.0";
    std::chrono::steady_clock::time_point start_time_;
    std::map<std::string, std::function<bool()>> checks_;
};

} // namespace garuda::beam
