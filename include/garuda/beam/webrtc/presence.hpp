#pragma once
// =====================================================================
// FASE G5.2 - Presence: Online/offline/last seen monitoring
// =====================================================================
#include <string>
#include <map>
#include <vector>
#include <mutex>
#include <optional>
#include <chrono>
#include <cstddef>

namespace garuda::beam {

enum class PresenceStatus { ONLINE, AWAY, BUSY, OFFLINE };

struct PresenceRecord {
    std::string user_id;
    PresenceStatus status = PresenceStatus::OFFLINE;
    std::chrono::steady_clock::time_point last_seen;
    std::string status_message;
};

class Presence {
private:
    std::map<std::string, PresenceRecord> records_;
    mutable std::mutex mu_;

public:
    void set_online(const std::string& user_id,
                    PresenceStatus status = PresenceStatus::ONLINE,
                    const std::string& message = "") {
        std::lock_guard<std::mutex> lock(mu_);
        PresenceRecord rec;
        rec.user_id = user_id;
        rec.status = status;
        rec.last_seen = std::chrono::steady_clock::now();
        rec.status_message = message;
        records_[user_id] = rec;
    }

    void set_offline(const std::string& user_id) {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = records_.find(user_id);
        if (it != records_.end()) {
            it->second.status = PresenceStatus::OFFLINE;
            it->second.last_seen = std::chrono::steady_clock::now();
        } else {
            PresenceRecord rec;
            rec.user_id = user_id;
            rec.status = PresenceStatus::OFFLINE;
            rec.last_seen = std::chrono::steady_clock::now();
            records_[user_id] = rec;
        }
    }

    std::optional<PresenceRecord> get_presence(const std::string& user_id) const {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = records_.find(user_id);
        if (it != records_.end()) return it->second;
        return std::nullopt;
    }

    std::vector<PresenceRecord> get_online_users() const {
        std::lock_guard<std::mutex> lock(mu_);
        std::vector<PresenceRecord> out;
        for (const auto& [uid, rec] : records_) {
            if (rec.status != PresenceStatus::OFFLINE) {
                out.push_back(rec);
            }
        }
        return out;
    }

    bool is_online(const std::string& user_id) const {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = records_.find(user_id);
        if (it != records_.end()) {
            return it->second.status != PresenceStatus::OFFLINE;
        }
        return false;
    }

    void heartbeat(const std::string& user_id) {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = records_.find(user_id);
        if (it != records_.end()) {
            it->second.last_seen = std::chrono::steady_clock::now();
        }
    }

    std::size_t online_count() const {
        std::lock_guard<std::mutex> lock(mu_);
        std::size_t count = 0;
        for (const auto& [uid, rec] : records_) {
            if (rec.status != PresenceStatus::OFFLINE) count++;
        }
        return count;
    }

    std::size_t total_count() const {
        std::lock_guard<std::mutex> lock(mu_);
        return records_.size();
    }
};

} // namespace garuda::beam
