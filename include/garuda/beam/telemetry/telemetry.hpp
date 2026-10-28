#pragma once
// =====================================================================
// FASE G7.3 - Telemetry Aggregator: Unified metrics + tracing + logging
// Setara erlang:system_info/1, logger.erl, & OpenTelemetry
// =====================================================================
#include "../sys/metrics.hpp"
#include "../sys/tracing.hpp"
#include "../sys/logger.hpp"

#include <string>
#include <map>
#include <vector>
#include <mutex>
#include <optional>
#include <chrono>
#include <cstdint>

namespace garuda::beam {

struct TelemetryConfig {
    bool enable_metrics = true;
    bool enable_tracing = true;
    bool enable_logging = true;
    std::string service_name = "garuda";
};

struct SystemInfo {
    std::string version;
    std::uint64_t process_count = 0;
    std::uint64_t total_memory_bytes = 0;
    std::uint64_t uptime_seconds = 0;
    std::uint64_t total_messages = 0;
};

class Telemetry {
private:
    TelemetryConfig config_;
    std::chrono::steady_clock::time_point started_at_;
    std::uint64_t message_count_ = 0;
    std::map<std::string, std::uint64_t> counters_;
    std::map<std::string, std::vector<double>> histograms_;
    mutable std::mutex mu_;

public:
    explicit Telemetry(TelemetryConfig config = {})
        : config_(std::move(config)), started_at_(std::chrono::steady_clock::now()) {}

    void record_message(const std::string& module, const std::string& type = "default") {
        std::lock_guard<std::mutex> lock(mu_);
        message_count_++;
        counters_["messages_total"]++;
        counters_["messages_" + module]++;
        counters_["messages_type_" + type]++;
    }

    SystemInfo system_info() const {
        std::lock_guard<std::mutex> lock(mu_);
        SystemInfo info;
        info.version = "Garuda BEAM v67.0.0";
        info.process_count = 1;
        info.total_memory_bytes = 1024 * 1024;
        info.uptime_seconds = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::steady_clock::now() - started_at_).count();
        info.total_messages = message_count_;
        return info;
    }

    void record_latency(const std::string& module, std::chrono::milliseconds duration) {
        std::lock_guard<std::mutex> lock(mu_);
        std::string key = "latency_" + module + "_ms";
        histograms_[key].push_back(static_cast<double>(duration.count()));
        counters_[key + "_count"]++;
    }

    TelemetryConfig config() const {
        std::lock_guard<std::mutex> lock(mu_);
        return config_;
    }

    std::uint64_t get_counter(const std::string& name) const {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = counters_.find(name);
        if (it != counters_.end()) return it->second;
        return 0;
    }

    std::uint64_t message_count() const {
        std::lock_guard<std::mutex> lock(mu_);
        return message_count_;
    }

    std::chrono::seconds uptime() const {
        std::lock_guard<std::mutex> lock(mu_);
        return std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::steady_clock::now() - started_at_);
    }

    bool health_check() const {
        std::lock_guard<std::mutex> lock(mu_);
        return config_.enable_metrics && config_.enable_logging;
    }
};

} // namespace garuda::beam
