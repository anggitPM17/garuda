#pragma once
// Garuda Standard Library: Schema Migration Engine (`schema_migrator.hpp`)
#include "sqlite3.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace garuda::beam::stdlib {

namespace fs = std::filesystem;

class SchemaMigrator {
public:
    static bool init(sqlite3* db) {
        if (!db) return false;
        const char* sql =
            "CREATE TABLE IF NOT EXISTS schema_migrations ("
            "version INTEGER PRIMARY KEY, "
            "applied_at TEXT DEFAULT CURRENT_TIMESTAMP"
            ");";
        char* err_msg = nullptr;
        int rc = sqlite3_exec(db, sql, nullptr, nullptr, &err_msg);
        if (rc != SQLITE_OK) {
            if (err_msg) sqlite3_free(err_msg);
            return false;
        }
        return true;
    }

    static int get_current_version(sqlite3* db) {
        if (!db) return 0;
        init(db);

        const char* sql = "SELECT MAX(version) FROM schema_migrations;";
        sqlite3_stmt* stmt = nullptr;
        int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK || !stmt) return 0;

        int current_version = 0;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            if (sqlite3_column_type(stmt, 0) != SQLITE_NULL) {
                current_version = sqlite3_column_int(stmt, 0);
            }
        }
        sqlite3_finalize(stmt);
        return current_version;
    }

    static bool apply_migration(sqlite3* db, int version, const std::string& sql_content) {
        if (!db) return false;

        char* err_msg = nullptr;
        // 1. Begin Transaction
        int rc = sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, &err_msg);
        if (rc != SQLITE_OK) {
            if (err_msg) { sqlite3_free(err_msg); err_msg = nullptr; }
            return false;
        }

        // 2. Execute Migration SQL
        rc = sqlite3_exec(db, sql_content.c_str(), nullptr, nullptr, &err_msg);
        if (rc != SQLITE_OK) {
            if (err_msg) { sqlite3_free(err_msg); err_msg = nullptr; }
            sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
            return false;
        }

        // 3. Record Version in schema_migrations
        std::string record_sql = "INSERT INTO schema_migrations (version) VALUES (" + std::to_string(version) + ");";
        rc = sqlite3_exec(db, record_sql.c_str(), nullptr, nullptr, &err_msg);
        if (rc != SQLITE_OK) {
            if (err_msg) { sqlite3_free(err_msg); err_msg = nullptr; }
            sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
            return false;
        }

        // 4. Commit Transaction
        rc = sqlite3_exec(db, "COMMIT;", nullptr, nullptr, &err_msg);
        if (rc != SQLITE_OK) {
            if (err_msg) { sqlite3_free(err_msg); err_msg = nullptr; }
            sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
            return false;
        }

        return true;
    }

    static bool migrate(sqlite3* db, const std::string& migrations_dir) {
        if (!db || !fs::exists(migrations_dir)) return false;
        init(db);

        int curr_ver = get_current_version(db);

        // Collect and sort .sql migration files by numeric prefix
        std::map<int, fs::path> migration_files;
        for (const auto& entry : fs::directory_iterator(migrations_dir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".sql") {
                std::string fname = entry.path().filename().string();
                int ver = extract_version_prefix(fname);
                if (ver > 0) {
                    migration_files[ver] = entry.path();
                }
            }
        }

        // Apply pending migrations in ascending version order
        for (const auto& [ver, path] : migration_files) {
            if (ver > curr_ver) {
                std::ifstream ifs(path);
                if (!ifs.is_open()) return false;

                std::string sql_content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
                ifs.close();

                bool ok = apply_migration(db, ver, sql_content);
                if (!ok) {
                    std::cerr << "  ⚠️ Migration version " << ver << " (" << path.filename().string() << ") failed & rolled back!\n";
                    return false;
                }
            }
        }

        return true;
    }

private:
    static int extract_version_prefix(const std::string& filename) {
        std::string num_str;
        for (char c : filename) {
            if (std::isdigit(static_cast<unsigned char>(c))) {
                num_str += c;
            } else {
                break;
            }
        }
        if (num_str.empty()) return 0;
        try {
            return std::stoi(num_str);
        } catch (...) {
            return 0;
        }
    }
};

} // namespace garuda::beam::stdlib
