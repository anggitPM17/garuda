#include "sqlite3.h"
#include <chrono>
#include <future>
#include <iostream>
#include <string>

#define GARUDA_CHECK(cond, msg) \
    if (!(cond)) { \
        std::cerr << "  ❌ ASSERTION FAILED: " << msg << " (" << #cond << ")\n"; \
        std::exit(1); \
    }

void run_test_with_timeout(const std::string& test_name, std::function<void()> test_fn, int timeout_seconds = 5) {
    auto future = std::async(std::launch::async, test_fn);
    if (future.wait_for(std::chrono::seconds(timeout_seconds)) == std::future_status::timeout) {
        std::cerr << "  ❌ " << test_name << " FAILED (TIMEOUT > " << timeout_seconds << "s)\n";
        std::exit(1);
    }
}

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA SQLITE3 AMALGAMATION DRIVER SUITE\n";
    std::cout << "=======================================================\n\n";

    // === Test 1: SQLite3 In-Memory Create Table, Insert & Select ===
    run_test_with_timeout("Test 1: SQLite3 In-Memory Operations", []() {
        sqlite3* db = nullptr;
        int rc = sqlite3_open(":memory:", &db);
        GARUDA_CHECK(rc == SQLITE_OK && db != nullptr, "sqlite3_open(:memory:) must succeed");

        // 1. Create Table
        const char* create_sql = "CREATE TABLE users (id INTEGER PRIMARY KEY, name TEXT);";
        char* err_msg = nullptr;
        rc = sqlite3_exec(db, create_sql, nullptr, nullptr, &err_msg);
        if (rc != SQLITE_OK) {
            std::string err = err_msg ? err_msg : "Unknown error";
            sqlite3_free(err_msg);
            GARUDA_CHECK(false, "CREATE TABLE failed: " + err);
        }

        // 2. Insert Data
        const char* insert_sql = "INSERT INTO users (name) VALUES ('Budi');";
        rc = sqlite3_exec(db, insert_sql, nullptr, nullptr, &err_msg);
        if (rc != SQLITE_OK) {
            std::string err = err_msg ? err_msg : "Unknown error";
            sqlite3_free(err_msg);
            GARUDA_CHECK(false, "INSERT INTO users failed: " + err);
        }

        // 3. Select Data
        sqlite3_stmt* stmt = nullptr;
        const char* select_sql = "SELECT name FROM users WHERE id=1;";
        rc = sqlite3_prepare_v2(db, select_sql, -1, &stmt, nullptr);
        GARUDA_CHECK(rc == SQLITE_OK && stmt != nullptr, "sqlite3_prepare_v2 failed");

        rc = sqlite3_step(stmt);
        GARUDA_CHECK(rc == SQLITE_ROW, "sqlite3_step must return SQLITE_ROW");

        const unsigned char* name_txt = sqlite3_column_text(stmt, 0);
        GARUDA_CHECK(name_txt != nullptr, "Column text must not be null");

        std::string name_str(reinterpret_cast<const char*>(name_txt));
        GARUDA_CHECK(name_str == "Budi", "Queried name must be 'Budi'");

        sqlite3_finalize(stmt);
        sqlite3_close(db);

        std::cout << "  ✓ Test 1: SQLite3 In-Memory Operations PASS (Retrieved: " << name_str << ")\n";
    }, 5);

    std::cout << "\n🎉 ALL SQLITE3 DRIVER TESTS PASSED PERFECTLY!\n";
    return 0;
}
