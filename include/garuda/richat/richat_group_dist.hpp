#pragma once
// =====================================================================
// FASE G8.4 - RICHAT Group Sender Key Distribution (setara richat_group_dist.erl)
// =====================================================================
#include <string>
#include <map>
#include <mutex>
#include <optional>
#include <vector>
#include <chrono>
#include <cstddef>

namespace garuda::richat {

struct SenderKey {
    std::string group_id;
    std::string epoch;      // rotation epoch (e.g. "e1", "e2")
    std::string key_b64;    // base64-encoded sender key
    std::chrono::steady_clock::time_point created_at;
};

class RichatGroupDist {
private:
    std::map<std::string, SenderKey> keys_;  // group_id -> latest SenderKey
    std::map<std::string, std::size_t> epoch_counters_;
    mutable std::mutex mu_;

public:
    SenderKey distribute_key(const std::string& group_id, const std::string& key_b64) {
        std::lock_guard<std::mutex> lock(mu_);
        std::size_t epoch_num = ++epoch_counters_[group_id];
        std::string epoch_str = "e" + std::to_string(epoch_num);

        SenderKey sk;
        sk.group_id = group_id;
        sk.epoch = epoch_str;
        sk.key_b64 = key_b64;
        sk.created_at = std::chrono::steady_clock::now();

        keys_[group_id] = sk;
        return sk;
    }

    SenderKey rotate_key(const std::string& group_id, const std::string& new_key_b64) {
        return distribute_key(group_id, new_key_b64);
    }

    std::optional<SenderKey> get_key(const std::string& group_id) const {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = keys_.find(group_id);
        if (it != keys_.end()) return it->second;
        return std::nullopt;
    }

    bool is_current_epoch(const std::string& group_id, const std::string& epoch) const {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = keys_.find(group_id);
        if (it != keys_.end()) {
            return it->second.epoch == epoch;
        }
        return false;
    }

    std::size_t key_count() const {
        std::lock_guard<std::mutex> lock(mu_);
        return keys_.size();
    }
};

} // namespace garuda::richat
