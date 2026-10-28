#pragma once
// =====================================================================
// R1 - RICHAT MESSAGE STORE
// Wrapper untuk interaksi dengan Jalur Langit (users, messages, sessions).
// =====================================================================
#include "../jalur_langit/jalur_langit_ultimate.hpp"
#include <string>
#include <vector>
#include <chrono>
#include <ctime>
#include <mutex>
#include <optional>
#include <algorithm>

namespace garuda::richat {

using namespace garuda::jalur_langit;

class RichatMessageStore {
private:
    JalurLangitUltimate& db_;
    std::mutex mutex_;

    // Helper: generate unique ID
    std::string generate_id(const std::string& prefix) {
        auto now = std::chrono::steady_clock::now().time_since_epoch().count();
        return prefix + "_" + std::to_string(now);
    }

public:
    explicit RichatMessageStore(JalurLangitUltimate& db) : db_(db) {}

    // --- User Management ---
    void save_user(const std::string& user_id, const std::string& username,
                   const std::string& auth_token, const std::string& device_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        Row row;
        row.columns["user_id"] = user_id;
        row.columns["username"] = username;
        row.columns["auth_token"] = auth_token;
        row.columns["device_id"] = device_id;
        row.columns["last_seen"] = static_cast<std::int64_t>(std::time(nullptr));
        db_.get_storage().put("users", user_id, row);
    }

    std::optional<Row> get_user(const std::string& user_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        return db_.get_storage().get("users", user_id);
    }

    // --- Session Management ---
    void save_session(const std::string& session_token, const std::string& user_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        Row row;
        row.columns["session_token"] = session_token;
        row.columns["user_id"] = user_id;
        row.columns["created_at"] = static_cast<std::int64_t>(std::time(nullptr));
        row.columns["expires_at"] = static_cast<std::int64_t>(std::time(nullptr) + 86400); // 1 hari
        db_.get_storage().put("sessions", session_token, row);
    }

    std::optional<Row> get_session(const std::string& session_token) {
        std::lock_guard<std::mutex> lock(mutex_);
        return db_.get_storage().get("sessions", session_token);
    }

    // --- Message Management ---
    std::string save_message(const std::string& sender_id, const std::string& receiver_id,
                             const std::string& content, const std::string& client_msg_id) {
        std::lock_guard<std::mutex> lock(mutex_);

        // Deduplikasi: cek apakah client_msg_id sudah ada untuk sender ini
        std::string dedup_key = sender_id + ":" + client_msg_id;
        auto existing = db_.get_storage().get("dedup_index", dedup_key);
        if (existing.has_value()) {
            return std::get<std::string>(existing->columns["message_id"]);
        }

        std::string message_id = generate_id("msg");
        Row row;
        row.columns["message_id"] = message_id;
        row.columns["sender_id"] = sender_id;
        row.columns["receiver_id"] = receiver_id;
        row.columns["content"] = content;
        row.columns["client_msg_id"] = client_msg_id;
        row.columns["timestamp"] = static_cast<std::int64_t>(std::time(nullptr));
        row.columns["status"] = std::string("sent");

        db_.get_storage().put("messages", message_id, row);

        // Simpan dedup index
        Row dedup_row;
        dedup_row.columns["message_id"] = message_id;
        db_.get_storage().put("dedup_index", dedup_key, dedup_row);

        return message_id;
    }

    std::vector<Row> get_messages(const std::string& user_a, const std::string& user_b, int limit = 50) {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<Row> result;
        auto all_messages = db_.get_storage().scan("messages");

        for (const auto& msg : all_messages) {
            auto s_it = msg.columns.find("sender_id");
            auto r_it = msg.columns.find("receiver_id");
            if (s_it == msg.columns.end() || r_it == msg.columns.end()) continue;

            std::string sender = std::get<std::string>(s_it->second);
            std::string receiver = std::get<std::string>(r_it->second);

            if ((sender == user_a && receiver == user_b) || (sender == user_b && receiver == user_a)) {
                result.push_back(msg);
            }
        }

        // Sort by timestamp descending
        std::sort(result.begin(), result.end(), [](const Row& a, const Row& b) {
            auto ta = std::get<std::int64_t>(a.columns.at("timestamp"));
            auto tb = std::get<std::int64_t>(b.columns.at("timestamp"));
            return ta > tb;
        });

        if (result.size() > static_cast<size_t>(limit)) {
            result.resize(limit);
        }
        return result;
    }

    void update_message_status(const std::string& message_id, const std::string& status) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto msg = db_.get_storage().get("messages", message_id);
        if (msg.has_value()) {
            msg->columns["status"] = status;
            db_.get_storage().put("messages", message_id, *msg);
        }
    }
};

} // namespace garuda::richat
