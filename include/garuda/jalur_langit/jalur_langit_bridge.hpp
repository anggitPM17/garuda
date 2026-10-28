#pragma once

#include "jalur_langit_ultimate.hpp"
#include "connection_pool.hpp"
#include "auth_manager.hpp"
#include "schema_migrator.hpp"
#include "json_serializer.hpp"
#include "../nusantara/nusantara_broker.hpp"

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <chrono>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <ctime>
#include <iomanip>

namespace garuda::jalur_langit {

class JalurLangitBridge {
public:
    struct Stats {
        std::size_t commands_received = 0;
        std::size_t commands_rejected = 0;
        std::size_t queries_executed = 0;
        std::size_t rows_returned = 0;
    };

private:
    std::unique_ptr<JalurLangitUltimate> db_;
    std::unique_ptr<ConnectionPool> pool_;
    std::unique_ptr<AuthManager> auth_;
    std::unique_ptr<SchemaMigrator> migrator_;
    std::string auth_token_;
    mutable std::mutex mutex_;
    bool running_ = false;

    // Rate limiter state
    struct RateLimiter {
        std::chrono::steady_clock::time_point window_start;
        std::size_t count = 0;
    };
    std::unordered_map<std::string, RateLimiter> rate_limiters_;
    std::mutex rate_mutex_;

    Stats stats_;
    mutable std::mutex stats_mutex_;

public:
    JalurLangitBridge(const std::string& data_dir = "./jalur_langit_data") {
        db_ = std::make_unique<JalurLangitUltimate>(data_dir);
        pool_ = std::make_unique<ConnectionPool>(5, data_dir);
        auth_ = std::make_unique<AuthManager>();
        migrator_ = std::make_unique<SchemaMigrator>();

        // Default admin grant
        auth_->grant("AUTOPILOT_AI", "schema", Permission::ADMIN);
        auth_->grant("AUTOPILOT_AI", "*", Permission::ADMIN);
    }

    ~JalurLangitBridge() {
        stop();
    }

    void set_auth_token(const std::string& token) {
        std::lock_guard<std::mutex> lock(mutex_);
        auth_token_ = token;
    }

    void start() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (running_) return;
        running_ = true;

        nusantara::NusantaraBroker::get_instance().register_entity(
            nusantara::EntityTarget::JALUR_LANGIT,
            [this](const nusantara::NusantaraMessage& msg) {
                this->handle_message(msg);
            }
        );
        std::cout << "[JALUR LANGIT BRIDGE] Terhubung ke NusantaraBroker (EntityTarget::JALUR_LANGIT).\n";
    }

    void stop() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!running_) return;
        running_ = false;
        std::cout << "[JALUR LANGIT BRIDGE] Dihentikan.\n";
    }

    Stats get_stats() const {
        std::lock_guard<std::mutex> lock(stats_mutex_);
        return stats_;
    }

