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
        std::exit(1); \
    }
}

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA QUERY BUILDER & PARAMETER BINDING SUITE\n";
    std::cout << "=======================================================\n\n";

    sqlite3* db = nullptr;
    int rc = sqlite3_open(":memory:", &db);
    GARUDA_CHECK(rc == SQLITE_OK && db != nullptr, "sqlite3_open(:memory:) must succeed");

    // Create Table
    const char* create_sql = "CREATE TABLE users (id INTEGER PRIMARY KEY, name TEXT, age INTEGER);";
    char* err_msg = nullptr;
    rc = sqlite3_exec(db, create_sql, nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        std::string err = err_msg ? err_msg : "Unknown error";
        sqlite3_free(err_msg);
        GARUDA_CHECK(false, "CREATE TABLE failed: " + err);
    }

    // === Test 1: Insert 3 Users & Select All ===
    run_test_with_timeout("Test 1: Insert 3 Users & Select All", [&]() {
        QueryBuilder qb1;
        qb1.insert_into("users", {{"name", "Budi"}, {"age", "18"}});
        bool ok1 = QueryBuilder::execute_prepared(db, qb1.to_sql());
        GARUDA_CHECK(ok1, "Insert Budi must succeed");

        QueryBuilder qb2;
        qb2.insert_into("users", {{"name", "Siti"}, {"age", "25"}});
        bool ok2 = QueryBuilder::execute_prepared(db, qb2.to_sql());
        GARUDA_CHECK(ok2, "Insert Siti must succeed");

        QueryBuilder qb3;
        qb3.insert_into("users", {{"name", "Agus"}, {"age", "30"}});
        bool ok3 = QueryBuilder::execute_prepared(db, qb3.to_sql());
        GARUDA_CHECK(ok3, "Insert Agus must succeed");

        QueryBuilder sel;
        sel.select({"id", "name", "age"}).from("users");
        auto rows = QueryBuilder::fetch_all_prepared(db, sel.to_sql());
        GARUDA_CHECK(rows.size() == 3, "Select all users must return 3 rows");

        std::cout << "  ✓ Test 1: Insert 3 Users & Select All PASS (Count: " << rows.size() << ")\n";
    }, 5);

    // === Test 2: Where Clause with Parameter Binding (age > 20) ===
    run_test_with_timeout("Test 2: Where Clause with Parameter Binding (age > 20)", [&]() {
        QueryBuilder sel;
        sel.select({"name", "age"}).from("users").where_int("age", ">", 20);

        auto built = sel.build();
        GARUDA_CHECK(built.first == "SELECT name, age FROM users WHERE age > ?", "Generated SQL must match parameter binding format");
        GARUDA_CHECK(built.second.size() == 1 && built.second[0] == "20", "Bound parameter must be 20");

        auto rows = QueryBuilder::fetch_all_prepared(db, sel.to_sql());
        GARUDA_CHECK(rows.size() == 2, "Select age > 20 must return 2 users (Siti & Agus)");

        std::cout << "  ✓ Test 2: Where Clause Parameter Binding PASS (Filtered Count: " << rows.size() << ")\n";
    }, 5);

    // === Test 3: SQL Injection Prevention Test ===
    run_test_with_timeout("Test 3: SQL Injection Prevention Test", [&]() {
        std::string malicious_input = "Budi' OR '1'='1";

        QueryBuilder sel;
        sel.select({"id", "name"}).from("users").where("name", "=", malicious_input);

        auto built = sel.build();
        GARUDA_CHECK(built.first == "SELECT id, name FROM users WHERE name = ?", "Malicious string must be bound as parameter ?");
        GARUDA_CHECK(built.second[0] == malicious_input, "Parameter value must equal raw string without evaluation");

        auto rows = QueryBuilder::fetch_all_prepared(db, sel.to_sql());
        GARUDA_CHECK(rows.empty(), "SQL injection input must be treated as literal text and return 0 rows (no data leak)");

        std::cout << "  ✓ Test 3: SQL Injection Prevention PASS (Returned 0 rows for malicious literal string)\n";
    }, 5);

    // === Test 4: Update User Age ===
    run_test_with_timeout("Test 4: Update User Age (Budi -> 30)", [&]() {
        QueryBuilder upd;
        upd.update("users").set("age", "30").where("name", "=", "Budi");
        bool ok = QueryBuilder::execute_prepared(db, upd.to_sql());
        GARUDA_CHECK(ok, "Update statement must execute successfully");

        QueryBuilder sel;
        sel.select({"age"}).from("users").where("name", "=", "Budi");
        auto rows = QueryBuilder::fetch_all_prepared(db, sel.to_sql());
        GARUDA_CHECK(rows.size() == 1 && rows[0][0] == "30", "Budi's age must be updated to 30");

        std::cout << "  ✓ Test 4: Update User Age PASS (New Age: " << rows[0][0] << ")\n";
    }, 5);

    // === Test 5: Delete User ===
    run_test_with_timeout("Test 5: Delete User (Siti)", [&]() {
        QueryBuilder del;
        del.delete_from("users").where("name", "=", "Siti");
        bool ok = QueryBuilder::execute_prepared(db, del.to_sql());
        GARUDA_CHECK(ok, "Delete statement must execute successfully");

        QueryBuilder sel;
        sel.select({"id"}).from("users");
        auto rows = QueryBuilder::fetch_all_prepared(db, sel.to_sql());
        GARUDA_CHECK(rows.size() == 2, "Remaining user count after deleting Siti must be 2");

        std::cout << "  ✓ Test 5: Delete User PASS (Remaining Count: " << rows.size() << ")\n";
    }, 5);

    sqlite3_close(db);

    std::cout << "\n🎉 ALL QUERY BUILDER & SQL INJECTION PREVENTION TESTS PASSED PERFECTLY!\n";
    return 0;
}
