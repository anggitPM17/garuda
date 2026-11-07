// =====================================================================
// GARUDA COMPILER — FASE 11 IN-MEMORY TABLE (ETS-STYLE) TEST SUITE (RED -> GREEN)
// =====================================================================

#include "../include/garuda/beam/tools/script_engine.hpp"
#include "../include/garuda/beam/stdlib/ets.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>

int main() {
    garuda::beam::ScriptEngine engine;
    int failures = 0;

    std::cout << "=======================================================\n";
    std::cout << "GARUDA COMPILER — FASE 11 IN-MEMORY TABLE (ETS) TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test 1: Et s.baru() -> assert table ID > 0
    auto r1 = engine.eval("modul TEts1 fungsi utama() -> Et s.baru() rampung");
    std::int64_t tid = 0;
    if (r1.has_value() && std::holds_alternative<std::int64_t>(r1->value())) {
        tid = std::get<std::int64_t>(r1->value());
    }

    if (tid > 0) {
        std::cout << "[Test 1] Et s.baru() -> Created Table ID: " << tid << " [PASS]\n";
    } else {
        std::cout << "[Test 1] Et s.baru() -> [FAIL] Expected positive table ID, got 0\n";
        failures++;
    }

    // Test 2: Et s.sisip(T, "user_1", "Anggit") then Et s.cari(T, "user_1") -> assert "Anggit"
    std::string code2 = "modul TEts2 fungsi utama() -> T = Et s.baru(), Et s.sisip(T, 'user_1', 'Anggit'), Et s.cari(T, 'user_1') rampung";
    auto r2 = engine.eval(code2);
    if (r2.has_value() && std::holds_alternative<std::string>(r2->value()) && std::get<std::string>(r2->value()) == "Anggit") {
        std::cout << "[Test 2] Et s.sisip & Et s.cari -> Matched Value 'Anggit' [PASS]\n";
    } else {
        std::cout << "[Test 2] Et s.sisip & Et s.cari -> [FAIL] Expected 'Anggit'\n";
        failures++;
    }

    // Test 3: Overwrite value with same key -> assert updated value "Tebet"
    std::string code3 = "modul TEts3 fungsi utama() -> T = Et s.baru(), Et s.sisip(T, 'kota', 'Jakarta'), Et s.sisip(T, 'kota', 'Tebet'), Et s.cari(T, 'kota') rampung";
    auto r3 = engine.eval(code3);
    if (r3.has_value() && std::holds_alternative<std::string>(r3->value()) && std::get<std::string>(r3->value()) == "Tebet") {
        std::cout << "[Test 3] Overwrite Key with Updated Value 'Tebet' [PASS]\n";
    } else {
        std::cout << "[Test 3] Overwrite Key -> [FAIL] Expected 'Tebet'\n";
        failures++;
    }

    // Test 4: Et s.hapus(T, "user_1") -> assert Et s.cari returns nil
    std::string code4 = "modul TEts4 fungsi utama() -> T = Et s.baru(), Et s.sisip(T, 'key1', 'val1'), Et s.hapus(T, 'key1'), Et s.cari(T, 'key1') rampung";
    auto r4 = engine.eval(code4);
    if (r4.has_value() && garuda::beam::is_atom(*r4, "nil")) {
        std::cout << "[Test 4] Et s.hapus Key -> Lookup returns nil [PASS]\n";
    } else {
        std::cout << "[Test 4] Et s.hapus Key -> [FAIL] Expected nil after delete\n";
        failures++;
    }

    // Test 5: Concurrent Multi-threaded Stress Test (1000 inserts from 10 parallel threads)
    const std::int64_t stid = 999;
    auto table = garuda::beam::stdlib::EtsManager::instance().get_table(stid);
    if (!table) {
        std::int64_t created = garuda::beam::stdlib::EtsManager::instance().create_table();
        table = garuda::beam::stdlib::EtsManager::instance().get_table(created);
    }

    std::atomic<int> insert_count{0};
    std::vector<std::thread> threads;
    for (int t = 0; t < 10; ++t) {
        threads.emplace_back([t, table, &insert_count]() {
            for (int i = 0; i < 100; ++i) {
                std::string k = "key_" + std::to_string(t) + "_" + std::to_string(i);
                table->insert(k, garuda::beam::Term(static_cast<int64_t>(t * 100 + i)));
                insert_count++;
            }
        });
    }

    for (auto& th : threads) {
        if (th.joinable()) th.join();
    }

    if (failures == 0 && insert_count.load() == 1000 && table->size() == 1000) {
        std::cout << "[Test 5] 1000 Concurrent Inserts Across 10 Threads -> Consistent 1000 Items [PASS]\n";
    } else {
        std::cout << "[Test 5] Concurrent Multi-threaded Stress Test -> [FAIL] Prior tests failed or table size: " << (table ? table->size() : 0) << "/1000\n";
        failures++;
    }

    if (failures > 0) {
        std::cout << "\n❌ " << failures << "/5 TESTS FAILED ON STUB CODE!\n";
        return 1;
    }

    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL 5 FASE 11 IN-MEMORY TABLE (ETS) TESTS PASSED PERFECTLY!\n";
    std::cout << "========================================\n";
    return 0;
}
