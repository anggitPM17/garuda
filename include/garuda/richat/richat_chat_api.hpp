#pragma once
// =====================================================================
// A.1 - RICHAT Chat API: 1-on-1 Messaging
// =====================================================================
#include "richat_message_store.hpp"
#include <string>
#include <vector>
#include <optional>
#include <chrono>
#include <algorithm>
#include <mutex>

namespace garuda::richat {

struct ChatMessage {
    std::string id;
    std::string sender_id;
    std::string recipient_id;
    std::string content;
    std::int64_t timestamp = 0;
    std::string status = "sent"; // "sent", "delivered", "read", "deleted"
};

class RichatChatApi {
private:
    std::vector<ChatMessage> messages_;
    mutable std::mutex mu_;

public:
    RichatChatApi() = default;

    std::optional<ChatMessage> send_message(
        const std::string& sender_id,
        const std::string& recipient_id,
        const std::string& content) {
        if (sender_id.empty() || recipient_id.empty() || content.empty()) {
            return std::nullopt;
        }

        std::lock_guard<std::mutex> lock(mu_);
        ChatMessage msg;
        msg.id = "msg_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) +
                 "_" + std::to_string(messages_.size() + 1);
        msg.sender_id = sender_id;
        msg.recipient_id = recipient_id;
        msg.content = content;
        msg.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        msg.status = "sent";

        messages_.push_back(msg);
        return msg;
    }

    std::vector<ChatMessage> get_messages(
        const std::string& user_id,
        const std::string& with_user_id,
        std::size_t limit = 50,
        std::int64_t /*before_timestamp*/ = 0) const {
        std::lock_guard<std::mutex> lock(mu_);
        std::vector<ChatMessage> out;

        for (auto it = messages_.rbegin(); it != messages_.rend() && out.size() < limit; ++it) {
            if ((it->sender_id == user_id && it->recipient_id == with_user_id) ||
                (it->sender_id == with_user_id && it->recipient_id == user_id)) {
                if (it->status != "deleted") {
                    out.push_back(*it);
                }
            }
        }
        return out;
    }

    std::optional<ChatMessage> get_message(const std::string& message_id) const {
        std::lock_guard<std::mutex> lock(mu_);
        for (const auto& msg : messages_) {
            if (msg.id == message_id) {
                return msg;
            }
        }
        return std::nullopt;
    }

    bool delete_message(const std::string& message_id, const std::string& user_id) {
        std::lock_guard<std::mutex> lock(mu_);
        for (auto& msg : messages_) {
            if (msg.id == message_id && (msg.sender_id == user_id || msg.recipient_id == user_id)) {
                msg.status = "deleted";
                return true;
            }
        }
        return false;
    }

    std::size_t total_count() const {
        std::lock_guard<std::mutex> lock(mu_);
        return messages_.size();
    }
};

} // namespace garuda::richat
