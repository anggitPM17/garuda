#include "../include/garuda/beam/stdlib/db.hpp"
#include <cassert>
#include <filesystem>
#include <iostream>
#include <thread>
#include <vector>

using namespace garuda::beam::stdlib;
using namespace garuda::beam;
namespace fs = std::filesystem;

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA STANDARD LIBRARY — SPRINT C.4 DATABASE TEST SUITE (10 TESTS)\n";
    std::cout << "=======================================================\n\n";

    fs::path tmp_db_file = fs::temp_directory_path() / "garuda_test_db_dir";
    fs::remove_all(tmp_db_file);

    // === Test 1: Open Database ===
    std::cout << "[TEST 1/10] Running Open Database test...\n";
    Database db(tmp_db_file.string());
    assert(fs::exists(tmp_db_file));
    std::cout << "  ✓ Test 1: Open Database PASS\n";

    // === Test 2: Insert Record ===
    std::cout << "[TEST 2/10] Running Insert Record test...\n";
    Term user1_rec = list_of({tuple_of({Term("nama"), Term("Budi")}), tuple_of({Term("umur"), Term(42LL)})});
    bool ok2 = db.insert("users", "usr_101", user1_rec);
    assert(ok2);
    assert(db.count("users") == 1);
    std::cout << "  ✓ Test 2: Insert Record PASS\n";

    // === Test 3: Query Record by Key ===
    std::cout << "[TEST 3/10] Running Query Record by Key test...\n";
    auto rec3 = db.get("users", "usr_101");
    assert(rec3.has_value());
    assert(rec3->get_if<garuda::beam::List>() != nullptr);
    std::cout << "  ✓ Test 3: Query Record by Key PASS\n";

    // === Test 4: Update Existing Record ===
    std::cout << "[TEST 4/10] Running Update Record test...\n";
    Term user1_updated = list_of({tuple_of({Term("nama"), Term("Budi Utomo")}), tuple_of({Term("umur"), Term(43LL)})});
    bool ok4 = db.update("users", "usr_101", user1_updated);
    assert(ok4);
    auto rec4 = db.get("users", "usr_101");
    assert(rec4.has_value());
    std::cout << "  ✓ Test 4: Update Record PASS\n";

    // === Test 5: Delete Record ===
    std::cout << "[TEST 5/10] Running Delete Record test...\n";
    db.insert("users", "usr_temp", Term("temporary_data"));
    assert(db.count("users") == 2);
    bool ok5 = db.remove("users", "usr_temp");
    assert(ok5);
    assert(db.count("users") == 1);
    assert(!db.get("users", "usr_temp").has_value());
    std::cout << "  ✓ Test 5: Delete Record PASS\n";

    // === Test 6: Query All Records in Table ===
    std::cout << "[TEST 6/10] Running Query All Records test...\n";
    db.insert("users", "usr_102", Term("Siti"));
    auto all_users = db.query_all("users");
    assert(all_users.size() == 2);
    std::cout << "  ✓ Test 6: Query All Records PASS\n";

    // === Test 7: Persistence Check (Flush & Re-open) ===
    std::cout << "[TEST 7/10] Running Persistence Check test...\n";
    db.flush();
    {
        Database db_reopened(tmp_db_file.string());
        assert(db_reopened.count("users") == 2);
        auto check_rec = db_reopened.get("users", "usr_101");
        assert(check_rec.has_value());
    }
    std::cout << "  ✓ Test 7: Persistence Check PASS\n";

    // === Test 8: Multi-Table Isolation ===
    std::cout << "[TEST 8/10] Running Multi-Table Isolation test...\n";
    db.insert("products", "prod_1", Term("Laptop Garuda"));
    assert(db.count("products") == 1);
    assert(db.count("users") == 2);
    assert(!db.get("users", "prod_1").has_value());
    std::cout << "  ✓ Test 8: Multi-Table Isolation PASS\n";

    // === Test 9: Concurrent Multi-Thread Inserts ===
    std::cout << "[TEST 9/10] Running Concurrent Multi-Thread Inserts test...\n";
    std::vector<std::thread> workers;
    for (int i = 0; i < 5; ++i) {
        workers.emplace_back([&db, i]() {
            std::string key = "usr_concurrent_" + std::to_string(i);
            db.insert("users", key, Term(i * 100));
        });
    }
    for (auto& w : workers) {
        if (w.joinable()) w.join();
    }
    assert(db.count("users") == 7); // 2 previous + 5 concurrent = 7!
    std::cout << "  ✓ Test 9: Concurrent Multi-Thread Inserts PASS\n";

    // === Test 10: Non-Existent Table/Key Error Handling ===
    std::cout << "[TEST 10/10] Running Non-Existent Error Handling test...\n";
    assert(!db.get("non_existent_table", "key").has_value());
    assert(!db.get("users", "key_that_does_not_exist").has_value());
    assert(!db.remove("non_existent_table", "key"));
    std::cout << "  ✓ Test 10: Non-Existent Error Handling PASS\n";

    // Clean up
    fs::remove_all(tmp_db_file);

    std::cout << "\n🎉 ALL 10 DATABASE ADAPTER TESTS PASSED PERFECTLY!\n";
    return 0;
}
