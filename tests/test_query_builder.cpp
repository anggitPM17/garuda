#include "../include/garuda/beam/stdlib/query_builder.hpp"
#include <chrono>
#include <future>
#include <iostream>

using namespace garuda::beam::stdlib;

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
    std::cout << "GARUDA QUERY BUILDER & PARAMETER BINDING SUITE\n";
    std::cout << "=======================================================\n\n";

    // === Test 1: QueryBuilder In-Memory SQLite Prepared Statements ===
    run_test_with_timeout("Test 1: QueryBuilder Insert & Select Prepared", []() {
        sqlite3* db = nullptr;
        int rc = sqlite3_open(":memory:", &db);
        GARUDA_CHECK(rc == SQLITE_OK && db != nullptr, "sqlite3_open(:memory:) must succeed");

        // 1. Create Table
        const char* create_sql = "CREATE TABLE users (id INTEGER PRIMARY KEY, name TEXT, age INTEGER);";
        char* err_msg = nullptr;
        rc = sqlite3_exec(db, create_sql, nullptr, nullptr, &err_msg);
        if (rc != SQLITE_OK) {
            std::string err = err_msg ? err_msg : "Unknown error";
            sqlite3_free(err_msg);
            GARUDA_CHECK(false, "CREATE TABLE failed: " + err);
        }

        // 2. Insert User 1 via QueryBuilder
        auto ins1 = QueryBuilder::insert_into("users", {"name", "age"}, {SqlParam::make_text("Budi"), SqlParam::make_int(25)}).to_sql();
        GARUDA_CHECK(ins1.sql == "INSERT INTO users (name, age) VALUES (?, ?)", "Insert SQL must match prepared format");
        bool ok1 = QueryBuilder::execute_prepared(db, ins1);
        GARUDA_CHECK(ok1, "Executing ins1 must succeed");

        // 3. Insert User 2 via QueryBuilder
        auto ins2 = QueryBuilder::insert_into("users", {"name", "age"}, {SqlParam::make_text("Siti"), SqlParam::make_int(30)}).to_sql();
        bool ok2 = QueryBuilder::execute_prepared(db, ins2);
        GARUDA_CHECK(ok2, "Executing ins2 must succeed");

        // 4. Select Users with age >= 28 via QueryBuilder
        auto q_select = QueryBuilder::select_from("users", {"id", "name", "age"}).where_int("age", ">=", 28).to_sql();
        GARUDA_CHECK(q_select.sql == "SELECT id, name, age FROM users WHERE age >= ?", "Select SQL must match prepared format");
        GARUDA_CHECK(q_select.params.size() == 1, "Select must have 1 bound parameter");

        auto rows = QueryBuilder::fetch_all_prepared(db, q_select);
        GARUDA_CHECK(rows.size() == 1, "Must return 1 row for age >= 28");
        GARUDA_CHECK(rows[0][1] == "Siti", "User name must be 'Siti'");
        GARUDA_CHECK(rows[0][2] == "30", "User age must be '30'");

        sqlite3_close(db);

        std::cout << "  ✓ Test 1: QueryBuilder Insert & Select Prepared PASS (Matched Name: " << rows[0][1] << ")\n";
    }, 5);

    std::cout << "\n🎉 ALL QUERY BUILDER & PARAMETER BINDING TESTS PASSED PERFECTLY!\n";
    return 0;
}
