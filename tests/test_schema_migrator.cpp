#include "../include/garuda/beam/stdlib/schema_migrator.hpp"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <string>
#include <vector>

using namespace garuda::beam::stdlib;
namespace fs = std::filesystem;

#define GARUDA_CHECK(cond, msg) \
    if (!(cond)) { \
        std::cerr << "  ❌ ASSERTION FAILED: " << msg << " (" << #cond << ")\n"; \
        std::exit(1); \
    }

void run_test_with_timeout(const std::string& test_name, std::function<void()> test_fn, int timeout_seconds = 10) {
    auto future = std::async(std::launch::async, test_fn);
    if (future.wait_for(std::chrono::seconds(timeout_seconds)) == std::future_status::timeout) {
        std::cerr << "  ❌ " << test_name << " FAILED (TIMEOUT > " << timeout_seconds << "s)\n";
        std::exit(1); \
    }
}

// Helper to check if a column exists in a table via PRAGMA table_info
bool has_column(sqlite3* db, const std::string& table, const std::string& col_name) {
    std::string sql = "PRAGMA table_info(" + table + ");";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK || !stmt) return false;

    bool found = false;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* txt = sqlite3_column_text(stmt, 1); // column name
        if (txt && std::string(reinterpret_cast<const char*>(txt)) == col_name) {
            found = true;
            break;
        }
    }
    sqlite3_finalize(stmt);
    return found;
}

void write_file(const fs::path& path, const std::string& content) {
    std::ofstream ofs(path, std::ios::trunc | std::ios::binary);
    ofs << content;
    ofs.close();
}

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA SCHEMA MIGRATION ENGINE SUITE\n";
    std::cout << "=======================================================\n\n";

    fs::path tmp_dir = fs::temp_directory_path() / "garuda_migrations_test";
    fs::remove_all(tmp_dir);
    fs::create_directories(tmp_dir);

    sqlite3* db = nullptr;
    int rc = sqlite3_open(":memory:", &db);
    GARUDA_CHECK(rc == SQLITE_OK && db != nullptr, "sqlite3_open(:memory:) must succeed");

    // Write initial 2 migration files
    write_file(tmp_dir / "001_create_users.sql", "CREATE TABLE users (id INTEGER PRIMARY KEY, name TEXT);");
    write_file(tmp_dir / "002_add_age.sql", "ALTER TABLE users ADD COLUMN age INTEGER;");

    // === Test 1: Initial Migration Execution (v1 & v2) ===
    run_test_with_timeout("Test 1: Initial Migration Execution", [&]() {
        bool ok = SchemaMigrator::migrate(db, tmp_dir.string());
        GARUDA_CHECK(ok, "Initial migration must succeed");

        int ver = SchemaMigrator::get_current_version(db);
        GARUDA_CHECK(ver == 2, "Current schema version must be 2");
        GARUDA_CHECK(has_column(db, "users", "name"), "Table 'users' must have column 'name'");
        GARUDA_CHECK(has_column(db, "users", "age"), "Table 'users' must have column 'age'");

        std::cout << "  ✓ Test 1: Initial Migration Execution PASS (Current Version: " << ver << ")\n";
    }, 5);

    // === Test 2: Idempotency Check ===
    run_test_with_timeout("Test 2: Idempotency Check", [&]() {
        bool ok = SchemaMigrator::migrate(db, tmp_dir.string());
        GARUDA_CHECK(ok, "Re-running migrate must succeed idempotently without errors");

        int ver = SchemaMigrator::get_current_version(db);
        GARUDA_CHECK(ver == 2, "Schema version must remain 2");

        std::cout << "  ✓ Test 2: Idempotency Check PASS (Version Remains: " << ver << ")\n";
    }, 5);

    // === Test 3: Incremental Migration (v3) ===
    run_test_with_timeout("Test 3: Incremental Migration Execution", [&]() {
        write_file(tmp_dir / "003_add_email.sql", "ALTER TABLE users ADD COLUMN email TEXT;");

        bool ok = SchemaMigrator::migrate(db, tmp_dir.string());
        GARUDA_CHECK(ok, "Incremental migration v3 must succeed");

        int ver = SchemaMigrator::get_current_version(db);
        GARUDA_CHECK(ver == 3, "Schema version must become 3");
        GARUDA_CHECK(has_column(db, "users", "email"), "Table 'users' must have column 'email'");

        std::cout << "  ✓ Test 3: Incremental Migration Execution PASS (Updated Version: " << ver << ")\n";
    }, 5);

    // === Test 4: Transaction Rollback on Error ===
    run_test_with_timeout("Test 4: Transaction Rollback on Error", [&]() {
        // Syntax error SQL file
        write_file(tmp_dir / "004_bad_query.sql", "INVALID SQL STATEMENT SYNTAX ERROR;;;");

        bool ok = SchemaMigrator::migrate(db, tmp_dir.string());
        GARUDA_CHECK(!ok, "Migrate with bad SQL query must return false");

        int ver = SchemaMigrator::get_current_version(db);
        GARUDA_CHECK(ver == 3, "Schema version must remain 3 due to transaction rollback");
        GARUDA_CHECK(!has_column(db, "users", "invalid_column"), "Column 'invalid_column' must NOT be added due to rollback");

        std::cout << "  ✓ Test 4: Transaction Rollback on Error PASS (Rolled Back cleanly to Version: " << ver << ")\n";
    }, 5);

    sqlite3_close(db);
    fs::remove_all(tmp_dir);

    std::cout << "\n🎉 ALL SCHEMA MIGRATION ENGINE TESTS PASSED PERFECTLY!\n";
    return 0;
}
