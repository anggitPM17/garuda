#pragma once
// =====================================================================
// FASE C.5.1 - Rate Limiter: Batasi request per IP/user
// =====================================================================
#include <string>
#include <map>
#include <deque>
#include <mutex>
#include <chrono>
#include <cstddef>

namespace garuda::security {

struct RateLimitConfig {
    int max_requests = 10;
    std::chrono::seconds window{60};
};

class RateLimiter {
private:
    struct Bucket {
        std::deque<std::chrono::steady_clock::time_point> timestamps;
    };
    std::map<std::string, Bucket> buckets_;
    RateLimitConfig config_;
    mutable std::mutex mu_;

public:
    RateLimiter() = default;
    explicit RateLimiter(RateLimitConfig config) : config_(config) {}

    bool allow(const std::string& key) {
        if (key.empty()) return true;
        std::lock_guard<std::mutex> lock(mu_);

        auto now = std::chrono::steady_clock::now();
        auto& bucket = buckets_[key];

        // Clean up expired timestamps older than window
        while (!bucket.timestamps.empty() &&
               (now - bucket.timestamps.front()) > config_.window) {
            bucket.timestamps.pop_front();
        }

        if (static_cast<int>(bucket.timestamps.size()) >= config_.max_requests) {
            return false; // Rate limit breached
        }

        bucket.timestamps.push_back(now);
        return true;
    }

    void reset(const std::string& key) {
        std::lock_guard<std::mutex> lock(mu_);
        buckets_.erase(key);
    }

    std::size_t bucket_count() const {
        std::lock_guard<std::mutex> lock(mu_);
        return buckets_.size();
    }

    std::size_t request_count(const std::string& key) const {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = buckets_.find(key);
        if (it != buckets_.end()) {
            return it->second.timestamps.size();
        }
        return 0;
    }

    void set_config(const RateLimitConfig& config) {
        std::lock_guard<std::mutex> lock(mu_);
        config_ = config;
    }

    RateLimitConfig get_config() const {
        std::lock_guard<std::mutex> lock(mu_);
        return config_;
    }
};

} // namespace garuda::security
