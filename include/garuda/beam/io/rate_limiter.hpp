#pragma once
// Garuda Production Infrastructure: Rate Limiter Engine (`rate_limiter.hpp`)
#include <chrono>
#include <deque>
#include <map>
#include <mutex>
#include <string>

namespace garuda::beam {

class RateLimiter {
public:
    RateLimiter() = default;

    bool is_allowed(const std::string& key, int max_requests, int window_seconds) {
        std::lock_guard<std::mutex> lock(mu_);
        auto now = std::chrono::steady_clock::now();
        auto window_duration = std::chrono::seconds(window_seconds);

        auto& timestamps = requests_[key];

        // Slide window: remove timestamps older than window_seconds
        while (!timestamps.empty() && (now - timestamps.front()) > window_duration) {
            timestamps.pop_front();
        }

        if (static_cast<int>(timestamps.size()) < max_requests) {
            timestamps.push_back(now);
            return true;
        }

        return false;
    }

    void reset(const std::string& key) {
        std::lock_guard<std::mutex> lock(mu_);
        requests_.erase(key);
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mu_);
        requests_.clear();
    }

private:
    mutable std::mutex mu_;
    std::map<std::string, std::deque<std::chrono::steady_clock::time_point>> requests_;
};

} // namespace garuda::beam
