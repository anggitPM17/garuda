#pragma once
// =====================================================================
// FASE C.5.3 - CAPTCHA Proof-of-Work (Hashcash style, no external dependency)
// =====================================================================
#include "../beam/stdlib/crypto.hpp"
#include <string>
#include <mutex>
#include <map>
#include <chrono>
#include <random>
#include <algorithm>
#include <cstddef>

namespace garuda::security {

struct Challenge {
    std::string id;
    std::string prefix;
    int difficulty = 3; // Leading zeroes
    std::chrono::steady_clock::time_point created_at;
    std::chrono::seconds ttl{300}; // 5 min TTL
};

class CaptchaPow {
private:
    std::map<std::string, Challenge> challenges_;
    mutable std::mutex mu_;

    static std::string random_hex(std::size_t len = 8) {
        static const char hex_chars[] = "0123456789abcdef";
        static std::mt19937 rng(static_cast<unsigned>(std::chrono::system_clock::now().time_since_epoch().count()));
        std::uniform_int_distribution<std::size_t> dist(0, 15);

        std::string out;
        out.reserve(len);
        for (std::size_t i = 0; i < len; ++i) {
            out += hex_chars[dist(rng)];
        }
        return out;
    }

public:
    Challenge generate_challenge(int difficulty = 3) {
        std::lock_guard<std::mutex> lock(mu_);
        Challenge ch;
        ch.id = "ch_" + random_hex(8);
        ch.prefix = "richat_" + random_hex(8);
        ch.difficulty = difficulty;
        ch.created_at = std::chrono::steady_clock::now();
        ch.ttl = std::chrono::seconds(300);

        challenges_[ch.id] = ch;
        return ch;
    }

    bool verify(const std::string& challenge_id, const std::string& nonce) {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = challenges_.find(challenge_id);
        if (it == challenges_.end()) return false;

        auto now = std::chrono::steady_clock::now();
        if ((now - it->second.created_at) > it->second.ttl) {
            challenges_.erase(it);
            return false; // Expired
        }

        std::string hash = garuda::beam::stdlib::Sha256::hash(it->second.prefix + nonce);
        std::string target_prefix(static_cast<std::size_t>(it->second.difficulty), '0');

        if (hash.compare(0, it->second.difficulty, target_prefix) == 0) {
            challenges_.erase(it); // One-time use
            return true;
        }
        return false;
    }

    std::size_t cleanup_expired() {
        std::lock_guard<std::mutex> lock(mu_);
        auto now = std::chrono::steady_clock::now();
        std::size_t count = 0;

        for (auto it = challenges_.begin(); it != challenges_.end(); ) {
            if ((now - it->second.created_at) > it->second.ttl) {
                it = challenges_.erase(it);
                count++;
            } else {
                ++it;
            }
        }
        return count;
    }

    std::size_t active_count() const {
        std::lock_guard<std::mutex> lock(mu_);
        return challenges_.size();
    }
};

} // namespace garuda::security
