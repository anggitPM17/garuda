#pragma once
// =====================================================================
// GPM — GARUDA PACKAGE MANAGER & REGISTRY
// Package Registry Service with SQLite Persistence
// =====================================================================

#include "sqlite3.h"
#include "../sys/json_logger.hpp"
#include <string>
#include <vector>
#include <mutex>
#include <sstream>
#include <iostream>
#include <algorithm>

namespace garuda::gpm {

struct PackageInfo {
    int id = 0;
    std::string name;
    std::string version;
    std::string description;
    std::string author;
    int downloads = 0;
    std::string created_at;
};

class GpmRegistry {
private:
    sqlite3* db_ = nullptr;
    mutable std::mutex mutex_;

    GpmRegistry() = default;

    ~GpmRegistry() {
        if (db_) {
            sqlite3_close(db_);
            db_ = nullptr;
        }
    }

    void init_db() {
        if (db_) return;
        int rc = sqlite3_open("/var/lib/richat-garuda/gpm_registry.db", &db_);
        if (rc != SQLITE_OK && db_) {
            sqlite3_close(db_);
            db_ = nullptr;
            rc = sqlite3_open("gpm_registry.db", &db_);
        }
        if (rc == SQLITE_OK && db_) {
            const char* sql = "CREATE TABLE IF NOT EXISTS packages ("
                              "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                              "name TEXT UNIQUE NOT NULL, "
                              "version TEXT NOT NULL, "
                              "description TEXT, "
                              "author TEXT, "
                              "downloads INTEGER DEFAULT 0, "
                              "created_at DATETIME DEFAULT CURRENT_TIMESTAMP);";
            char* err_msg = nullptr;
            sqlite3_exec(db_, sql, nullptr, nullptr, &err_msg);
            if (err_msg) {
                sqlite3_free(err_msg);
            }

            // Pre-seed standard Garuda libraries if empty
            seed_initial_packages();
        }
    }

    void seed_initial_packages() {
        const char* check_sql = "SELECT COUNT(*) FROM packages;";
        sqlite3_stmt* stmt = nullptr;
        int count = 0;
        if (sqlite3_prepare_v2(db_, check_sql, -1, &stmt, nullptr) == SQLITE_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                count = sqlite3_column_int(stmt, 0);
            }
            sqlite3_finalize(stmt);
        }

        if (count == 0) {
            publish_package("garuda_json", "1.2.0", "Native high-performance JSON parser & encoder for GVM Actor model.", "Garuda Team");
            publish_package("garuda_http", "2.1.0", "Non-blocking HTTP & WebSocket server toolkit for GVM.", "Garuda Team");
            publish_package("garuda_mnesia", "1.0.4", "Distributed in-memory & disk key-value database engine.", "Garuda Team");
            publish_package("garuda_crypto", "3.0.1", "End-to-end encryption toolkit (AES-GCM 256, X25519 DH).", "Garuda Security Core");
            publish_package("garuda_webrtc", "1.1.0", "Real-time P2P Signaling & Presence indicator module.", "Garuda Team");
            garuda::sys::JsonLogger::info("GPM Seeded initial Garuda standard packages.");
        }
    }

public:
    static GpmRegistry& get_instance() {
        static GpmRegistry instance;
        return instance;
    }

