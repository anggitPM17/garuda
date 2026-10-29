#pragma once
// =====================================================================
// GARUDA ARENA GAME ENGINE (ACTOR MODEL)
// Real-Time Multiplayer Actor Game Engine running on GVM / Garuda BEAM
// Persistent Leaderboard via SQLite (Jalur Langit Storage)
// =====================================================================

#include "sqlite3.h"
#include "../sys/json_logger.hpp"
#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <chrono>
#include <cmath>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <random>
#include <functional>

namespace garuda::game {

// Consts for Game Engine
constexpr float ARENA_WIDTH = 1000.0f;
constexpr float ARENA_HEIGHT = 1000.0f;
constexpr float PLAYER_RADIUS = 16.0f;
constexpr float BULLET_RADIUS = 5.0f;
constexpr float BULLET_SPEED = 12.0f;
constexpr int BULLET_MAX_TTL = 75; // ~2.5 detik pada 30 FPS
constexpr int BULLET_DAMAGE = 20;

struct PlayerState {
    std::string id;
    std::string name;
    float x = 500.0f;
    float y = 500.0f;
    float angle = 0.0f; // Radians
    int hp = 100;
    int score = 0;
    std::string color = "#ef4444";
    std::chrono::steady_clock::time_point last_active = std::chrono::steady_clock::now();
};

struct BulletState {
    std::string id;
    std::string owner_id;
    float x = 0.0f;
    float y = 0.0f;
    float vx = 0.0f;
    float vy = 0.0f;
    int ttl = BULLET_MAX_TTL;
};

class ArenaEngine {
private:
    std::unordered_map<std::string, PlayerState> players_;
    std::vector<BulletState> bullets_;
    mutable std::mutex mutex_;
    uint64_t bullet_counter_ = 0;
    sqlite3* db_ = nullptr;

    std::vector<std::string> player_colors_ = {
        "#ef4444", "#3b82f6", "#10b981", "#f59e0b",
        "#8b5cf6", "#ec4899", "#14b8a6", "#f97316"
    };

    std::vector<std::function<void(const std::string&)>> broadcast_callbacks_;

    ArenaEngine() = default;

    ~ArenaEngine() {
        if (db_) {
            sqlite3_close(db_);
            db_ = nullptr;
        }
    }

    void init_db() {
        if (db_) return;
        int rc = sqlite3_open("/var/lib/richat-garuda/arena_leaderboard.db", &db_);
        if (rc != SQLITE_OK) {
            rc = sqlite3_open("arena_leaderboard.db", &db_);
        }
        if (rc == SQLITE_OK && db_) {
            const char* sql = "CREATE TABLE IF NOT EXISTS leaderboard ("
                              "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                              "name TEXT NOT NULL, "
                              "score INTEGER NOT NULL, "
                              "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP);";
            char* err_msg = nullptr;
            sqlite3_exec(db_, sql, nullptr, nullptr, &err_msg);
            if (err_msg) {
                sqlite3_free(err_msg);
            }
        }
    }

    std::string pick_color(const std::string& id) {
        std::size_t hash = std::hash<std::string>{}(id);
        return player_colors_[hash % player_colors_.size()];
    }

public:
    static ArenaEngine& get_instance() {
        static ArenaEngine instance;
        return instance;
    }

    // High Score Persistence
    void save_high_score(const std::string& name, int score) {
        if (score <= 0) return;
        init_db();
        if (!db_) return;

        const char* sql = "INSERT INTO leaderboard (name, score) VALUES (?, ?);";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(stmt, 2, score);
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
            garuda::sys::JsonLogger::info("High Score Disimpan ke DB: " + name + " -> " + std::to_string(score));
        }
    }

