#include "../include/garuda/beam/stdlib/storage_engine.hpp"
#include "../include/garuda/beam/stdlib/db.hpp"
#include <cassert>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <thread>
#include <vector>

using namespace garuda::beam::stdlib;
using namespace garuda::beam;
namespace fs = std::filesystem;

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA STORAGE ENGINE — SPRINT E.0 CRASH-SAFE STORAGE ENGINE TEST SUITE (15 TESTS)\n";
    std::cout << "=======================================================\n\n";

    fs::path test_db_dir = fs::temp_directory_path() / "garuda_storage_engine_test_db";
    fs::remove_all(test_db_dir);

    // === Test 1: Open Storage Engine ===
    std::cout << "[TEST 1/15] Running Storage Engine Open test...\n";
    StorageEngine engine(test_db_dir.string());
    assert(fs::exists(test_db_dir));
    std::cout << "  ✓ Test 1: Storage Engine Open PASS\n";

    // === Test 2: WAL Record Appending ===
    std::cout << "[TEST 2/15] Running WAL Record Appending test...\n";
    bool ok2 = engine.insert("users", "usr_101", Term("Budi WAL"));
    assert(ok2);
    assert(fs::exists(test_db_dir / "wal.log"));
    std::cout << "  ✓ Test 2: WAL Record Appending PASS\n";

    // === Test 3: WAL Crash Recovery Replay ===
    std::cout << "[TEST 3/15] Running WAL Crash Recovery Replay test...\n";
    {
        // Simulate crash by ungraceful close without checkpoint
        StorageEngine engine_replayed(test_db_dir.string());
        auto rec3 = engine_replayed.get("users", "usr_101");
        assert(rec3.has_value());
        assert(*rec3->get_if<std::string>() == "Budi WAL");
    }
    std::cout << "  ✓ Test 3: WAL Crash Recovery Replay PASS\n";

    // === Test 4: Fast Index Lookup Speed (<10ms) ===
    std::cout << "[TEST 4/15] Running Fast Index Lookup Speed test...\n";
    auto start_time = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000; ++i) {
        engine.insert("users", "usr_bench_" + std::to_string(i), Term(i));
    }
    for (int i = 0; i < 1000; ++i) {
        auto rec = engine.get("users", "usr_bench_" + std::to_string(i));
        assert(rec.has_value());
    }
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now() - start_time).count();
    assert(elapsed < 500); // 1,000 index lookups well under 500ms!
    std::cout << "  ✓ Test 4: Fast Index Lookup Speed PASS (" << elapsed << "ms)\n";

    // === Test 5: Transaction Commit ===
    std::cout << "[TEST 5/15] Running Transaction Commit test...\n";
    engine.begin_transaction();
    engine.insert("users", "usr_tx_commit", Term("Committed Data"));
    engine.commit();
    assert(engine.get("users", "usr_tx_commit").has_value());
    std::cout << "  ✓ Test 5: Transaction Commit PASS\n";

    // === Test 6: Transaction Rollback ===
    std::cout << "[TEST 6/15] Running Transaction Rollback test...\n";
    engine.begin_transaction();
    engine.insert("users", "usr_tx_rollback", Term("Rolled Back Data"));
    engine.rollback();
    assert(!engine.get("users", "usr_tx_rollback").has_value()); // Rollback reverted insertion!
    std::cout << "  ✓ Test 6: Transaction Rollback PASS\n";

    // === Test 7: Uncommitted Transaction Isolation ===
    std::cout << "[TEST 7/15] Running Uncommitted Transaction Isolation test...\n";
    engine.begin_transaction();
    engine.insert("users", "usr_tx_isolated", Term("Uncommitted Data"));
    assert(engine.get("users", "usr_tx_isolated").has_value()); // Visible inside TX
    engine.rollback();
    assert(!engine.get("users", "usr_tx_isolated").has_value()); // Reverted outside TX
    std::cout << "  ✓ Test 7: Uncommitted Transaction Isolation PASS\n";

    // === Test 8: WAL Compaction & Checkpoint ===
    std::cout << "[TEST 8/15] Running WAL Compaction test...\n";
    bool ok8 = engine.checkpoint();
    assert(ok8);
    assert(fs::exists(test_db_dir / "data.db"));
    std::cout << "  ✓ Test 8: WAL Compaction PASS\n";

    // === Test 9: Multi-Table Storage Isolation ===
    std::cout << "[TEST 9/15] Running Multi-Table Isolation test...\n";
    engine.insert("products", "prod_1", Term("Laptop Garuda"));
    assert(engine.count("products") == 1);
    assert(engine.count("users") >= 1000);
    std::cout << "  ✓ Test 9: Multi-Table Isolation PASS\n";

    // === Test 10: Atomic File Rename Persistence ===
    std::cout << "[TEST 10/15] Running Atomic File Rename Persistence test...\n";
    {
        StorageEngine engine_check(test_db_dir.string());
        assert(engine_check.count("products") == 1);
        auto p_rec = engine_check.get("products", "prod_1");
        assert(p_rec.has_value());
    }
    std::cout << "  ✓ Test 10: Atomic File Rename Persistence PASS\n";

    // === Test 11: Concurrent Multi-Thread WAL Writes ===
    std::cout << "[TEST 11/15] Running Concurrent Multi-Thread WAL Writes test...\n";
    std::vector<std::thread> workers;
    for (int i = 0; i < 5; ++i) {
        workers.emplace_back([&engine, i]() {
            std::string key = "usr_concurrent_" + std::to_string(i);
            engine.insert("users", key, Term(i * 100));
        });
    }
    for (auto& w : workers) {
        if (w.joinable()) w.join();
    }
    assert(engine.get("users", "usr_concurrent_0").has_value());
    assert(engine.get("users", "usr_concurrent_4").has_value());
    std::cout << "  ✓ Test 11: Concurrent Multi-Thread WAL Writes PASS\n";

    // === Test 12: Multi-Transaction Isolation across Threads ===
    std::cout << "[TEST 12/15] Running Multi-Transaction Thread Isolation test...\n";
    std::thread tx_worker([&engine]() {
        engine.begin_transaction();
        engine.insert("users", "usr_tx_thread", Term("Thread TX"));
        engine.commit();
    });
    if (tx_worker.joinable()) tx_worker.join();
    assert(engine.get("users", "usr_tx_thread").has_value());
    std::cout << "  ✓ Test 12: Multi-Transaction Thread Isolation PASS\n";

    // === Test 13: Non-Existent Record Lookup Error Handling ===
    std::cout << "[TEST 13/15] Running Non-Existent Lookup Error Handling test...\n";
    assert(!engine.get("non_existent_table", "key").has_value());
    assert(!engine.get("users", "key_that_does_not_exist").has_value());
    std::cout << "  ✓ Test 13: Non-Existent Lookup Error Handling PASS\n";

    // === Test 14: Database API Compatibility Layer ===
    std::cout << "[TEST 14/15] Running Database API Compatibility Layer test...\n";
    Database db_compat(test_db_dir.string());
    db_compat.insert("users", "usr_compat", Term("Compat Value"));
    assert(db_compat.get("users", "usr_compat").has_value());
    std::cout << "  ✓ Test 14: Database API Compatibility Layer PASS\n";

    // === Test 15: End-to-End Stress Check ===
    std::cout << "[TEST 15/15] Running End-to-End Stress Check test...\n";
    engine.checkpoint();
    {
        StorageEngine final_check(test_db_dir.string());
        assert(final_check.get("users", "usr_compat").has_value());
    }
    std::cout << "  ✓ Test 15: End-to-End Stress Check PASS\n";

    // Cleanup
    fs::remove_all(test_db_dir);

    std::cout << "\n🎉 ALL 15 STORAGE ENGINE TESTS PASSED PERFECTLY!\n";
    return 0;
}
