#pragma once
// Garuda Package Manager (GPM) Registry Engine (`gpm.hpp`)
// Mengelola pendaftaran, versi, pustaka standar, dan metadata package GPM via SQLite3.
#include "../sys/json_logger.hpp"

#include <sqlite3.h>
#include <cstdint>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace garuda::gpm {

struct PackageInfo {
    int id;
    std::string name;
    std::string version;
    std::string description;
    std::string author;
    std::string status;
    std::string created_at;
    std::string language;
    std::string repo_path;
};

class GpmRegistry {
private:
    sqlite3* db_ = nullptr;
    std::mutex mutex_;

    GpmRegistry() {
        init_db();
    }

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
                              "created_at DATETIME DEFAULT CURRENT_TIMESTAMP, "
                              "language TEXT DEFAULT 'cpp', "
                              "repo_path TEXT);";
            char* err_msg = nullptr;
            sqlite3_exec(db_, sql, nullptr, nullptr, &err_msg);
            if (err_msg) {
                sqlite3_free(err_msg);
            }

            // Migrations for existing databases
            const char* sql_alt1 = "ALTER TABLE packages ADD COLUMN language TEXT DEFAULT 'cpp';";
            sqlite3_exec(db_, sql_alt1, nullptr, nullptr, nullptr);
            const char* sql_alt2 = "ALTER TABLE packages ADD COLUMN repo_path TEXT;";
            sqlite3_exec(db_, sql_alt2, nullptr, nullptr, nullptr);

            // Pre-seed standard Garuda libraries
            seed_initial_packages();
        }
    }

    void seed_initial_packages() {
        publish_package("garuda_std", "1.0.0", "Pustaka standar fundamental: string, math, dan utilitas tipe data.", "Garuda Core Team", "source_only", "cpp", "include/garuda/beam/stdlib");
        publish_package("garuda_json", "1.2.0", "Parser & encoder JSON berkinerja tinggi untuk GVM Actor Model.", "Garuda Core Team", "source_only", "cpp", "include/garuda/beam/stdlib");
        publish_package("garuda_regex", "1.1.0", "Engine Regular Expression (Regex) berkinerja tinggi untuk GVM.", "Garuda Core Team", "coming_soon", "cpp", "");
        publish_package("garuda_datetime", "1.0.2", "Manipulasi tanggal, waktu, dan zona waktu ISO-8601.", "Garuda Core Team", "coming_soon", "cpp", "");
        publish_package("garuda_http", "2.1.0", "HTTP Client & Server ringan non-blocking untuk REST API.", "Garuda Core Team", "source_only", "cpp", "include/garuda/beam/stdlib");
        publish_package("garuda_ws", "1.0.5", "WebSocket Client & Server toolkit untuk aplikasi real-time.", "Garuda Core Team", "source_only", "cpp", "include/garuda/beam/stdlib");
        publish_package("garuda_network", "1.0.1", "TCP/UDP Socket, DNS Resolver, dan Ping network toolkit.", "Garuda Network Group", "source_only", "cpp", "include/garuda/beam/stdlib");
        publish_package("garuda_sqlite", "1.3.0", "Binding native SQLite untuk database relasional Jalur Langit.", "Garuda Storage Core", "source_only", "cpp", "include/garuda/beam/stdlib");
        publish_package("garuda_mnesia", "2.0.0", "Penyimpanan Key-Value & Mnesia terdistribusi bawaan GVM.", "Garuda Core Team", "source_only", "cpp", "include/garuda/beam/stdlib");
        publish_package("garuda_crypto", "3.0.1", "Algoritma kriptografi: SHA-256, AES-GCM 256, HMAC, X25519 DH.", "Garuda Security Core", "source_only", "cpp", "include/garuda/beam/stdlib");
        publish_package("garuda_jwt", "1.0.0", "Generator & validator JSON Web Token (JWT HS256) untuk SSO.", "Garuda Identity Team", "source_only", "cpp", "include/garuda/beam/stdlib");
        publish_package("garuda_logger", "1.1.0", "Structured JSON logging dengan output warna terminal & journalctl.", "Garuda Sys Group", "source_only", "cpp", "include/garuda/sys");
        publish_package("garuda_cli", "1.0.0", "Framework untuk membuat aplikasi Command Line Interface (CLI).", "Garuda Tooling Team", "source_only", "cpp", "src");
        publish_package("garuda_test", "1.0.0", "Framework Unit Test untuk menulis test case bahasa Garuda.", "Garuda Quality Team", "source_only", "cpp", "tests");

        // Seed 5 .grd pure Garuda packages
        publish_package("garuda_string_grd", "1.0.0", "Modul Manipulasi String Murni Bahasa Garuda (.grd).", "Garuda Core Team", "source_only_grd", "grd", "std/string.grd");
        publish_package("garuda_result_grd", "1.0.0", "Modul Result & Option Monad Murni Bahasa Garuda (.grd).", "Garuda Core Team", "source_only_grd", "grd", "std/result.grd");
        publish_package("garuda_list_grd", "1.0.0", "Modul Pemrosesan List Murni Bahasa Garuda (.grd).", "Garuda Core Team", "source_only_grd", "grd", "std/list.grd");
        publish_package("garuda_map_grd", "1.0.0", "Modul Pemrosesan Map/Kamus Murni Bahasa Garuda (.grd).", "Garuda Core Team", "source_only_grd", "grd", "std/map.grd");
        publish_package("garuda_json_grd", "1.0.0", "Modul Serializer JSON Murni Bahasa Garuda (.grd).", "Garuda Core Team", "source_only_grd", "grd", "std/json.grd");

        garuda::sys::JsonLogger::info("GPM Seeded 19 Garuda packages (14 C++ & 5 .grd).");
    }

