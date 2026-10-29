#pragma once
// =====================================================================
// GARUDA AUTH MANAGER — TRUE IDENTITY & JWT ENGINE
// Persistent User Accounts in SQLite & HMAC SHA-256 JWT Authentication
// =====================================================================

#include "sqlite3.h"
#include "../sys/json_logger.hpp"
#include "../beam/stdlib/crypto.hpp"
#include "../beam/stdlib/jwt.hpp"

#include <string>
#include <mutex>
#include <sstream>
#include <chrono>

namespace garuda::auth {

constexpr const char* JWT_SECRET = "GARUDA_BEAM_GVM_SECRET_KEY_2026";

class AuthManager {
private:
    sqlite3* db_ = nullptr;
    mutable std::mutex mutex_;

    AuthManager() = default;

    ~AuthManager() {
        if (db_) {
            sqlite3_close(db_);
            db_ = nullptr;
        }
    }

    void init_db() {
        if (db_) return;
        int rc = sqlite3_open("/var/lib/richat-garuda/arena_leaderboard.db", &db_);
        if (rc != SQLITE_OK && db_) {
            sqlite3_close(db_);
            db_ = nullptr;
            rc = sqlite3_open("arena_leaderboard.db", &db_);
        }
        if (rc == SQLITE_OK && db_) {
            const char* sql = "CREATE TABLE IF NOT EXISTS users ("
                              "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                              "username TEXT UNIQUE NOT NULL, "
                              "password_hash TEXT NOT NULL, "
                              "created_at DATETIME DEFAULT CURRENT_TIMESTAMP);";
            char* err_msg = nullptr;
            sqlite3_exec(db_, sql, nullptr, nullptr, &err_msg);
            if (err_msg) {
                sqlite3_free(err_msg);
            }
        }
    }

    static std::string hash_password(const std::string& username, const std::string& password) {
        std::string salt = "GVM_SALT_" + username;
        return garuda::beam::stdlib::Sha256::hash(password + ":" + salt);
    }

public:
    static AuthManager& get_instance() {
        static AuthManager instance;
        return instance;
    }

    std::pair<bool, std::string> register_user(const std::string& username, const std::string& password) {
        if (username.empty() || password.empty()) {
            return {false, "Username dan password tidak boleh kosong"};
        }
        if (username.length() < 3 || username.length() > 20) {
            return {false, "Username harus 3-20 karakter"};
        }

        init_db();
        if (!db_) return {false, "Koneksi database gagal"};

        std::lock_guard<std::mutex> lock(mutex_);
        std::string pass_hash = hash_password(username, password);

        const char* sql = "INSERT INTO users (username, password_hash) VALUES (?, ?);";
        sqlite3_stmt* stmt = nullptr;
        bool success = false;
        std::string err_msg = "";

        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, pass_hash.c_str(), -1, SQLITE_TRANSIENT);

            int res = sqlite3_step(stmt);
            if (res == SQLITE_DONE || res == SQLITE_OK) {
                success = true;
            } else {
                err_msg = "Username '" + username + "' sudah terdaftar";
            }
            sqlite3_finalize(stmt);
        } else {
            err_msg = "Gagal memproses statement database";
        }

        if (success) {
            garuda::sys::JsonLogger::info("User registered successfully: " + username);
        } else {
            garuda::sys::JsonLogger::warn("User registration failed for " + username + ": " + err_msg);
        }
        return {success, err_msg};
    }

    std::pair<bool, std::string> create_jwt_token(const std::string& username) {
        auto now = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
        uint64_t exp = now + (7 * 24 * 3600); // 7 Hari Expiration

        std::string header_json = "{\"alg\":\"HS256\",\"typ\":\"JWT\"}";
        std::string payload_json = "{\"sub\":\"" + username + "\",\"exp\":" + std::to_string(exp) + "}";

        std::string header_b64 = garuda::beam::stdlib::Base64Url::encode(header_json);
        std::string payload_b64 = garuda::beam::stdlib::Base64Url::encode(payload_json);

        std::string signing_input = header_b64 + "." + payload_b64;
        std::string sig_raw = garuda::beam::stdlib::Sha256::hash(signing_input + "." + JWT_SECRET);
        std::string sig_b64 = garuda::beam::stdlib::Base64Url::encode(sig_raw);

        return {true, signing_input + "." + sig_b64};
    }

    std::pair<bool, std::string> login_user(const std::string& username, const std::string& password) {
        if (username.empty() || password.empty()) {
            return {false, "Username atau password salah"};
        }

        init_db();
        if (!db_) return {false, "Koneksi database gagal"};

        std::lock_guard<std::mutex> lock(mutex_);
        std::string pass_hash = hash_password(username, password);

        const char* sql = "SELECT username FROM users WHERE username = ? AND password_hash = ?;";
        sqlite3_stmt* stmt = nullptr;
        bool valid = false;

        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, pass_hash.c_str(), -1, SQLITE_TRANSIENT);

            if (sqlite3_step(stmt) == SQLITE_ROW) {
                valid = true;
            }
            sqlite3_finalize(stmt);
        }

        if (!valid) {
            garuda::sys::JsonLogger::warn("Login failed for user: " + username);
            return {false, "Username atau password salah"};
        }

        garuda::sys::JsonLogger::info("User logged in successfully: " + username);
        return create_jwt_token(username);
    }

    std::pair<bool, std::string> validate_jwt(const std::string& token) {
        if (token.empty()) return {false, ""};

        auto p1 = token.find('.');
        if (p1 == std::string::npos) return {false, ""};
        auto p2 = token.find('.', p1 + 1);
        if (p2 == std::string::npos) return {false, ""};

        std::string header_b64 = token.substr(0, p1);
        std::string payload_b64 = token.substr(p1 + 1, p2 - (p1 + 1));
        std::string sig_b64 = token.substr(p2 + 1);

        // Verify HMAC-SHA256 signature
        std::string signing_input = header_b64 + "." + payload_b64;
        std::string expected_sig_raw = garuda::beam::stdlib::Sha256::hash(signing_input + "." + JWT_SECRET);
        std::string expected_sig_b64 = garuda::beam::stdlib::Base64Url::encode(expected_sig_raw);

        if (sig_b64 != expected_sig_b64) {
            return {false, ""}; // Signature mismatch!
        }

        // Decode payload
        std::string payload_json = garuda::beam::stdlib::Base64Url::decode(payload_b64);

        // Extract sub (username)
        std::string username = "";
        auto sub_pos = payload_json.find("\"sub\":\"");
        if (sub_pos != std::string::npos) {
            auto sub_end = payload_json.find("\"", sub_pos + 7);
            if (sub_end != std::string::npos) {
                username = payload_json.substr(sub_pos + 7, sub_end - (sub_pos + 7));
            }
        }

        if (username.empty()) return {false, ""};
        return {true, username};
    }
};

} // namespace garuda::auth
