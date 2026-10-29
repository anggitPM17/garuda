#pragma once
// =====================================================================
// A.3 - RICHAT Real-time API (Presence, Typing, Receipt, Reaction)
// =====================================================================
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <algorithm>

namespace garuda::richat {

class RichatRealtime {
private:
    std::map<std::string, bool> presence_; // user_id -> online
    std::map<std::string, std::vector<std::string>> typing_; // conversation_id -> user_ids
    std::map<std::string, std::string> receipts_; // message_id -> status ("sent", "delivered", "read")
    std::map<std::string, std::map<std::string, std::string>> reactions_; // message_id -> (user_id -> emoji)
    mutable std::mutex mu_;

public:
    RichatRealtime() = default;

    // Presence
    void set_online(const std::string& user_id) {
        if (user_id.empty()) return;
        std::lock_guard<std::mutex> lock(mu_);
        presence_[user_id] = true;
    }

    void set_offline(const std::string& user_id) {
        if (user_id.empty()) return;
        std::lock_guard<std::mutex> lock(mu_);
        presence_[user_id] = false;
    }

    std::vector<std::string> get_online_users() const {
        std::lock_guard<std::mutex> lock(mu_);
        std::vector<std::string> out;
        for (const auto& [uid, is_on] : presence_) {
            if (is_on) out.push_back(uid);
        }
        return out;
    }

    // Typing Indicator
    void start_typing(const std::string& user_id, const std::string& conversation_id) {
        if (user_id.empty() || conversation_id.empty()) return;
        std::lock_guard<std::mutex> lock(mu_);
        auto& list = typing_[conversation_id];
        if (std::find(list.begin(), list.end(), user_id) == list.end()) {
            list.push_back(user_id);
        }
    }

    void stop_typing(const std::string& user_id, const std::string& conversation_id) {
        if (user_id.empty() || conversation_id.empty()) return;
        std::lock_guard<std::mutex> lock(mu_);
        auto& list = typing_[conversation_id];
        auto it = std::find(list.begin(), list.end(), user_id);
        if (it != list.end()) {
            list.erase(it);
        }
    }

    std::vector<std::string> get_typing_users(const std::string& conversation_id) const {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = typing_.find(conversation_id);
        if (it != typing_.end()) return it->second;
        return {};
    }

    // Read Receipt
    void mark_delivered(const std::string& message_id) {
        if (message_id.empty()) return;
        std::lock_guard<std::mutex> lock(mu_);
        receipts_[message_id] = "delivered";
    }

    void mark_read(const std::string& message_id) {
        if (message_id.empty()) return;
        std::lock_guard<std::mutex> lock(mu_);
        receipts_[message_id] = "read";
    }

    std::string get_receipt_status(const std::string& message_id) const {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = receipts_.find(message_id);
        if (it != receipts_.end()) return it->second;
        return "sent";
    }

    // Reaction
    void add_reaction(const std::string& message_id, const std::string& user_id, const std::string& emoji) {
        if (message_id.empty() || user_id.empty() || emoji.empty()) return;
        std::lock_guard<std::mutex> lock(mu_);
        reactions_[message_id][user_id] = emoji;
    }

    void remove_reaction(const std::string& message_id, const std::string& user_id) {
        if (message_id.empty() || user_id.empty()) return;
        std::lock_guard<std::mutex> lock(mu_);
        reactions_[message_id].erase(user_id);
    }

    std::map<std::string, std::string> get_reactions(const std::string& message_id) const {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = reactions_.find(message_id);
        if (it != reactions_.end()) return it->second;
        return {};
    }
};

} // namespace garuda::richat
