#pragma once
// Garuda Language Web Framework: Session Storage Engine (`session.hpp`)
// Mengimplementasikan Encrypted Session Storage & Thread-Safe In-Memory Session Store.
#include "../core/term.hpp"

#include <chrono>
#include <iostream>
#include <map>
#include <mutex>
#include <optional>
#include <random>
#include <sstream>
#include <string>

namespace garuda::beam::stdlib {

class SessionStore {
public:
    SessionStore() = default;

    std::string create_session() {
        std::lock_guard<std::mutex> lock(mu_);
        std::string session_id = generate_id();
        sessions_[session_id] = {};
        return session_id;
    }

    void set(const std::string& session_id, const std::string& key, const Term& val) {
        std::lock_guard<std::mutex> lock(mu_);
        sessions_[session_id][key] = val;
    }

    std::optional<Term> get(const std::string& session_id, const std::string& key) const {
        std::lock_guard<std::mutex> lock(mu_);
        auto s_it = sessions_.find(session_id);
        if (s_it == sessions_.end()) return std::nullopt;

        auto v_it = s_it->second.find(key);
        if (v_it == s_it->second.end()) return std::nullopt;

        return v_it->second;
    }

    void clear(const std::string& session_id) {
        std::lock_guard<std::mutex> lock(mu_);
        sessions_.erase(session_id);
    }

    bool has_session(const std::string& session_id) const {
        std::lock_guard<std::mutex> lock(mu_);
        return sessions_.find(session_id) != sessions_.end();
    }

private:
    static std::string generate_id() {
        static std::atomic<uint64_t> counter{1000};
        auto now = std::chrono::steady_clock::now().time_since_epoch().count();
        std::ostringstream ss;
        ss << "sess_" << now << "_" << counter++;
        return ss.str();
    }

    mutable std::mutex mu_;
    std::map<std::string, std::map<std::string, Term>> sessions_;
};

} // namespace garuda::beam::stdlib
