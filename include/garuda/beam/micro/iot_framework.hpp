#pragma once
// Garuda Language Embedded Framework: IoT & Edge Computing (`iot_framework.hpp`)
#include "gvm_micro.hpp"

#include <algorithm>
#include <chrono>
#include <functional>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

namespace garuda::beam::micro {

class EdgeRule {
public:
    EdgeRule(double temp_threshold, std::function<void(double)> alert_cb)
        : temp_threshold_(temp_threshold), alert_cb_(std::move(alert_cb)) {}

    void evaluate(double temp_reading) {
        if (temp_reading > temp_threshold_) {
            if (alert_cb_) alert_cb_(temp_reading);
        }
    }

private:
    double temp_threshold_;
    std::function<void(double)> alert_cb_;
};

class LocalAggregator {
public:
    LocalAggregator() = default;

    void add_sample(double val) {
        samples_.push_back(val);
    }

    double get_min() const {
        if (samples_.empty()) return 0.0;
        return *std::min_element(samples_.begin(), samples_.end());
    }

    double get_max() const {
        if (samples_.empty()) return 0.0;
        return *std::max_element(samples_.begin(), samples_.end());
    }

    double get_avg() const {
        if (samples_.empty()) return 0.0;
        double sum = std::accumulate(samples_.begin(), samples_.end(), 0.0);
        return sum / samples_.size();
    }

    void clear() { samples_.clear(); }

private:
    std::vector<double> samples_;
};

} // namespace garuda::beam::micro
