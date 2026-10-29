#pragma once
// =====================================================================
// A.2 - RICHAT Group Chat API
// =====================================================================
#include "richat_chat_api.hpp"
#include <string>
#include <vector>
#include <map>
#include <optional>
#include <algorithm>
#include <mutex>
#include <chrono>

namespace garuda::richat {

struct RichatGroup {
    std::string id;
    std::string name;
    std::string owner_id;
    std::vector<std::string> member_ids;
    std::int64_t created_at = 0;
};

class RichatGroupApi {
private:
    std::vector<RichatGroup> groups_;
    std::vector<ChatMessage> group_messages_;
    mutable std::mutex mu_;

public:
    RichatGroupApi() = default;

    std::optional<RichatGroup> create_group(
        const std::string& owner_id,
        const std::string& name,
        const std::vector<std::string>& member_ids) {
        if (owner_id.empty() || name.empty()) return std::nullopt;

        std::lock_guard<std::mutex> lock(mu_);
        RichatGroup grp;
        grp.id = "grp_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) +
                 "_" + std::to_string(groups_.size() + 1);
        grp.name = name;
        grp.owner_id = owner_id;
        grp.member_ids = member_ids;

        if (std::find(grp.member_ids.begin(), grp.member_ids.end(), owner_id) == grp.member_ids.end()) {
            grp.member_ids.push_back(owner_id);
        }

        grp.created_at = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();

        groups_.push_back(grp);
        return grp;
    }

    bool join_group(const std::string& group_id, const std::string& user_id) {
        if (group_id.empty() || user_id.empty()) return false;
        std::lock_guard<std::mutex> lock(mu_);

        for (auto& grp : groups_) {
            if (grp.id == group_id) {
                if (std::find(grp.member_ids.begin(), grp.member_ids.end(), user_id) == grp.member_ids.end()) {
                    grp.member_ids.push_back(user_id);
                }
                return true;
            }
        }
        return false;
    }

    bool leave_group(const std::string& group_id, const std::string& user_id) {
        if (group_id.empty() || user_id.empty()) return false;
        std::lock_guard<std::mutex> lock(mu_);

        for (auto& grp : groups_) {
            if (grp.id == group_id) {
                auto it = std::find(grp.member_ids.begin(), grp.member_ids.end(), user_id);
                if (it != grp.member_ids.end()) {
                    grp.member_ids.erase(it);
                    return true;
                }
            }
        }
        return false;
    }

    std::vector<RichatGroup> list_groups(const std::string& user_id) const {
        std::lock_guard<std::mutex> lock(mu_);
        std::vector<RichatGroup> out;

        for (const auto& grp : groups_) {
            if (std::find(grp.member_ids.begin(), grp.member_ids.end(), user_id) != grp.member_ids.end()) {
                out.push_back(grp);
            }
        }
        return out;
    }

    std::optional<ChatMessage> send_group_message(
        const std::string& group_id,
        const std::string& sender_id,
        const std::string& content) {
        if (group_id.empty() || sender_id.empty() || content.empty()) return std::nullopt;
        std::lock_guard<std::mutex> lock(mu_);

        // Verify sender is group member
        auto it = std::find_if(groups_.begin(), groups_.end(), [&](const RichatGroup& g) { return g.id == group_id; });
        if (it == groups_.end()) return std::nullopt;

        if (std::find(it->member_ids.begin(), it->member_ids.end(), sender_id) == it->member_ids.end()) {
            return std::nullopt; // Sender not in group
        }

        ChatMessage msg;
        msg.id = "gmsg_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
        msg.sender_id = sender_id;
        msg.recipient_id = group_id; // Recipient is group_id
        msg.content = content;
        msg.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        msg.status = "sent";

        group_messages_.push_back(msg);
        return msg;
    }

    std::vector<ChatMessage> get_group_messages(const std::string& group_id, std::size_t limit = 50) const {
        std::lock_guard<std::mutex> lock(mu_);
        std::vector<ChatMessage> out;

        for (auto it = group_messages_.rbegin(); it != group_messages_.rend() && out.size() < limit; ++it) {
            if (it->recipient_id == group_id) {
                out.push_back(*it);
            }
        }
        return out;
    }

    std::size_t group_count() const {
        std::lock_guard<std::mutex> lock(mu_);
        return groups_.size();
    }
};

} // namespace garuda::richat
