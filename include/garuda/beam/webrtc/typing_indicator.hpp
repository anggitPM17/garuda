#pragma once
// =====================================================================
// FASE G5.3 - Typing Indicator: Typing/recording status broadcast
// =====================================================================
#include <string>
#include <map>
#include <vector>
#include <mutex>
#include <optional>
#include <chrono>
#include <cstddef>

namespace garuda::beam {

struct TypingState {
    std::string user_id;
    std::string conversation_id;
    bool is_recording = false;
    std::chrono::steady_clock::time_point started_at;
    int ttl_seconds = 5;  // Auto-expire after 5s

    bool is_expired() const {
        if (ttl_seconds == 0) return true;
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::steady_clock::now() - started_at).count();
        return elapsed >= ttl_seconds;
    }
};

class TypingIndicator {
private:
    std::map<std::string, TypingState> states_;  // key: user_id:conversation_id
    mutable std::mutex mu_;

    static std::string make_key(const std::string& user_id,
                                const std::string& conversation_id) {
        return user_id + ":" + conversation_id;
    }

public:
    void start_typing(const std::string& user_id,
                      const std::string& conversation_id,
                      bool is_recording = false,
                      int ttl_seconds = 5) {
        std::lock_guard<std::mutex> lock(mu_);
        std::string key = make_key(user_id, conversation_id);
        TypingState st;
        st.user_id = user_id;
        st.conversation_id = conversation_id;
        st.is_recording = is_recording;
        st.started_at = std::chrono::steady_clock::now();
        st.ttl_seconds = ttl_seconds;
        states_[key] = st;
    }

    void stop_typing(const std::string& user_id,
                     const std::string& conversation_id) {
        std::lock_guard<std::mutex> lock(mu_);
        std::string key = make_key(user_id, conversation_id);
        states_.erase(key);
    }

    bool is_typing(const std::string& user_id,
                   const std::string& conversation_id) const {
        std::lock_guard<std::mutex> lock(mu_);
        std::string key = make_key(user_id, conversation_id);
        auto it = states_.find(key);
        if (it != states_.end()) {
            return !it->second.is_expired();
        }
        return false;
    }

    std::vector<std::string> get_typing_users(const std::string& conversation_id) const {
        std::lock_guard<std::mutex> lock(mu_);
        std::vector<std::string> users;
        for (const auto& [key, st] : states_) {
            if (st.conversation_id == conversation_id && !st.is_expired()) {
                users.push_back(st.user_id);
            }
        }
        return users;
    }

    std::size_t sweep_expired() {
        std::lock_guard<std::mutex> lock(mu_);
        std::size_t count = 0;
        for (auto it = states_.begin(); it != states_.end(); ) {
            if (it->second.is_expired()) {
                it = states_.erase(it);
                count++;
            } else {
                ++it;
            }
        }
        return count;
    }

    std::size_t active_count() const {
        std::lock_guard<std::mutex> lock(mu_);
        std::size_t count = 0;
        for (const auto& [key, st] : states_) {
            if (!st.is_expired()) count++;
        }
        return count;
    }
};

} // namespace garuda::beam
