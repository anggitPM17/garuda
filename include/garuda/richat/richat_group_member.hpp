#pragma once
// =====================================================================
// FASE G8.2 - RICHAT Group Member (setara richat_group_member.erl)
// =====================================================================
#include <string>
#include <map>
#include <set>
#include <vector>
#include <mutex>
#include <optional>
#include <chrono>
#include <cstddef>

namespace garuda::richat {

enum class MemberRole { OWNER, ADMIN, MEMBER };

struct GroupMember {
    std::string group_id;
    std::string user_id;
    MemberRole role = MemberRole::MEMBER;
    std::chrono::steady_clock::time_point joined_at;
};

class RichatGroupMember {
private:
    std::map<std::string, std::map<std::string, GroupMember>> groups_;
    // ^ group_id -> user_id -> GroupMember
    mutable std::mutex mu_;

public:
    bool add_member(const std::string& group_id,
                    const std::string& user_id,
                    MemberRole role = MemberRole::MEMBER) {
        if (group_id.empty() || user_id.empty()) return false;

        std::lock_guard<std::mutex> lock(mu_);
        if (groups_[group_id].find(user_id) != groups_[group_id].end()) {
            return false; // Duplicate
        }

        GroupMember gm;
        gm.group_id = group_id;
        gm.user_id = user_id;
        gm.role = role;
        gm.joined_at = std::chrono::steady_clock::now();

        groups_[group_id][user_id] = gm;
        return true;
    }

    bool remove_member(const std::string& group_id, const std::string& user_id) {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = groups_.find(group_id);
        if (it == groups_.end()) return false;

        bool erased = it->second.erase(user_id) > 0;
        if (it->second.empty()) {
            groups_.erase(it);
        }
        return erased;
    }

    bool is_member(const std::string& group_id, const std::string& user_id) const {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = groups_.find(group_id);
        if (it != groups_.end()) {
            return it->second.find(user_id) != it->second.end();
        }
        return false;
    }

    std::optional<GroupMember> get_member(const std::string& group_id, const std::string& user_id) const {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = groups_.find(group_id);
        if (it != groups_.end()) {
            auto m_it = it->second.find(user_id);
            if (m_it != it->second.end()) {
                return m_it->second;
            }
        }
        return std::nullopt;
    }

    std::vector<GroupMember> list_members(const std::string& group_id) const {
        std::lock_guard<std::mutex> lock(mu_);
        std::vector<GroupMember> out;
        auto it = groups_.find(group_id);
        if (it != groups_.end()) {
            for (const auto& [uid, gm] : it->second) {
                out.push_back(gm);
            }
        }
        return out;
    }

    bool set_role(const std::string& group_id, const std::string& user_id, MemberRole role) {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = groups_.find(group_id);
        if (it != groups_.end()) {
            auto m_it = it->second.find(user_id);
            if (m_it != it->second.end()) {
                m_it->second.role = role;
                return true;
            }
        }
        return false;
    }

    std::size_t member_count(const std::string& group_id) const {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = groups_.find(group_id);
        if (it != groups_.end()) return it->second.size();
        return 0;
    }

    std::size_t group_count() const {
        std::lock_guard<std::mutex> lock(mu_);
        return groups_.size();
    }

    bool can_moderate(const std::string& group_id, const std::string& user_id) const {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = groups_.find(group_id);
        if (it != groups_.end()) {
            auto m_it = it->second.find(user_id);
            if (m_it != it->second.end()) {
                return m_it->second.role == MemberRole::OWNER || m_it->second.role == MemberRole::ADMIN;
            }
        }
        return false;
    }
};

} // namespace garuda::richat
