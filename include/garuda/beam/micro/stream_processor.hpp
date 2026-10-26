#pragma once
// Garuda Language Embedded Edge Engine: Real-Time Stream Processor (`stream_processor.hpp`)
#include "sensor_pipeline.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <functional>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

namespace garuda::beam::micro {

constexpr std::size_t STREAM_BUFFER_CAPACITY = 256;

enum class BackpressureStrategy { DropOldest, RejectNewest };

class StreamProcessor {
public:
    explicit StreamProcessor(BackpressureStrategy strategy = BackpressureStrategy::DropOldest)
        : strategy_(strategy) {}

    bool push(const SensorSample& sample) {
        if (size_ >= STREAM_BUFFER_CAPACITY) {
            if (strategy_ == BackpressureStrategy::RejectNewest) {
                rejected_count_++;
                return false; // Rejected due to backpressure!
            } else {
                // Drop oldest
                head_ = (head_ + 1) % STREAM_BUFFER_CAPACITY;
                size_--;
                dropped_count_++;
            }
        }

        buffer_[tail_] = sample;
        tail_ = (tail_ + 1) % STREAM_BUFFER_CAPACITY;
        size_++;
        return true;
    }

    std::optional<SensorSample> pop() {
        if (size_ == 0) return std::nullopt;

        SensorSample s = buffer_[head_];
        head_ = (head_ + 1) % STREAM_BUFFER_CAPACITY;
        size_--;
        return s;
    }

    std::size_t size() const { return size_; }
    std::size_t dropped_count() const { return dropped_count_; }
    std::size_t rejected_count() const { return rejected_count_; }

    void flush() {
        head_ = 0;
        tail_ = 0;
        size_ = 0;
    }

private:
    BackpressureStrategy strategy_;
    std::array<SensorSample, STREAM_BUFFER_CAPACITY> buffer_{};
    std::size_t head_ = 0;
    std::size_t tail_ = 0;
    std::size_t size_ = 0;
    std::size_t dropped_count_ = 0;
    std::size_t rejected_count_ = 0;
};

} // namespace garuda::beam::micro
