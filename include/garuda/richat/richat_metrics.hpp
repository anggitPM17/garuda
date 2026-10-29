#pragma once
// =====================================================================
// R3 - RICHAT METRICS (Prometheus-compatible)
// =====================================================================
#include <atomic>
#include <string>
#include <sstream>
#include <map>
#include <mutex>
#include <cstdint>

namespace garuda::richat {

class RichatMetrics {
private:
    std::atomic<uint64_t> total_requests_{0};
    std::atomic<uint64_t> total_errors_{0};
    std::atomic<uint64_t> total_latency_ms_{0};

    // Metrics per action
    std::map<std::string, uint64_t> action_counts_;
    std::mutex mutex_;

public:
    void record_request(const std::string& action, uint64_t latency_ms, bool success) {
        total_requests_.fetch_add(1);
        total_latency_ms_.fetch_add(latency_ms);
        if (!success) total_errors_.fetch_add(1);

        std::lock_guard<std::mutex> lock(mutex_);
        action_counts_[action]++;
    }

    // Render dalam format Prometheus
    std::string render_prometheus() {
        std::stringstream ss;

        uint64_t reqs = total_requests_.load();
        uint64_t errs = total_errors_.load();
        uint64_t total_lat = total_latency_ms_.load();
        double avg_lat = reqs > 0 ? static_cast<double>(total_lat) / reqs : 0.0;

        ss << "# HELP richat_requests_total Total requests processed\n";
        ss << "# TYPE richat_requests_total counter\n";
        ss << "richat_requests_total " << reqs << "\n";

        ss << "# HELP richat_errors_total Total errors processed\n";
        ss << "# TYPE richat_errors_total counter\n";
        ss << "richat_errors_total " << errs << "\n";

        ss << "# HELP richat_latency_ms_avg Average latency in ms\n";
        ss << "# TYPE richat_latency_ms_avg gauge\n";
        ss << "richat_latency_ms_avg " << avg_lat << "\n";

        // Per action
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto& [action, count] : action_counts_) {
            ss << "richat_action_total{action=\"" << action << "\"} " << count << "\n";
        }

        return ss.str();
    }
};

} // namespace garuda::richat
