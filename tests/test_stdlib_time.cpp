// =====================================================================
// GARUDA COMPILER — FASE 15.6 STDLIB TIME TEST SUITE
// =====================================================================

#include "../include/garuda/beam/tools/script_engine.hpp"
#include <iostream>
#include <cassert>
#include <string>

int main() {
    garuda::beam::ScriptEngine engine;
    int failures = 0;

    std::cout << "========================================\n";
    std::cout << "FASE 15.6 - STDLIB TIME TESTS\n";
    std::cout << "========================================\n\n";

    // Test 1: Time.sekarang (now)
    auto r1 = engine.eval("modul TT1 fungsi utama() -> Time.sekarang() rampung");
    if (r1.has_value() && std::holds_alternative<int64_t>(r1->value()) && std::get<int64_t>(r1->value()) > 1700000000LL) {
        std::cout << "[Test 1] Time.sekarang() -> Valid Epoch Seconds [PASS]\n";
    } else {
        std::cout << "[Test 1] Time.sekarang -> [FAIL]\n";
        failures++;
    }

    // Test 2: Time.format
    auto r2 = engine.eval("modul TT2 fungsi utama() -> Time.format(1730000000) rampung");
    if (r2.has_value() && std::holds_alternative<std::string>(r2->value()) && !std::get<std::string>(r2->value()).empty()) {
        std::cout << "[Test 2] Time.format(1730000000) -> \"" << std::get<std::string>(r2->value()) << "\" [PASS]\n";
    } else {
        std::cout << "[Test 2] Time.format -> [FAIL]\n";
        failures++;
    }

    // Test 3: Time.tambah_detik
    auto r3 = engine.eval("modul TT3 fungsi utama() -> Time.tambah_detik(1000, 300) rampung");
    if (r3.has_value() && std::holds_alternative<int64_t>(r3->value()) && std::get<int64_t>(r3->value()) == 1300) {
        std::cout << "[Test 3] Time.tambah_detik(1000, 300) -> 1300 [PASS]\n";
    } else {
        std::cout << "[Test 3] Time.tambah_detik -> [FAIL]\n";
        failures++;
    }

    if (failures > 0) {
        std::cout << "\n❌ " << failures << " STDLIB TIME TESTS FAILED!\n";
        return 1;
    }

    std::cout << "\n========================================\n";
    std::cout << "FASE 15.6 - STDLIB TIME SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
