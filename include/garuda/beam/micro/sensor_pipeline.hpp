#pragma once
// Garuda Language Embedded Real-Time Sensor Pipeline (`sensor_pipeline.hpp`)
#include "sensor_drivers.hpp"

#include <algorithm>
#include <chrono>
#include <functional>
#include <iostream>
#include <queue>
#include <string>
#include <vector>

namespace garuda::beam::micro {

struct SensorSample {
    std::string sensor_name;
    double value;
    std::uint64_t timestamp_ms;
};

class SensorPipeline {
public:
    SensorPipeline(std::string name, std::size_t sample_period_ms, std::function<double()> read_cb)
        : name_(std::move(name)), sample_period_ms_(sample_period_ms), read_cb_(std::move(read_cb)) {}

    void sample_now() {
        if (read_cb_) {
            double val = read_cb_();
            buffer_.push({name_, val, 1000});
            sample_count_++;
        }
    }

    bool has_samples() const { return !buffer_.empty(); }

    SensorSample pop_sample() {
        if (buffer_.empty()) return {"", 0.0, 0};
        auto s = buffer_.front();
        buffer_.pop();
        return s;
    }

    std::size_t sample_count() const { return sample_count_; }

private:
    std::string name_;
    std::size_t sample_period_ms_;
    std::function<double()> read_cb_;
    std::queue<SensorSample> buffer_;
    std::size_t sample_count_ = 0;
};

class PipelineScheduler {
public:
    PipelineScheduler() = default;

    void add_pipeline(SensorPipeline pipe) {
        pipelines_.push_back(std::move(pipe));
    }

    void tick() {
        for (auto& p : pipelines_) {
            p.sample_now();
        }
    }

    std::size_t total_samples() const {
        std::size_t total = 0;
        for (const auto& p : pipelines_) {
            total += p.sample_count();
        }
        return total;
    }

private:
    std::vector<SensorPipeline> pipelines_;
};

} // namespace garuda::beam::micro
