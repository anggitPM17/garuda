#pragma once
// =====================================================================
// FASE G5.5 - Reaction: Message reactions (emoji)
// =====================================================================
#include <string>
#include <map>
#include <vector>
#include <mutex>
#include <optional>
#include <cstddef>

namespace garuda::beam {

struct Reaction {
    std::string message_id;
    std::string user_id;
    std::string emoji;
};

class ReactionStore {
private:
    std::map<std::string, std::map<std::string, std::string>> reactions_;
    // ^ message_id -> user_id -> emoji
    mutable std::mutex mu_;

public:
    void add_reaction(const std::string& message_id,
                      const std::string& user_id,
                      const std::string& emoji) {
        std::lock_guard<std::mutex> lock(mu_);
        reactions_[message_id][user_id] = emoji;
    }

    bool remove_reaction(const std::string& message_id,
                         const std::string& user_id) {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = reactions_.find(message_id);
        if (it == reactions_.end()) return false;

        bool erased = it->second.erase(user_id) > 0;
        if (it->second.empty()) {
            reactions_.erase(it);
        }
        return erased;
    }

    std::map<std::string, std::string> get_reactions(const std::string& message_id) const {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = reactions_.find(message_id);
        if (it != reactions_.end()) return it->second;
        return {};
    }

    std::size_t total_reaction_count(const std::string& message_id) const {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = reactions_.find(message_id);
        if (it != reactions_.end()) return it->second.size();
        return 0;
    }
};

} // namespace garuda::beam
