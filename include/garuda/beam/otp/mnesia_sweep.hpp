#pragma once
// =====================================================================
// FASE G3.3 - GARUDA MNESIA SWEEP (mnesia_sweep.hpp)
// Auto-sweep expired data (setara richat_sweep.erl)
// =====================================================================
#include <string>
#include <map>
#include <vector>
#include <chrono>
#include <mutex>
#include <optional>
#include <cstddef>
#include <cstdint>

namespace garuda::beam {

struct ExpirableRow {
    std::string key;
    std::string value;
    std::chrono::steady_clock::time_point expires_at;

    bool is_expired() const {
        return std::chrono::steady_clock::now() >= expires_at;
    }
};

class MnesiaSweep {
private:
    std::map<std::string, ExpirableRow> rows_;
    mutable std::mutex mu_;
    std::size_t sweep_count_ = 0;

public:
    // Tulis row dengan TTL (dalam milidetik, 0 = permanent)
    void write(const std::string& key, const std::string& value, std::uint64_t ttl_ms = 0) {
        std::lock_guard<std::mutex> lock(mu_);
        ExpirableRow row;
        row.key = key;
        row.value = value;
        if (ttl_ms == 0) {
            row.expires_at = std::chrono::steady_clock::time_point::max();
        } else {
            row.expires_at = std::chrono::steady_clock::now() + std::chrono::milliseconds(ttl_ms);
        }
        rows_[key] = row;
    }

    // Direct write with specific expiration time_point (for deterministic tests)
    void write_expired(const std::string& key, const std::string& value) {
        std::lock_guard<std::mutex> lock(mu_);
        ExpirableRow row;
        row.key = key;
        row.value = value;
        row.expires_at = std::chrono::steady_clock::now() - std::chrono::seconds(10);
        rows_[key] = row;
    }

    std::optional<std::string> read(const std::string& key) {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = rows_.find(key);
        if (it == rows_.end()) return std::nullopt;
        if (it->second.is_expired()) {
            return std::nullopt;
        }
        return it->second.value;
    }

    std::size_t sweep() {
        std::lock_guard<std::mutex> lock(mu_);
        std::size_t removed = 0;
        for (auto it = rows_.begin(); it != rows_.end(); ) {
            if (it->second.is_expired()) {
                it = rows_.erase(it);
                removed++;
            } else {
                ++it;
            }
        }
        sweep_count_++;
        return removed;
    }

    std::size_t total_rows() const {
        std::lock_guard<std::mutex> lock(mu_);
        return rows_.size();
    }

    std::size_t expired_rows() const {
        std::lock_guard<std::mutex> lock(mu_);
        std::size_t count = 0;
        for (const auto& [k, row] : rows_) {
            if (row.is_expired()) count++;
        }
        return count;
    }

    std::size_t sweep_count() const {
        std::lock_guard<std::mutex> lock(mu_);
        return sweep_count_;
    }
};

} // namespace garuda::beam
