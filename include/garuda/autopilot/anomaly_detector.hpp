#pragma once
// =====================================================================
// FASE D.1.1 - Anomaly Detector: Deteksi anomali dari metrics & logs
// =====================================================================
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <chrono>
#include <optional>
#include <algorithm>
#include <cstddef>

namespace garuda::autopilot {

enum class AnomalySeverity { LOW, MEDIUM, HIGH, CRITICAL };

enum class AnomalyType {
    HIGH_CPU,
    HIGH_MEMORY,
    HIGH_ERROR_RATE,
    REPEATED_CRASH,
    SLOW_RESPONSE,
    DISK_FULL,
    NETWORK_ERROR
};

struct Anomaly {
    std::string id;
    AnomalyType type;
    AnomalySeverity severity;
    std::string description;
    std::string metric_name;
    double actual_value = 0.0;
    double threshold = 0.0;
    std::chrono::steady_clock::time_point detected_at;
};

struct AnomalyRule {
    AnomalyType type;
    std::string metric_name;
    double threshold = 0.0;
    AnomalySeverity severity = AnomalySeverity::MEDIUM;
    int consecutive_breaches = 1;
};

class AnomalyDetector {
private:
    std::vector<AnomalyRule> rules_;
    std::map<AnomalyType, int> breach_counts_;
    std::vector<Anomaly> active_anomalies_;
    mutable std::mutex mu_;

public:
    AnomalyDetector() {
        // Register default rules
        rules_.push_back({AnomalyType::HIGH_CPU, "cpu_percent", 80.0, AnomalySeverity::HIGH, 3});
        rules_.push_back({AnomalyType::HIGH_MEMORY, "memory_percent", 80.0, AnomalySeverity::HIGH, 3});
        rules_.push_back({AnomalyType::HIGH_ERROR_RATE, "error_rate", 5.0, AnomalySeverity::HIGH, 5});
        rules_.push_back({AnomalyType::REPEATED_CRASH, "crash_count", 3.0, AnomalySeverity::CRITICAL, 1});
        rules_.push_back({AnomalyType::SLOW_RESPONSE, "response_time_ms", 1000.0, AnomalySeverity::MEDIUM, 3});
        rules_.push_back({AnomalyType::DISK_FULL, "disk_percent", 90.0, AnomalySeverity::CRITICAL, 1});
    }

    void add_rule(const AnomalyRule& rule) {
        std::lock_guard<std::mutex> lock(mu_);
        rules_.push_back(rule);
    }

    void ingest_metric(const std::string& name, double value) {
        std::lock_guard<std::mutex> lock(mu_);
        for (const auto& rule : rules_) {
            if (rule.metric_name == name) {
                if (value > rule.threshold) {
                    breach_counts_[rule.type]++;
                    if (breach_counts_[rule.type] >= rule.consecutive_breaches) {
                        // Check if already active
                        auto it = std::find_if(active_anomalies_.begin(), active_anomalies_.end(),
                            [&](const Anomaly& a) { return a.type == rule.type; });

                        if (it == active_anomalies_.end()) {
                            Anomaly a;
                            a.id = "anom_" + std::to_string(active_anomalies_.size() + 1) + "_" + name;
                            a.type = rule.type;
                            a.severity = rule.severity;
                            a.description = "Metric " + name + " breached threshold (" +
                                            std::to_string(value) + " > " + std::to_string(rule.threshold) + ")";
                            a.metric_name = name;
                            a.actual_value = value;
                            a.threshold = rule.threshold;
                            a.detected_at = std::chrono::steady_clock::now();
                            active_anomalies_.push_back(a);
                        } else {
                            it->actual_value = value;
                        }
                    }
                } else {
                    breach_counts_[rule.type] = 0;
                }
            }
        }
    }

    std::vector<Anomaly> active_anomalies() const {
        std::lock_guard<std::mutex> lock(mu_);
        return active_anomalies_;
    }

    void clear_anomaly(const std::string& anomaly_id) {
        std::lock_guard<std::mutex> lock(mu_);
        active_anomalies_.erase(
            std::remove_if(active_anomalies_.begin(), active_anomalies_.end(),
                [&](const Anomaly& a) { return a.id == anomaly_id; }),
            active_anomalies_.end());
    }

    std::size_t rule_count() const {
        std::lock_guard<std::mutex> lock(mu_);
        return rules_.size();
    }

    std::size_t anomaly_count() const {
        std::lock_guard<std::mutex> lock(mu_);
        return active_anomalies_.size();
    }
};

} // namespace garuda::autopilot
