#pragma once
// Garuda Language Embedded Edge Engine: Edge Alert Dispatcher (`edge_alert_dispatcher.hpp`)
#include "anomaly_detector.hpp"

#include <algorithm>
#include <chrono>
#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace garuda::beam::micro {

struct EdgeAlert {
    std::string alert_type;
    std::string sensor_name;
    double value;
    std::uint64_t timestamp_ms;
};

class EdgeAlertDispatcher {
public:
    EdgeAlertDispatcher(std::size_t max_rate_per_sec = 10) : max_rate_per_sec_(max_rate_per_sec) {}

    bool dispatch(const EdgeAlert& alert, std::function<void(const EdgeAlert&)> output_cb) {
        if (alert.alert_type == last_alert_type_ && alert.sensor_name == last_sensor_name_ && alert.value == last_value_) {
            dedup_count_++;
            return false; // Deduplicated!
        }

        if (dispatched_count_ >= max_rate_per_sec_) {
            rate_limited_count_++;
            return false; // Rate limited!
        }

        last_alert_type_ = alert.alert_type;
        last_sensor_name_ = alert.sensor_name;
        last_value_ = alert.value;

        if (output_cb) output_cb(alert);
        dispatched_count_++;
        return true;
    }

    std::size_t dispatched_count() const { return dispatched_count_; }
    std::size_t dedup_count() const { return dedup_count_; }
    std::size_t rate_limited_count() const { return rate_limited_count_; }

    void reset_rate_limit() {
        dispatched_count_ = 0;
    }

private:
    std::size_t max_rate_per_sec_;
    std::string last_alert_type_;
    std::string last_sensor_name_;
    double last_value_ = 0.0;

    std::size_t dispatched_count_ = 0;
    std::size_t dedup_count_ = 0;
    std::size_t rate_limited_count_ = 0;
};

} // namespace garuda::beam::micro
