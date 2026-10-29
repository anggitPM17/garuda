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

    // --- Room Management ---
    std::string create_room(const std::string& creator_id, const std::string& room_name) {
        std::lock_guard<std::mutex> lock(mutex_);
        std::string room_id = generate_id("room");
        Row row;
        row.columns["room_id"] = room_id;
        row.columns["room_name"] = room_name;
        row.columns["creator_id"] = creator_id;
        row.columns["created_at"] = static_cast<std::int64_t>(std::time(nullptr));
        db_.get_storage().put("rooms", room_id, row);

        // Creator otomatis jadi member
        std::string member_key = room_id + ":" + creator_id;
        Row mrow;
        mrow.columns["room_id"] = room_id;
        mrow.columns["user_id"] = creator_id;
        mrow.columns["joined_at"] = static_cast<std::int64_t>(std::time(nullptr));
        db_.get_storage().put("room_members", member_key, mrow);

        return room_id;
    }

    void join_room(const std::string& user_id, const std::string& room_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        std::string member_key = room_id + ":" + user_id;
        Row row;
        row.columns["room_id"] = room_id;
        row.columns["user_id"] = user_id;
        row.columns["joined_at"] = static_cast<std::int64_t>(std::time(nullptr));
        db_.get_storage().put("room_members", member_key, row);
    }

    void leave_room(const std::string& user_id, const std::string& room_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        (void)user_id;
        (void)room_id;
    }

    std::vector<std::string> get_room_members(const std::string& room_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<std::string> members;
        auto all_members = db_.get_storage().scan("room_members");
        for (const auto& m : all_members) {
            auto r_it = m.columns.find("room_id");
            auto u_it = m.columns.find("user_id");
            if (r_it != m.columns.end() && u_it != m.columns.end()) {
                if (std::get<std::string>(r_it->second) == room_id) {
                    members.push_back(std::get<std::string>(u_it->second));
                }
            }
        }
        return members;
    }

    std::string save_room_message(const std::string& sender_id, const std::string& room_id, const std::string& content, const std::string& client_msg_id) {
        std::lock_guard<std::mutex> lock(mutex_);

        std::string dedup_key = sender_id + ":" + client_msg_id;
        auto existing = db_.get_storage().get("dedup_index", dedup_key);
        if (existing.has_value()) {
            return std::get<std::string>(existing->columns["message_id"]);
        }

        std::string message_id = generate_id("rmsg");
        Row row;
        row.columns["message_id"] = message_id;
        row.columns["room_id"] = room_id;
        row.columns["sender_id"] = sender_id;
        row.columns["content"] = content;
        row.columns["timestamp"] = static_cast<std::int64_t>(std::time(nullptr));
        row.columns["status"] = std::string("sent");

        db_.get_storage().put("room_messages", message_id, row);

        Row dedup_row;
        dedup_row.columns["message_id"] = message_id;
        db_.get_storage().put("dedup_index", dedup_key, dedup_row);

        return message_id;
    }

    std::vector<Row> get_room_messages(const std::string& room_id, int limit = 50) {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<Row> result;
        auto all_messages = db_.get_storage().scan("room_messages");

        for (const auto& msg : all_messages) {
            auto r_it = msg.columns.find("room_id");
            if (r_it != msg.columns.end() && std::get<std::string>(r_it->second) == room_id) {
                result.push_back(msg);
            }
        }

        std::sort(result.begin(), result.end(), [](const Row& a, const Row& b) {
            return std::get<std::int64_t>(a.columns.at("timestamp")) > std::get<std::int64_t>(b.columns.at("timestamp"));
        });

        if (result.size() > static_cast<size_t>(limit)) result.resize(limit);
        return result;
    }

    // =========================================================
    // R4 - STATUS & MOMENT MANAGEMENT
    // =========================================================

    // --- Status (Hilang setelah 24 jam) ---
    std::string create_status(const std::string& user_id, const std::string& content, const std::string& media_url) {
        std::lock_guard<std::mutex> lock(mutex_);
        std::string status_id = generate_id("status");
        Row row;
        row.columns["status_id"] = status_id;
        row.columns["user_id"] = user_id;
        row.columns["content"] = content;
        row.columns["media_url"] = media_url;
        row.columns["created_at"] = static_cast<std::int64_t>(std::time(nullptr));
        row.columns["expires_at"] = static_cast<std::int64_t>(std::time(nullptr) + 86400); // 24 jam
        db_.get_storage().put("statuses", status_id, row);
        return status_id;
    }

    std::vector<Row> get_statuses(const std::string& user_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        (void)user_id;
        std::vector<Row> result;
        auto all_statuses = db_.get_storage().scan("statuses");
        std::int64_t now = static_cast<std::int64_t>(std::time(nullptr));

        for (const auto& s : all_statuses) {
            auto e_it = s.columns.find("expires_at");
            if (e_it == s.columns.end()) continue;

            if (std::get<std::int64_t>(e_it->second) > now) {
                result.push_back(s);
            }
        }
        return result;
    }

    // --- Moment (Feed/Story) ---
    std::string create_moment(const std::string& user_id, const std::string& media_url,
                              const std::string& caption, const std::string& visibility) {
        std::lock_guard<std::mutex> lock(mutex_);
        std::string moment_id = generate_id("moment");
        Row row;
        row.columns["moment_id"] = moment_id;
        row.columns["user_id"] = user_id;
        row.columns["media_url"] = media_url;
        row.columns["caption"] = caption;
        row.columns["visibility"] = visibility; // "public", "friends", "private"
        row.columns["created_at"] = static_cast<std::int64_t>(std::time(nullptr));
        db_.get_storage().put("moments", moment_id, row);
        return moment_id;
    }

    std::vector<Row> get_moments(const std::string& user_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<Row> result;
        auto all_moments = db_.get_storage().scan("moments");

        for (const auto& m : all_moments) {
            auto v_it = m.columns.find("visibility");
            if (v_it == m.columns.end()) continue;

            std::string vis = std::get<std::string>(v_it->second);
            auto u_it = m.columns.find("user_id");
            if (vis == "public" || (u_it != m.columns.end() && std::get<std::string>(u_it->second) == user_id)) {
                result.push_back(m);
            }
        }

        std::sort(result.begin(), result.end(), [](const Row& a, const Row& b) {
            return std::get<std::int64_t>(a.columns.at("created_at")) > std::get<std::int64_t>(b.columns.at("created_at"));
        });
        return result;
    }

    bool like_moment(const std::string& user_id, const std::string& moment_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        std::string like_key = moment_id + ":" + user_id;

        auto existing = db_.get_storage().get("moment_likes", like_key);
        if (existing.has_value()) return false;

        Row row;
        row.columns["moment_id"] = moment_id;
        row.columns["user_id"] = user_id;
        row.columns["created_at"] = static_cast<std::int64_t>(std::time(nullptr));
        db_.get_storage().put("moment_likes", like_key, row);
        return true;
    }

    int get_like_count(const std::string& moment_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        int count = 0;
        auto all_likes = db_.get_storage().scan("moment_likes");
        for (const auto& l : all_likes) {
            auto m_it = l.columns.find("moment_id");
            if (m_it != l.columns.end() && std::get<std::string>(m_it->second) == moment_id) {
                count++;
            }
        }
        return count;
    }

    // =========================================================
    // R5 - CALL LOG MANAGEMENT
    // =========================================================

    std::string start_call(const std::string& caller_id, const std::string& receiver_id, const std::string& type) {
        std::lock_guard<std::mutex> lock(mutex_);
        std::string call_id = generate_id("call");
        Row row;
        row.columns["call_id"] = call_id;
        row.columns["caller_id"] = caller_id;
        row.columns["receiver_id"] = receiver_id;
        row.columns["type"] = type; // "audio" atau "video"
        row.columns["status"] = std::string("ringing"); // ringing, answered, missed, rejected
        row.columns["start_time"] = static_cast<std::int64_t>(std::time(nullptr));
        row.columns["end_time"] = static_cast<std::int64_t>(0); // 0 = belum selesai
        row.columns["duration_seconds"] = static_cast<std::int64_t>(0);
        db_.get_storage().put("call_logs", call_id, row);
        return call_id;
    }

    bool end_call(const std::string& call_id, const std::string& status) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto call = db_.get_storage().get("call_logs", call_id);
        if (!call.has_value()) return false;

        std::int64_t now = static_cast<std::int64_t>(std::time(nullptr));
        std::int64_t start_time = std::get<std::int64_t>(call->columns["start_time"]);

        call->columns["end_time"] = now;
        call->columns["status"] = status; // "answered", "missed", "rejected"
        call->columns["duration_seconds"] = now - start_time;

        db_.get_storage().put("call_logs", call_id, *call);
        return true;
    }

    std::vector<Row> get_call_logs(const std::string& user_id, int limit = 50) {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<Row> result;
        auto all_calls = db_.get_storage().scan("call_logs");

        for (const auto& c : all_calls) {
            auto caller_it = c.columns.find("caller_id");
            auto receiver_it = c.columns.find("receiver_id");
            if (caller_it == c.columns.end() || receiver_it == c.columns.end()) continue;

            if (std::get<std::string>(caller_it->second) == user_id ||
                std::get<std::string>(receiver_it->second) == user_id) {
                result.push_back(c);
            }
        }

        std::sort(result.begin(), result.end(), [](const Row& a, const Row& b) {
            return std::get<std::int64_t>(a.columns.at("start_time")) > std::get<std::int64_t>(b.columns.at("start_time"));
        });

        if (result.size() > static_cast<size_t>(limit)) {
            result.resize(limit);
        }
        return result;
    }
};

} // namespace garuda::richat
