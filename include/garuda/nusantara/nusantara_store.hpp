#pragma once
#include "nusantara_protocol.hpp"
#include "sqlite3.h"
#include <string>
#include <vector>
#include <utility>
#include <optional>
#include <iostream>
#include <cstdint>
#include <ctime>

namespace garuda::nusantara {

class NusantaraMessageStore {
private:
    sqlite3* db_ = nullptr;

public:
    NusantaraMessageStore() = default;

    ~NusantaraMessageStore() {
        close();
    }

    NusantaraMessageStore(NusantaraMessageStore&& other) noexcept : db_(other.db_) {
        other.db_ = nullptr;
    }

    NusantaraMessageStore& operator=(NusantaraMessageStore&& other) noexcept {
        if (this != &other) {
            close();
            db_ = other.db_;
            other.db_ = nullptr;
        }
        return *this;
    }

    NusantaraMessageStore(const NusantaraMessageStore&) = delete;
    NusantaraMessageStore& operator=(const NusantaraMessageStore&) = delete;

    bool is_open() const { return db_ != nullptr; }

    bool open(const std::string& db_path) {
        close();
        int rc = sqlite3_open(db_path.c_str(), &db_);
        if (rc != SQLITE_OK || !db_) {
            if (db_) {
                sqlite3_close(db_);
                db_ = nullptr;
            }
            return false;
        }

        char* err_msg = nullptr;
        rc = sqlite3_exec(db_, "PRAGMA journal_mode=WAL; PRAGMA synchronous=NORMAL;", nullptr, nullptr, &err_msg);
        if (rc != SQLITE_OK) {
            if (err_msg) sqlite3_free(err_msg);
            sqlite3_close(db_);
            db_ = nullptr;
            return false;
        }

        const char* schema = R"(
            CREATE TABLE IF NOT EXISTS nusantara_messages (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                message_id TEXT NOT NULL,
                sender TEXT NOT NULL,
                receiver TEXT NOT NULL,
                action TEXT NOT NULL,
                payload_json TEXT NOT NULL,
                status TEXT NOT NULL DEFAULT 'pending',
                created_at INTEGER NOT NULL,
                completed_at INTEGER
            );
            CREATE INDEX IF NOT EXISTS idx_status_created ON nusantara_messages(status, created_at);
        )";

        rc = sqlite3_exec(db_, schema, nullptr, nullptr, &err_msg);
        if (rc != SQLITE_OK) {
            if (err_msg) sqlite3_free(err_msg);
            sqlite3_close(db_);
            db_ = nullptr;
            return false;
        }

        return true;
    }

    void close() {
        if (db_) {
            sqlite3_close(db_);
            db_ = nullptr;
        }
    }

    std::int64_t store_message(const NusantaraMessage& msg) {
        if (!db_) return -1;
        const char* sql = "INSERT INTO nusantara_messages "
                          "(message_id, sender, receiver, action, payload_json, status, created_at) "
                          "VALUES (?, ?, ?, ?, ?, 'pending', ?);";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return -1;

        std::string payload = msg.to_json();
        std::int64_t now = static_cast<std::int64_t>(std::time(nullptr));

        sqlite3_bind_text(stmt, 1, msg.message_id.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, to_string(msg.sender), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, to_string(msg.receiver), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, to_string(msg.action), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 5, payload.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, 6, now);

        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        if (rc != SQLITE_DONE) return -1;

        return static_cast<std::int64_t>(sqlite3_last_insert_rowid(db_));
    }

    bool mark_completed(std::int64_t id) {
        if (!db_ || id < 0) return false;
        const char* sql = "UPDATE nusantara_messages SET status = 'done', completed_at = ? WHERE id = ?;";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

        std::int64_t now = static_cast<std::int64_t>(std::time(nullptr));
        sqlite3_bind_int64(stmt, 1, now);
        sqlite3_bind_int64(stmt, 2, id);

        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        return (rc == SQLITE_DONE);
    }

    std::vector<std::pair<std::int64_t, NusantaraMessage>> load_pending() {
        std::vector<std::pair<std::int64_t, NusantaraMessage>> results;
        if (!db_) return results;

        const char* sql = "SELECT id, payload_json FROM nusantara_messages WHERE status = 'pending' ORDER BY id ASC;";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return results;

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::int64_t id = static_cast<std::int64_t>(sqlite3_column_int64(stmt, 0));
            const unsigned char* payload_txt = sqlite3_column_text(stmt, 1);
            if (payload_txt) {
                std::string json_str(reinterpret_cast<const char*>(payload_txt));
                auto msg = NusantaraMessage::from_json(json_str);
                if (msg.has_value()) {
                    results.push_back({id, *msg});
                }
            }
        }

        sqlite3_finalize(stmt);
        return results;
    }

    std::size_t prune_completed(int retention_seconds = 3600) {
        if (!db_) return 0;
        const char* sql = "DELETE FROM nusantara_messages WHERE status = 'done' AND completed_at < ?;";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return 0;

        std::int64_t threshold = static_cast<std::int64_t>(std::time(nullptr)) - retention_seconds;
        sqlite3_bind_int64(stmt, 1, threshold);

        int rc = sqlite3_step(stmt);
        std::size_t deleted = 0;
        if (rc == SQLITE_DONE) {
            deleted = static_cast<std::size_t>(sqlite3_changes(db_));
        }
        sqlite3_finalize(stmt);
        return deleted;
    }

    std::size_t pending_count() const {
        if (!db_) return 0;
        const char* sql = "SELECT COUNT(*) FROM nusantara_messages WHERE status = 'pending';";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return 0;

        std::size_t count = 0;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = static_cast<std::size_t>(sqlite3_column_int64(stmt, 0));
        }
        sqlite3_finalize(stmt);
        return count;
    }
};

} // namespace garuda::nusantara
