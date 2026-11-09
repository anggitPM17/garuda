// =====================================================================
// GARUDA COMPILER — FASE 15.1 STDLIB LIST TEST SUITE
// =====================================================================

#include "../include/garuda/beam/tools/script_engine.hpp"
#include <iostream>
#include <cassert>
#include <string>

int main() {
    garuda::beam::ScriptEngine engine;
    int failures = 0;

    std::cout << "========================================\n";
    std::cout << "FASE 15.1 - STDLIB LIST TESTS\n";
    std::cout << "========================================\n\n";

    // Test 1: List.panjang
    auto r1 = engine.eval("modul T1 fungsi utama() -> List.panjang([10, 20, 30]) rampung");
    if (r1.has_value() && std::holds_alternative<int64_t>(r1->value()) && std::get<int64_t>(r1->value()) == 3) {
        std::cout << "[Test 1] List.panjang([10, 20, 30]) -> 3 [PASS]\n";
    } else {
        std::cout << "[Test 1] List.panjang -> [FAIL]\n";
        failures++;
    }

    // Test 2: List.ambil (take)
    auto r2 = engine.eval("modul T2 fungsi utama() -> List.panjang(List.ambil([1, 2, 3, 4, 5], 3)) rampung");
    if (r2.has_value() && std::holds_alternative<int64_t>(r2->value()) && std::get<int64_t>(r2->value()) == 3) {
        std::cout << "[Test 2] List.ambil([1,2,3,4,5], 3) length -> 3 [PASS]\n";
    } else {
        std::cout << "[Test 2] List.ambil -> [FAIL]\n";
        failures++;
    }

    // Test 3: List.buang (drop)
    auto r3 = engine.eval("modul T3 fungsi utama() -> List.panjang(List.buang([1, 2, 3, 4, 5], 2)) rampung");
    if (r3.has_value() && std::holds_alternative<int64_t>(r3->value()) && std::get<int64_t>(r3->value()) == 3) {
        std::cout << "[Test 3] List.buang([1,2,3,4,5], 2) length -> 3 [PASS]\n";
    } else {
        std::cout << "[Test 3] List.buang -> [FAIL]\n";
        failures++;
    }

    // Test 4: List.gabung (concat)
    auto r4 = engine.eval("modul T4 fungsi utama() -> List.panjang(List.gabung([1, 2], [3, 4, 5])) rampung");
    if (r4.has_value() && std::holds_alternative<int64_t>(r4->value()) && std::get<int64_t>(r4->value()) == 5) {
        std::cout << "[Test 4] List.gabung([1,2], [3,4,5]) length -> 5 [PASS]\n";
    } else {
        std::cout << "[Test 4] List.gabung -> [FAIL]\n";
        failures++;
    }

    if (failures > 0) {
        std::cout << "\n❌ " << failures << " STDLIB LIST TESTS FAILED!\n";
        return 1;
    }

    std::cout << "\n========================================\n";
    std::cout << "FASE 15.1 - STDLIB LIST SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
