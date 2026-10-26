#pragma once
// Garuda Language Embedded Edge Engine: Anomaly Detector (`anomaly_detector.hpp`)
#include "windowed_aggregator.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <vector>

namespace garuda::beam::micro {

class AnomalyDetector {
public:
    explicit AnomalyDetector(double z_threshold = 3.0) : z_threshold_(z_threshold) {}

    bool detect_zscore(double new_val, double window_mean, double window_stddev) const {
        if (window_stddev <= 0.0) return false;
        double z = std::abs(new_val - window_mean) / window_stddev;
        return z > z_threshold_;
    }

    bool detect_threshold(double val, double min_val, double max_val) const {
        return val < min_val || val > max_val;
    }

    bool detect_moving_avg_dev(double val, double moving_avg, double max_pct_dev) const {
        if (moving_avg == 0.0) return false;
        double dev = (std::abs(val - moving_avg) / std::abs(moving_avg)) * 100.0;
        return dev > max_pct_dev;
    }

private:
    double z_threshold_ = 3.0;
};

} // namespace garuda::beam::micro
