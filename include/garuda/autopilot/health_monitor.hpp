#pragma once
// =====================================================================
// FASE 5.1 - HEALTH MONITOR
// Cek kesehatan sistem (DB, RAM, latensi) sebelum AI eksekusi.
// Sumber metrik injectable -> deterministic untuk test.
// =====================================================================
#include <functional>
#include <string>
#include <mutex>

namespace garuda::autopilot {

struct HealthSnapshot {
    bool db_ok = true;
    double ram_usage_mb = 0.0;
    double latency_ms = 0.0;

    // Threshold default (bisa di-override via HealthMonitor)
    static constexpr double DEFAULT_MAX_RAM_MB = 2048.0;
    static constexpr double DEFAULT_MAX_LATENCY_MS = 500.0;

    bool is_healthy(double max_ram_mb = DEFAULT_MAX_RAM_MB,
                    double max_latency_ms = DEFAULT_MAX_LATENCY_MS) const {
        return db_ok
            && ram_usage_mb <= max_ram_mb
            && latency_ms  <= max_latency_ms;
    }
};

class HealthMonitor {
public:
    using MetricSource = std::function<HealthSnapshot()>;

private:
    MetricSource source_;
    double max_ram_mb_;
    double max_latency_ms_;
    mutable std::mutex mutex_;

public:
    explicit HealthMonitor(MetricSource source,
                           double max_ram_mb = HealthSnapshot::DEFAULT_MAX_RAM_MB,
                           double max_latency_ms = HealthSnapshot::DEFAULT_MAX_LATENCY_MS)
        : source_(std::move(source)),
          max_ram_mb_(max_ram_mb),
          max_latency_ms_(max_latency_ms) {}

    // Sampling aman: jika source throw, kembalikan snapshot "tidak sehat".
    HealthSnapshot sample() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!source_) {
            HealthSnapshot bad;
            bad.db_ok = false;
            return bad;
        }
        try {
            return source_();
        } catch (...) {
            HealthSnapshot bad;
            bad.db_ok = false;
            return bad;
        }
    }

    bool is_healthy() {
        return sample().is_healthy(max_ram_mb_, max_latency_ms_);
    }

    double max_ram_mb() const { return max_ram_mb_; }
    double max_latency_ms() const { return max_latency_ms_; }
};

} // namespace garuda::autopilot
