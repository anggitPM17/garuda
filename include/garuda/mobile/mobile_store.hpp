#ifndef GARUDA_MOBILE_STORE_HPP
#define GARUDA_MOBILE_STORE_HPP

#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <chrono>
#include <iostream>
#include <algorithm>

namespace garuda {
namespace mobile {

struct PendingAction {
    uint64_t id = 0;
    std::string action_type;
    std::string payload;
    uint64_t timestamp_ms = 0;
    bool is_synced = false;
};

class GvmMobileStore {
private:
    std::map<std::string, std::string> kv_store_;
    std::vector<PendingAction> pending_queue_;
    uint64_t next_action_id_ = 1;
    bool is_online_ = true;
    mutable std::mutex store_mutex_;

public:
    GvmMobileStore() = default;

    void set_online_status(bool online) {
        std::lock_guard<std::mutex> lock(store_mutex_);
        is_online_ = online;
    }

    bool is_online() const {
        std::lock_guard<std::mutex> lock(store_mutex_);
        return is_online_;
    }

    void put(const std::string& key, const std::string& value) {
        std::lock_guard<std::mutex> lock(store_mutex_);
        kv_store_[key] = value;
    }

    bool get(const std::string& key, std::string& out_value) const {
        std::lock_guard<std::mutex> lock(store_mutex_);
        auto it = kv_store_.find(key);
        if (it != kv_store_.end()) {
            out_value = it->second;
            return true;
        }
        return false;
    }

    bool remove(const std::string& key) {
        std::lock_guard<std::mutex> lock(store_mutex_);
        return kv_store_.erase(key) > 0;
    }

    uint64_t queue_action(const std::string& action_type, const std::string& payload) {
        std::lock_guard<std::mutex> lock(store_mutex_);
        PendingAction act;
        act.id = next_action_id_++;
        act.action_type = action_type;
        act.payload = payload;
        act.timestamp_ms = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
        act.is_synced = false;

        pending_queue_.push_back(act);
        return act.id;
    }

    std::vector<PendingAction> get_pending_actions() const {
        std::lock_guard<std::mutex> lock(store_mutex_);
        std::vector<PendingAction> pending;
        for (const auto& act : pending_queue_) {
            if (!act.is_synced) {
                pending.push_back(act);
            }
        }
        return pending;
    }

    bool mark_action_synced(uint64_t action_id) {
        std::lock_guard<std::mutex> lock(store_mutex_);
        for (auto& act : pending_queue_) {
            if (act.id == action_id) {
                act.is_synced = true;
                return true;
            }
        }
        return false;
    }

    size_t clear_synced_actions() {
        std::lock_guard<std::mutex> lock(store_mutex_);
        size_t initial_count = pending_queue_.size();
        pending_queue_.erase(
            std::remove_if(pending_queue_.begin(), pending_queue_.end(),
                           [](const PendingAction& act) { return act.is_synced; }),
            pending_queue_.end()
        );
        return initial_count - pending_queue_.size();
    }

    size_t pending_count() const {
        std::lock_guard<std::mutex> lock(store_mutex_);
        size_t count = 0;
        for (const auto& act : pending_queue_) {
            if (!act.is_synced) count++;
        }
        return count;
    }
};

} // namespace mobile
} // namespace garuda

#endif // GARUDA_MOBILE_STORE_HPP
