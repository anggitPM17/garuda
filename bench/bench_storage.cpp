#include "../include/garuda/beam/stdlib/storage_engine.hpp"
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
    std::cout << "GARUDA STORAGE ENGINE PERFORMANCE BENCHMARK (SPRINT E.0)\n";
    std::cout << "=======================================================\n\n";

    fs::path bench_db_dir = fs::temp_directory_path() / "garuda_bench_storage_db";
    fs::remove_all(bench_db_dir);

    StorageEngine db(bench_db_dir.string());

    // Benchmark 1: Insert 10,000 Records
    std::cout << "[BENCH 1] Inserting 10,000 records...\n";
    auto start_1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 10000; ++i) {
        db.insert("users", "usr_" + std::to_string(i), Term("User " + std::to_string(i)));
    }
    auto elapsed_1 = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now() - start_1).count();
    std::cout << "  ⏱️ 10,000 Inserts completed in " << elapsed_1 << " ms ("
              << (10000.0 / (elapsed_1 / 1000.0)) << " ops/sec)\n\n";

    // Benchmark 2: Query 1,000 Records by Key
    std::cout << "[BENCH 2] Querying 1,000 records by key...\n";
    auto start_2 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000; ++i) {
        auto rec = db.get("users", "usr_" + std::to_string(i * 10));
        (void)rec;
    }
    auto elapsed_2 = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now() - start_2).count();
    std::cout << "  ⏱️ 1,000 Lookups completed in " << (elapsed_2 / 1000.0) << " ms ("
              << (1000.0 / (elapsed_2 / 1000000.0)) << " ops/sec)\n\n";

    // Benchmark 3: Concurrent 10-Thread Inserts (1,000 ops each = 10,000 total)
    std::cout << "[BENCH 3] Running Concurrent 10-Thread Inserts (10,000 total ops)...\n";
    auto start_3 = std::chrono::high_resolution_clock::now();
    std::vector<std::thread> workers;
    for (int t = 0; t < 10; ++t) {
        workers.emplace_back([&db, t]() {
            for (int i = 0; i < 1000; ++i) {
                std::string key = "usr_concurrent_" + std::to_string(t) + "_" + std::to_string(i);
                db.insert("users", key, Term(i));
            }
        });
    }
    for (auto& w : workers) {
        if (w.joinable()) w.join();
    }
    auto elapsed_3 = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now() - start_3).count();
    std::cout << "  ⏱️ Concurrent 10-Thread Inserts completed in " << elapsed_3 << " ms ("
              << (10000.0 / (elapsed_3 / 1000.0)) << " ops/sec)\n\n";

    // Benchmark 4: Checkpoint & Compaction Speed
    std::cout << "[BENCH 4] Executing Storage Engine Checkpoint & Compaction...\n";
    auto start_4 = std::chrono::high_resolution_clock::now();
    db.checkpoint();
    auto elapsed_4 = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now() - start_4).count();
    std::cout << "  ⏱️ Checkpoint & WAL Compaction completed in " << elapsed_4 << " ms\n\n";

    fs::remove_all(bench_db_dir);
    std::cout << "🎉 ALL STORAGE ENGINE BENCHMARKS EXECUTED SUCCESSFULLY!\n";
    return 0;
}
