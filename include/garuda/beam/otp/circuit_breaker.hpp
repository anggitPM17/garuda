#pragma once
// Garuda OTP: Circuit Breaker & Sliding-Window Rate Limiter (`circuit_breaker.hpp`)
// Melindungi proses Garuda dari efek domino kegagalan (cascading failure) dan serangan DDoS.
#include <chrono>
#include <cstdint>
#include <functional>
#include <iostream>
#include <map>
#include <mutex>
#include <string>
#include <vector>

namespace garuda::beam {

enum class CircuitState { Closed, Open, HalfOpen };

class CircuitBreaker {
public:
    explicit CircuitBreaker(std::size_t failure_threshold = 5, std::uint64_t reset_timeout_ms = 10000)
        : threshold_(failure_threshold), timeout_ms_(reset_timeout_ms), state_(CircuitState::Closed) {}

    template <typename Func>
    auto call(Func fn) -> std::optional<decltype(fn())> {
        std::lock_guard<std::mutex> lock(mu_);
        std::uint64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();

        if (state_ == CircuitState::Open) {
            if (now - last_failure_time_ > timeout_ms_) {
                state_ = CircuitState::HalfOpen;
                std::cout << "🟡 [GARUDA CIRCUIT BREAKER] Circuit transitioned to HALF-OPEN" << std::endl;
            } else {
                std::cout << "🔴 [GARUDA CIRCUIT BREAKER] Circuit OPEN: Request rejected!" << std::endl;
                return std::nullopt;
            }
        }

        try {
            auto res = fn();
            if (state_ == CircuitState::HalfOpen) {
                state_ = CircuitState::Closed;
                failures_ = 0;
                std::cout << "🟢 [GARUDA CIRCUIT BREAKER] Circuit recovered to CLOSED" << std::endl;
            }
            return res;
        } catch (...) {
            failures_++;
            last_failure_time_ = now;
            if (failures_ >= threshold_) {
                state_ = CircuitState::Open;
                std::cout << "🚨 [GARUDA CIRCUIT BREAKER] Threshold reached! Circuit OPENED." << std::endl;
            }
            throw;
        }
    }

    CircuitState state() const { return state_; }

private:
    std::size_t threshold_;
    std::uint64_t timeout_ms_;
    CircuitState state_;
    std::size_t failures_ = 0;
    std::uint64_t last_failure_time_ = 0;
    std::mutex mu_;
};

class RateLimiter {
public:
    bool check_limit(const std::string& key, std::size_t max_requests, std::uint64_t window_ms) {
        std::lock_guard<std::mutex> lock(mu_);
        std::uint64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();

        auto& timestamps = requests_[key];
        // Clean expired timestamps outside window
        while (!timestamps.empty() && now - timestamps.front() > window_ms) {
            timestamps.erase(timestamps.begin());
        }

        if (timestamps.size() >= max_requests) {
            return false; // Rate limit exceeded!
        }

        timestamps.push_back(now);
        return true;
    }

private:
    std::mutex mu_;
    std::map<std::string, std::vector<std::uint64_t>> requests_;
};

} // namespace garuda::beam
