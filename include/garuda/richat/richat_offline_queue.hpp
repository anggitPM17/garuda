#pragma once
// =====================================================================
// A.4 - RICHAT Offline Queue & Sync Engine
// =====================================================================
#include "richat_chat_api.hpp"
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <algorithm>

namespace garuda::richat {

struct QueuedMessage {
    std::string id;
    std::string sender_id;
    std::string recipient_id;
    std::string content;
    std::int64_t created_at = 0;
    int retry_count = 0;
};

class RichatOfflineQueue {
private:
    std::map<std::string, QueuedMessage> queue_;
    mutable std::mutex mu_;

public:
    explicit RichatOfflineQueue(const std::string& /*db_path*/ = ":memory:") {}

    bool enqueue(const QueuedMessage& msg) {
        if (msg.id.empty() || msg.sender_id.empty() || msg.recipient_id.empty()) return false;
        std::lock_guard<std::mutex> lock(mu_);
        queue_[msg.id] = msg;
        return true;
    }

    std::vector<QueuedMessage> get_pending(std::size_t limit = 100) const {
        std::lock_guard<std::mutex> lock(mu_);
        std::vector<QueuedMessage> out;
        for (const auto& [id, msg] : queue_) {
            out.push_back(msg);
            if (out.size() >= limit) break;
        }
        return out;
    }

    bool mark_sent(const std::string& message_id) {
        std::lock_guard<std::mutex> lock(mu_);
        return queue_.erase(message_id) > 0;
    }

    bool increment_retry(const std::string& message_id) {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = queue_.find(message_id);
        if (it != queue_.end()) {
            it->second.retry_count++;
            return true;
        }
        return false;
    }

    std::size_t pending_count() const {
        std::lock_guard<std::mutex> lock(mu_);
        return queue_.size();
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mu_);
        queue_.clear();
    }
};

} // namespace garuda::richat