public:
    static GpmRegistry& get_instance() {
        static GpmRegistry instance;
        return instance;
    }

    std::pair<bool, std::string> publish_package(const std::string& name, const std::string& version, const std::string& description, const std::string& author, const std::string& status = "source_only", const std::string& language = "cpp", const std::string& repo_path = "") {
        if (name.empty() || version.empty()) {
            return {false, "Name or version is empty"};
        }
        init_db();
        if (!db_) {
            return {false, "Database connection failed"};
        }

        std::lock_guard<std::mutex> lock(mutex_);

        const char* sql_ins = "INSERT INTO packages (name, version, description, author, status, language, repo_path) VALUES (?, ?, ?, ?, ?, ?, ?);";
        sqlite3_stmt* stmt = nullptr;
        bool success = false;
        std::string err_reason = "";

        if (sqlite3_prepare_v2(db_, sql_ins, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, version.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 3, description.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 4, author.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 5, status.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 6, language.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 7, repo_path.c_str(), -1, SQLITE_TRANSIENT);

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
            const char* sql_upd = "UPDATE packages SET version = ?, description = ?, author = ?, status = ?, language = ?, repo_path = ? WHERE name = ?;";
            if (sqlite3_prepare_v2(db_, sql_upd, -1, &stmt, nullptr) == SQLITE_OK) {
                sqlite3_bind_text(stmt, 1, version.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 2, description.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 3, author.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 4, status.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 5, language.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 6, repo_path.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 7, name.c_str(), -1, SQLITE_TRANSIENT);

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
            garuda::sys::JsonLogger::info("Package dipublikasikan: " + name + " v" + version + " (" + language + ") oleh " + author);
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
            sql = "SELECT id, name, version, description, author, status, strftime('%Y-%m-%d', created_at), language, repo_path FROM packages ORDER BY id ASC;";
        } else {
            sql = "SELECT id, name, version, description, author, status, strftime('%Y-%m-%d', created_at), language, repo_path FROM packages "
                  "WHERE name LIKE ? OR description LIKE ? OR author LIKE ? ORDER BY id ASC;";
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
                const unsigned char* status = sqlite3_column_text(stmt, 5);
                const unsigned char* date = sqlite3_column_text(stmt, 6);
                const unsigned char* lang = sqlite3_column_text(stmt, 7);
                const unsigned char* rpath = sqlite3_column_text(stmt, 8);

                std::string s_name = name ? reinterpret_cast<const char*>(name) : "";
                std::string s_version = version ? reinterpret_cast<const char*>(version) : "1.0.0";
                std::string s_desc = desc ? reinterpret_cast<const char*>(desc) : "";
                std::string s_author = author ? reinterpret_cast<const char*>(author) : "Anon";
                std::string s_status = status ? reinterpret_cast<const char*>(status) : "coming_soon";
                std::string s_date = date ? reinterpret_cast<const char*>(date) : "";
                std::string s_lang = lang ? reinterpret_cast<const char*>(lang) : "cpp";
                std::string s_rpath = rpath ? reinterpret_cast<const char*>(rpath) : "";

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
                   << "\"status\":\"" << escape_json(s_status) << "\","
                   << "\"created_at\":\"" << escape_json(s_date) << "\","
                   << "\"language\":\"" << escape_json(s_lang) << "\","
                   << "\"repo_path\":\"" << escape_json(s_rpath) << "\"}";
            }
            sqlite3_finalize(stmt);
        }
        ss << "]";
        return ss.str();
    }
};

} // namespace garuda::gpm