    std::pair<bool, std::string> publish_package(const std::string& name, const std::string& version, const std::string& description, const std::string& author) {
        if (name.empty() || version.empty()) {
            return {false, "Name or version is empty"};
        }
        init_db();
        if (!db_) {
            return {false, "Database connection failed"};
        }

        std::lock_guard<std::mutex> lock(mutex_);

        const char* sql_ins = "INSERT INTO packages (name, version, description, author) VALUES (?, ?, ?, ?);";
        sqlite3_stmt* stmt = nullptr;
        bool success = false;
        std::string err_reason = "";

        if (sqlite3_prepare_v2(db_, sql_ins, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, version.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 3, description.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 4, author.c_str(), -1, SQLITE_TRANSIENT);

            int res = sqlite3_step(stmt);
            if (res == SQLITE_DONE || res == SQLITE_OK) {
                success = true;
            } else {
                err_reason = std::string("Insert step failed: ") + sqlite3_errmsg(db_);
            }
            sqlite3_finalize(stmt);
        } else {
            err_reason = std::string("Insert prepare failed: ") + sqlite3_errmsg(db_);
        }

        if (!success) {
            const char* sql_upd = "UPDATE packages SET version = ?, description = ?, author = ? WHERE name = ?;";
            if (sqlite3_prepare_v2(db_, sql_upd, -1, &stmt, nullptr) == SQLITE_OK) {
                sqlite3_bind_text(stmt, 1, version.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 2, description.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 3, author.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 4, name.c_str(), -1, SQLITE_TRANSIENT);

                int res = sqlite3_step(stmt);
                if (res == SQLITE_DONE || res == SQLITE_OK) {
                    success = true;
                    err_reason = "";
                } else {
                    err_reason += std::string(" | Update step failed: ") + sqlite3_errmsg(db_);
                }
                sqlite3_finalize(stmt);
            } else {
                err_reason += std::string(" | Update prepare failed: ") + sqlite3_errmsg(db_);
            }
        }

        if (success) {
            garuda::sys::JsonLogger::info("Package dipublikasikan: " + name + " v" + version + " oleh " + author);
        } else {
            garuda::sys::JsonLogger::error("GPM Registry DB Error: " + err_reason);
        }
        return {success, err_reason};
    }

    std::string get_packages_json(const std::string& query = "") {
        init_db();
        if (!db_) return "[]";

        std::lock_guard<std::mutex> lock(mutex_);
        std::string sql;
        if (query.empty()) {
            sql = "SELECT id, name, version, description, author, downloads, strftime('%Y-%m-%d', created_at) FROM packages ORDER BY downloads DESC, id DESC;";
        } else {
            sql = "SELECT id, name, version, description, author, downloads, strftime('%Y-%m-%d', created_at) FROM packages "
                  "WHERE name LIKE ? OR description LIKE ? OR author LIKE ? ORDER BY downloads DESC;";
        }

        sqlite3_stmt* stmt = nullptr;
        std::ostringstream ss;
        ss << "[";
        bool first = true;

        if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            if (!query.empty()) {
                std::string pattern = "%" + query + "%";
                sqlite3_bind_text(stmt, 1, pattern.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 2, pattern.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 3, pattern.c_str(), -1, SQLITE_TRANSIENT);
            }

            while (sqlite3_step(stmt) == SQLITE_ROW) {
                if (!first) ss << ",";
                first = false;

                int id = sqlite3_column_int(stmt, 0);
                const unsigned char* name = sqlite3_column_text(stmt, 1);
                const unsigned char* version = sqlite3_column_text(stmt, 2);
                const unsigned char* desc = sqlite3_column_text(stmt, 3);
                const unsigned char* author = sqlite3_column_text(stmt, 4);
                int downloads = sqlite3_column_int(stmt, 5);
                const unsigned char* date = sqlite3_column_text(stmt, 6);

                std::string s_name = name ? reinterpret_cast<const char*>(name) : "";
                std::string s_version = version ? reinterpret_cast<const char*>(version) : "1.0.0";
                std::string s_desc = desc ? reinterpret_cast<const char*>(desc) : "";
                std::string s_author = author ? reinterpret_cast<const char*>(author) : "Anon";
                std::string s_date = date ? reinterpret_cast<const char*>(date) : "";

                // Escape string
                auto escape_json = [](const std::string& input) {
                    std::ostringstream out;
                    for (char c : input) {
                        if (c == '"') out << "\\\"";
                        else if (c == '\\') out << "\\\\";
                        else if (c == '\n') out << "\\n";
                        else out << c;
                    }
                    return out.str();
                };

                ss << "{\"id\":" << id << ","
                   << "\"name\":\"" << escape_json(s_name) << "\","
                   << "\"version\":\"" << escape_json(s_version) << "\","
                   << "\"description\":\"" << escape_json(s_desc) << "\","
                   << "\"author\":\"" << escape_json(s_author) << "\","
                   << "\"downloads\":" << downloads << ","
                   << "\"created_at\":\"" << escape_json(s_date) << "\"}";
            }
            sqlite3_finalize(stmt);
        }
        ss << "]";
        return ss.str();
    }
};

} // namespace garuda::gpm