private:
    void increment_received() {
        std::lock_guard<std::mutex> lock(stats_mutex_);
        stats_.commands_received++;
    }

    void increment_rejected() {
        std::lock_guard<std::mutex> lock(stats_mutex_);
        stats_.commands_rejected++;
    }

    void increment_queries(std::size_t rows = 0) {
        std::lock_guard<std::mutex> lock(stats_mutex_);
        stats_.queries_executed++;
        stats_.rows_returned += rows;
    }

    void audit_log(const std::string& sender, const std::string& action,
                   const std::string& target, bool success, const std::string& detail) {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
#if defined(_MSC_VER)
        struct tm buf;
        localtime_s(&buf, &in_time_t);
        ss << std::put_time(&buf, "%Y-%m-%d %H:%M:%S");
#else
        ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
#endif

        std::cout << "[AUDIT] " << ss.str()
                  << " | " << sender
                  << " | " << action
                  << " | " << target
                  << " | " << (success ? "SUCCESS" : "FAIL")
                  << " | " << detail << "\n";
    }

    bool validate_auth(const nusantara::NusantaraMessage& msg) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (auth_token_.empty()) {
            return true; // Jika token belum diset, ijinkan mode terbuka
        }
        auto it = msg.payload.find("auth_token");
        if (it == msg.payload.end() || it->second != auth_token_) {
            return false;
        }
        return true;
    }

    bool check_rate_limit(const std::string& sender) {
        std::lock_guard<std::mutex> lock(rate_mutex_);
        auto now = std::chrono::steady_clock::now();
        auto& rl = rate_limiters_[sender];

        if (rl.count == 0 || (now - rl.window_start >= std::chrono::seconds(1))) {
            rl.window_start = now;
            rl.count = 1;
            return true;
        }

        rl.count++;
        if (rl.count > 100) {
            return false;
        }
        return true;
    }

    bool validate_identifier(const std::string& ident) {
        if (ident.empty() || ident.size() > 64) return false;
        if (!std::isalpha(static_cast<unsigned char>(ident[0])) && ident[0] != '_') return false;
        for (char c : ident) {
            if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_') return false;
        }
        return true;
    }

    bool validate_query(const std::string& query) {
        if (query.empty() || query.size() > 4096) return false;

        // Cek Karakter terlarang / semicolon untuk mencegat multi-query SQL Injection
        if (query.find(';') != std::string::npos) return false;

        std::string upper_q = query;
        std::transform(upper_q.begin(), upper_q.end(), upper_q.begin(),
                       [](unsigned char c) { return static_cast<char>(std::toupper(c)); });

        // Trim leading whitespace
        std::size_t start = upper_q.find_first_not_of(" \t\n\r");
        if (start == std::string::npos) return false;
        std::string trimmed = upper_q.substr(start);

        // Hanya ijinkan SELECT atau INSERT
        if (trimmed.rfind("SELECT", 0) != 0 && trimmed.rfind("INSERT", 0) != 0) {
            return false;
        }

        // Cek kata kunci berbahaya
        std::vector<std::string> forbidden = {"DROP", "DELETE", "TRUNCATE", "ALTER", "UPDATE"};
        for (const auto& word : forbidden) {
            if (upper_q.find(word) != std::string::npos) {
                return false;
            }
        }
        return true;
    }

    bool validate_payload_size(const nusantara::NusantaraMessage& msg) {
        std::size_t total_size = msg.message_id.size();
        for (const auto& [k, v] : msg.payload) {
            total_size += k.size() + v.size();
        }
        return total_size <= 1048576; // 1 MB
    }

    void send_response(nusantara::EntityTarget receiver, int status_code, bool success,
                       const std::string& message, const QueryResult* result = nullptr) {
        nusantara::NusantaraMessage resp;
        resp.message_id = "resp_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
        resp.sender = nusantara::EntityTarget::JALUR_LANGIT;
        resp.receiver = receiver;
        resp.action = nusantara::NusantaraAction::BROADCAST_MSG;

        resp.payload["status"] = std::to_string(status_code);
        resp.payload["success"] = success ? "true" : "false";
        resp.payload["message"] = message;

        if (result) {
            resp.payload["data"] = JsonSerializer::serialize_result(*result);
        }

        nusantara::NusantaraBroker::get_instance().send_message(resp);
    }

    void handle_message(const nusantara::NusantaraMessage& msg) {
        increment_received();
        std::string sender_str = nusantara::to_string(msg.sender);
        std::string action_str = nusantara::to_string(msg.action);

        try {
            // Layer 1: Payload Size Check
            if (!validate_payload_size(msg)) {
                increment_rejected();
                audit_log(sender_str, action_str, "PAYLOAD", false, "Payload size exceeds 1 MB (413)");
                send_response(msg.sender, 413, false, "Payload size exceeds 1 MB");
                return;
            }

            // Layer 2: Auth Check
            if (!validate_auth(msg)) {
                increment_rejected();
                audit_log(sender_str, action_str, "AUTH", false, "Authentication failed (401)");
                send_response(msg.sender, 401, false, "Unauthorized: Invalid or missing auth token");
                return;
            }

            // Layer 3: Rate Limiting Check
            if (!check_rate_limit(sender_str)) {
                increment_rejected();
                audit_log(sender_str, action_str, "RATE_LIMIT", false, "Rate limit exceeded (429)");
                send_response(msg.sender, 429, false, "Too Many Requests: Rate limit exceeded");
                return;
            }

            // Layer 4: Whitelist Action Check
            if (msg.action == nusantara::NusantaraAction::JALUR_LANGIT_CREATE_TABLE) {
                handle_create_table(msg);
            } else if (msg.action == nusantara::NusantaraAction::JALUR_LANGIT_INSERT) {
                handle_insert(msg);
            } else if (msg.action == nusantara::NusantaraAction::JALUR_LANGIT_QUERY) {
                handle_query(msg);
            } else if (msg.action == nusantara::NusantaraAction::JALUR_LANGIT_UPDATE_SCHEMA) {
                handle_update_schema(msg);
            } else {
                increment_rejected();
                audit_log(sender_str, action_str, "WHITELIST", false, "Action not allowed (403)");
                send_response(msg.sender, 403, false, "Forbidden: Action not allowed for Jalur Langit");
            }
        } catch (const std::exception& e) {
            increment_rejected();
            audit_log(sender_str, action_str, "BRIDGE", false, std::string("Internal Error: ") + e.what());
            send_response(msg.sender, 500, false, std::string("Internal Server Error: ") + e.what());
        }
    }

    void handle_create_table(const nusantara::NusantaraMessage& msg) {
        std::string sender_str = nusantara::to_string(msg.sender);
        auto t_it = msg.payload.find("table_name");
        if (t_it == msg.payload.end() || !validate_identifier(t_it->second)) {
            increment_rejected();
            audit_log(sender_str, "CREATE_TABLE", t_it != msg.payload.end() ? t_it->second : "EMPTY", false, "Invalid table name (400)");
            send_response(msg.sender, 400, false, "Invalid or missing table_name");
            return;
        }

        std::string table_name = t_it->second;

        // Parse columns e.g. "id:INT:pk,nama:STRING,umur:INT"
        auto c_it = msg.payload.find("columns");
        std::vector<ColumnDef> columns;
        if (c_it != msg.payload.end()) {
            std::stringstream ss(c_it->second);
            std::string item;
            while (std::getline(ss, item, ',')) {
                if (item.empty()) continue;
                std::stringstream css(item);
                std::string col_name, col_type, is_pk_str;
                std::getline(css, col_name, ':');
                std::getline(css, col_type, ':');
                std::getline(css, is_pk_str, ':');

                if (validate_identifier(col_name)) {
                    bool is_pk = (is_pk_str == "pk" || is_pk_str == "true");
                    columns.push_back(ColumnDef{col_name, col_type.empty() ? "STRING" : col_type, is_pk});
                }
            }
        }

        if (columns.empty()) {
            columns = {{"id", "INT", true}, {"data", "STRING", false}};
        }

        db_->create_table(table_name, columns);
        increment_queries();
        audit_log(sender_str, "CREATE_TABLE", table_name, true, "Table created successfully");
        send_response(msg.sender, 200, true, "Table '" + table_name + "' created successfully");
    }

    void handle_insert(const nusantara::NusantaraMessage& msg) {
        std::string sender_str = nusantara::to_string(msg.sender);
        auto t_it = msg.payload.find("table_name");
        if (t_it == msg.payload.end() || !validate_identifier(t_it->second)) {
            increment_rejected();
            audit_log(sender_str, "INSERT", t_it != msg.payload.end() ? t_it->second : "EMPTY", false, "Invalid table_name (400)");
            send_response(msg.sender, 400, false, "Invalid or missing table_name");
            return;
        }

        std::string table_name = t_it->second;
        auto q_it = msg.payload.find("query");
        std::string query;
        if (q_it != msg.payload.end()) {
            query = q_it->second;
        } else {
            query = "INSERT INTO " + table_name + " VALUES (...)";
        }

        auto res = db_->execute(query);
        if (res.success) {
            increment_queries(res.affected_rows);
            audit_log(sender_str, "INSERT", table_name, true, "Insert executed successfully");
            send_response(msg.sender, 200, true, "Insert executed successfully", &res);
        } else {
            increment_rejected();
            audit_log(sender_str, "INSERT", table_name, false, res.error_message);
            send_response(msg.sender, 400, false, res.error_message);
        }
    }

    void handle_query(const nusantara::NusantaraMessage& msg) {
        std::string sender_str = nusantara::to_string(msg.sender);
        auto q_it = msg.payload.find("query");
        if (q_it == msg.payload.end() || !validate_query(q_it->second)) {
            increment_rejected();
            audit_log(sender_str, "QUERY", "SQL", false, "Invalid or disallowed query (400)");
            send_response(msg.sender, 400, false, "Invalid or disallowed query string");
            return;
        }

        std::string query = q_it->second;
        auto res = db_->execute(query);

        // Resource limit: max 1000 rows
        if (res.rows.size() > 1000) {
            res.rows.resize(1000);
        }

        if (res.success) {
            increment_queries(res.rows.size());
            audit_log(sender_str, "QUERY", query, true, "Query executed successfully");
            send_response(msg.sender, 200, true, "Query executed successfully", &res);
        } else {
            increment_rejected();
            audit_log(sender_str, "QUERY", query, false, res.error_message);
            send_response(msg.sender, 400, false, res.error_message);
        }
    }

    void handle_update_schema(const nusantara::NusantaraMessage& msg) {
        std::string sender_str = nusantara::to_string(msg.sender);

        // Zero-Trust Auth Admin Check
        if (!auth_->check_permission(sender_str, "schema", Permission::ADMIN) &&
            !auth_->check_permission(sender_str, "*", Permission::ADMIN)) {
            increment_rejected();
            audit_log(sender_str, "UPDATE_SCHEMA", "SCHEMA", false, "Admin permission required (403)");
            send_response(msg.sender, 403, false, "Forbidden: Admin permission required for schema updates");
            return;
        }

        auto v_it = msg.payload.find("version");
        auto d_it = msg.payload.find("description");
        int version = (v_it != msg.payload.end()) ? std::stoi(v_it->second) : 1;
        std::string desc = (d_it != msg.payload.end()) ? d_it->second : "Schema update";

        migrator_->add_migration(version, desc, [this, msg](JalurLangitUltimate& db) {
            auto t_it = msg.payload.find("table_name");
            if (t_it != msg.payload.end() && validate_identifier(t_it->second)) {
                db.create_table(t_it->second, {{"id", "INT", true}, {"version", "INT", false}});
            }
        });

        migrator_->run_migrations(*db_);
        increment_queries();
        audit_log(sender_str, "UPDATE_SCHEMA", "v" + std::to_string(version), true, "Schema migration executed");
        send_response(msg.sender, 200, true, "Schema updated to version " + std::to_string(migrator_->current_version()));
    }
};

} // namespace garuda::jalur_langit
