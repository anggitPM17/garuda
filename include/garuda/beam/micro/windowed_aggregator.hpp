#pragma once
// Garuda Language Embedded Edge Engine: Windowed Aggregator (`windowed_aggregator.hpp`)
#include "stream_processor.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <numeric>
#include <vector>

namespace garuda::beam::micro {

struct WindowStats {
    double sum = 0.0;
    double avg = 0.0;
    double min = 0.0;
    double max = 0.0;
    std::size_t count = 0;
    double p50 = 0.0;
    double p95 = 0.0;
    double p99 = 0.0;
};

class TumblingWindow {
public:
    explicit TumblingWindow(std::size_t window_size) : window_size_(window_size) {}

    bool add_sample(double val) {
        samples_.push_back(val);
        if (samples_.size() >= window_size_) {
            compute_stats();
            return true; // Window full / triggered!
        }
        return false;
    }

    WindowStats get_stats() const { return stats_; }

    void flush() {
        samples_.clear();
        stats_ = WindowStats{};
    }

private:
    void compute_stats() {
        if (samples_.empty()) return;

        stats_.count = samples_.size();
        stats_.sum = std::accumulate(samples_.begin(), samples_.end(), 0.0);
        stats_.avg = stats_.sum / stats_.count;
        stats_.min = *std::min_element(samples_.begin(), samples_.end());
        stats_.max = *std::max_element(samples_.begin(), samples_.end());

        auto sorted = samples_;
        std::sort(sorted.begin(), sorted.end());
        stats_.p50 = sorted[static_cast<std::size_t>(sorted.size() * 0.50)];
        stats_.p95 = sorted[static_cast<std::size_t>((sorted.size() - 1) * 0.95)];
        stats_.p99 = sorted[static_cast<std::size_t>((sorted.size() - 1) * 0.99)];
    }

    std::size_t window_size_;
    std::vector<double> samples_;
    WindowStats stats_;
};

class SlidingWindow {
public:
    SlidingWindow(std::size_t window_size, std::size_t slide_step)
        : window_size_(window_size), slide_step_(slide_step) {}

    bool add_sample(double val) {
        samples_.push_back(val);
        step_count_++;

        if (samples_.size() > window_size_) {
            samples_.erase(samples_.begin());
        }

        if (samples_.size() == window_size_ && step_count_ >= slide_step_) {
            step_count_ = 0;
            compute_stats();
            return true; // Sliding step triggered!
        }
        return false;
    }

    WindowStats get_stats() const { return stats_; }

private:
    void compute_stats() {
        if (samples_.empty()) return;

        stats_.count = samples_.size();
        stats_.sum = std::accumulate(samples_.begin(), samples_.end(), 0.0);
        stats_.avg = stats_.sum / stats_.count;
        stats_.min = *std::min_element(samples_.begin(), samples_.end());
        stats_.max = *std::max_element(samples_.begin(), samples_.end());

        auto sorted = samples_;
        std::sort(sorted.begin(), sorted.end());
        stats_.p50 = sorted[static_cast<std::size_t>(sorted.size() * 0.50)];
        stats_.p95 = sorted[static_cast<std::size_t>((sorted.size() - 1) * 0.95)];
        stats_.p99 = sorted[static_cast<std::size_t>((sorted.size() - 1) * 0.99)];
    }

    std::size_t window_size_;
    std::size_t slide_step_;
    std::size_t step_count_ = 0;
    std::vector<double> samples_;
    WindowStats stats_;
};

class SessionWindow {
public:
    explicit SessionWindow(std::uint64_t max_gap_ms) : max_gap_ms_(max_gap_ms) {}

    bool add_sample(double val, std::uint64_t timestamp_ms) {
        if (last_timestamp_ms_ > 0 && (timestamp_ms - last_timestamp_ms_) > max_gap_ms_) {
            compute_stats();
            samples_.clear();
            samples_.push_back(val);
            last_timestamp_ms_ = timestamp_ms;
            return true; // Gap exceeded -> Session flushed!
        }

        samples_.push_back(val);
        last_timestamp_ms_ = timestamp_ms;
        return false;
    }

    WindowStats get_stats() const { return stats_; }

private:
    void compute_stats() {
        if (samples_.empty()) return;

        stats_.count = samples_.size();
        stats_.sum = std::accumulate(samples_.begin(), samples_.end(), 0.0);
        stats_.avg = stats_.sum / stats_.count;
        stats_.min = *std::min_element(samples_.begin(), samples_.end());
        stats_.max = *std::max_element(samples_.begin(), samples_.end());
    }

    std::uint64_t max_gap_ms_;
    std::uint64_t last_timestamp_ms_ = 0;
    std::vector<double> samples_;
    WindowStats stats_;
};

} // namespace garuda::beam::micro