    std::string get_top_leaderboard_json() {
        init_db();
        if (!db_) return "[]";

        std::lock_guard<std::mutex> lock(mutex_);
        const char* sql = "SELECT name, score, strftime('%Y-%m-%d %H:%M', timestamp) FROM leaderboard ORDER BY score DESC LIMIT 10;";
        sqlite3_stmt* stmt = nullptr;

        std::ostringstream ss;
        ss << "[";
        bool first = true;

        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                if (!first) ss << ",";
                first = false;

                const unsigned char* name = sqlite3_column_text(stmt, 0);
                int score = sqlite3_column_int(stmt, 1);
                const unsigned char* ts = sqlite3_column_text(stmt, 2);

                std::string s_name = name ? reinterpret_cast<const char*>(name) : "Anon";
                std::string s_ts = ts ? reinterpret_cast<const char*>(ts) : "";

                ss << "{\"name\":\"" << s_name << "\","
                   << "\"score\":" << score << ","
                   << "\"date\":\"" << s_ts << "\"}";
            }
            sqlite3_finalize(stmt);
        }
        ss << "]";
        return ss.str();
    }

    // Callback registrasi untuk broadcast state via WS
    void register_broadcast_callback(std::function<void(const std::string&)> cb) {
        std::lock_guard<std::mutex> lock(mutex_);
        broadcast_callbacks_.push_back(cb);
    }

    void handle_join(const std::string& player_id, const std::string& name = "", const std::string& color = "") {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = players_.find(player_id);
        if (it == players_.end()) {
            PlayerState p;
            p.id = player_id;
            p.name = name.empty() ? ("Pemain_" + player_id.substr(0, 4)) : name;

            // Random initial spawn point in center safe-zone
            std::mt19937 rng(static_cast<unsigned int>(std::chrono::steady_clock::now().time_since_epoch().count()));
            std::uniform_real_distribution<float> dist_x(100.0f, ARENA_WIDTH - 100.0f);
            std::uniform_real_distribution<float> dist_y(100.0f, ARENA_HEIGHT - 100.0f);

            p.x = dist_x(rng);
            p.y = dist_y(rng);
            p.angle = 0.0f;
            p.hp = 100;
            p.score = 0;
            p.color = color.empty() ? pick_color(player_id) : color;
            p.last_active = std::chrono::steady_clock::now();

            players_[player_id] = p;
            garuda::sys::JsonLogger::info("Pemain bergabung: " + p.name + " (" + player_id + "), Warna: " + p.color);
        } else {
            it->second.last_active = std::chrono::steady_clock::now();
            if (!name.empty()) it->second.name = name;
            if (!color.empty()) it->second.color = color;
        }
    }

    void process_move(const std::string& player_id, float dx, float dy, float angle = 0.0f) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = players_.find(player_id);
        if (it == players_.end()) {
            // Auto join if not found
            PlayerState p;
            p.id = player_id;
            p.name = "Pemain_" + player_id.substr(0, 4);
            p.x = 500.0f;
            p.y = 500.0f;
            p.color = pick_color(player_id);
            players_[player_id] = p;
            it = players_.find(player_id);
        }

        auto& p = it->second;
        p.last_active = std::chrono::steady_clock::now();

        // Speed multiplier
        float speed = 6.0f;
        p.x += dx * speed;
        p.y += dy * speed;
        p.angle = angle;

        // Clamp to map boundaries
        p.x = std::clamp(p.x, PLAYER_RADIUS, ARENA_WIDTH - PLAYER_RADIUS);
        p.y = std::clamp(p.y, PLAYER_RADIUS, ARENA_HEIGHT - PLAYER_RADIUS);
    }

    void process_shoot(const std::string& player_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = players_.find(player_id);
        if (it == players_.end()) return;

        auto& p = it->second;
        p.last_active = std::chrono::steady_clock::now();

        // Spawn bullet moving in direction of angle
        BulletState b;
        b.id = "b_" + std::to_string(++bullet_counter_);
        b.owner_id = player_id;

        // Spawn slightly in front of ship
        b.x = p.x + std::cos(p.angle) * (PLAYER_RADIUS + 4.0f);
        b.y = p.y + std::sin(p.angle) * (PLAYER_RADIUS + 4.0f);

        b.vx = std::cos(p.angle) * BULLET_SPEED;
        b.vy = std::sin(p.angle) * BULLET_SPEED;
        b.ttl = BULLET_MAX_TTL;

        bullets_.push_back(b);
    }

    void tick() {
        std::lock_guard<std::mutex> lock(mutex_);

        // 1. Update Bullets position & TTL
        for (auto& b : bullets_) {
            b.x += b.vx;
            b.y += b.vy;
            b.ttl--;
        }

        // 2. Remove expired or out-of-bounds bullets
        bullets_.erase(
            std::remove_if(bullets_.begin(), bullets_.end(), [](const BulletState& b) {
                return b.ttl <= 0 || b.x < 0 || b.x > ARENA_WIDTH || b.y < 0 || b.y > ARENA_HEIGHT;
            }),
            bullets_.end()
        );

        // 3. Collision Detection: Bullets vs Players
        for (auto b_it = bullets_.begin(); b_it != bullets_.end(); ) {
            bool bullet_destroyed = false;

            for (auto& [p_id, player] : players_) {
                if (player.hp <= 0) continue;
                if (b_it->owner_id == p_id) continue; // No self-harm

                float dx = b_it->x - player.x;
                float dy = b_it->y - player.y;
                float dist = std::sqrt(dx * dx + dy * dy);

                if (dist < (PLAYER_RADIUS + BULLET_RADIUS)) {
                    // Hit!
                    player.hp -= BULLET_DAMAGE;
                    bullet_destroyed = true;

                    // If player killed
                    if (player.hp <= 0) {
                        player.hp = 0;

                        // Save high score of killed player before reset
                        if (player.score > 0) {
                            save_high_score(player.name, player.score);
                        }

                        // Reward shooter
                        auto shooter_it = players_.find(b_it->owner_id);
                        if (shooter_it != players_.end()) {
                            shooter_it->second.score += 100;
                        }

                        // Respawn player
                        player.hp = 100;
                        player.score = 0;
                        std::mt19937 rng(static_cast<unsigned int>(std::chrono::steady_clock::now().time_since_epoch().count()));
                        std::uniform_real_distribution<float> dist_x(100.0f, ARENA_WIDTH - 100.0f);
                        std::uniform_real_distribution<float> dist_y(100.0f, ARENA_HEIGHT - 100.0f);
                        player.x = dist_x(rng);
                        player.y = dist_y(rng);
                    }
                    break;
                }
            }

            if (bullet_destroyed) {
                b_it = bullets_.erase(b_it);
            } else {
                ++b_it;
            }
        }

        // 4. Inactive player cleanup (timeout after 30 sec)
        auto now = std::chrono::steady_clock::now();
        for (auto it = players_.begin(); it != players_.end(); ) {
            auto sec = std::chrono::duration_cast<std::chrono::seconds>(now - it->second.last_active).count();
            if (sec > 30) {
                if (it->second.score > 0) {
                    save_high_score(it->second.name, it->second.score);
                }
                garuda::sys::JsonLogger::info("Inactive player removed: " + it->first);
                it = players_.erase(it);
            } else {
                ++it;
            }
        }
    }

    std::string get_state_json() const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::ostringstream ss;
        ss << "{\"type\":\"arena_state\",\"players\":[";

        bool first_p = true;
        for (const auto& [id, p] : players_) {
            if (!first_p) ss << ",";
            first_p = false;
            ss << "{\"id\":\"" << p.id << "\","
               << "\"name\":\"" << p.name << "\","
               << "\"x\":" << std::round(p.x * 10.0f) / 10.0f << ","
               << "\"y\":" << std::round(p.y * 10.0f) / 10.0f << ","
               << "\"angle\":" << std::round(p.angle * 100.0f) / 100.0f << ","
               << "\"hp\":" << p.hp << ","
               << "\"score\":" << p.score << ","
               << "\"color\":\"" << p.color << "\"}";
        }

        ss << "],\"bullets\":[";
        bool first_b = true;
        for (const auto& b : bullets_) {
            if (!first_b) ss << ",";
            first_b = false;
            ss << "{\"id\":\"" << b.id << "\","
               << "\"x\":" << std::round(b.x * 10.0f) / 10.0f << ","
               << "\"y\":" << std::round(b.y * 10.0f) / 10.0f << "}";
        }
        ss << "]}";

        return ss.str();
    }

    std::string sanitize_player_id(const std::string& id) const {
        std::string clean = "";
        for (char c : id) {
            if (std::isalnum(static_cast<unsigned char>(c)) || c == '_') {
                clean += c;
            }
        }
        if (clean.empty()) clean = "p_anon";
        return clean.substr(0, 32);
    }

    std::string sanitize_player_name(const std::string& name) const {
        std::string clean = "";
        for (char c : name) {
            if (std::isalnum(static_cast<unsigned char>(c)) || c == ' ') {
                clean += c;
            }
        }
        if (clean.empty()) clean = "PilotGaruda";
        return clean.substr(0, 15);
    }

    float clamp_angle(float angle) const {
        if (std::isnan(angle) || std::isinf(angle)) return 0.0f;
        constexpr float PI = 3.14159265f;
        while (angle > PI) angle -= 2.0f * PI;
        while (angle < -PI) angle += 2.0f * PI;
        return angle;
    }

    void handle_json_input(const std::string& raw_player_id, const std::string& json_text) {
        std::string player_id = sanitize_player_id(raw_player_id);

        if (json_text.find("\"action\":\"move\"") != std::string::npos ||
            json_text.find("\"action\": \"move\"") != std::string::npos) {

            float dx = 0.0f, dy = 0.0f, angle = 0.0f;

            try {
                auto dx_pos = json_text.find("\"dx\":");
                if (dx_pos != std::string::npos) dx = std::stof(json_text.substr(dx_pos + 5));

                auto dy_pos = json_text.find("\"dy\":");
                if (dy_pos != std::string::npos) dy = std::stof(json_text.substr(dy_pos + 5));

                auto angle_pos = json_text.find("\"angle\":");
                if (angle_pos != std::string::npos) angle = std::stof(json_text.substr(angle_pos + 8));
            } catch (...) {}

            angle = clamp_angle(angle);
            dx = std::clamp(dx, -2.0f, 2.0f);
            dy = std::clamp(dy, -2.0f, 2.0f);

            process_move(player_id, dx, dy, angle);
        }
        else if (json_text.find("\"action\":\"shoot\"") != std::string::npos ||
                 json_text.find("\"action\": \"shoot\"") != std::string::npos) {
            process_shoot(player_id);
        }
        else if (json_text.find("\"action\":\"join\"") != std::string::npos ||
                 json_text.find("\"action\": \"join\"") != std::string::npos) {
            std::string name = "";
            auto name_pos = json_text.find("\"name\":\"");
            if (name_pos != std::string::npos) {
                auto name_end = json_text.find("\"", name_pos + 8);
                if (name_end != std::string::npos) {
                    name = json_text.substr(name_pos + 8, name_end - (name_pos + 8));
                }
            }
            name = sanitize_player_name(name);

            std::string color = "";
            auto color_pos = json_text.find("\"color\":\"");
            if (color_pos != std::string::npos) {
                auto color_end = json_text.find("\"", color_pos + 9);
                if (color_end != std::string::npos) {
                    color = json_text.substr(color_pos + 9, color_end - (color_pos + 9));
                }
            }

            handle_join(player_id, name, color);
        }
    }
};

} // namespace garuda::game
