// =====================================================================
// GARUDA COMPILER — FASE 15.5 STDLIB MATH TEST SUITE
// =====================================================================

#include "../include/garuda/beam/tools/script_engine.hpp"
#include <iostream>
#include <cassert>
#include <cmath>

int main() {
    garuda::beam::ScriptEngine engine;
    int failures = 0;

    std::cout << "========================================\n";
    std::cout << "FASE 15.5 - STDLIB MATH TESTS\n";
    std::cout << "========================================\n\n";

    // Test 1: Math.akar (sqrt)
    auto r1 = engine.eval("modul TM1 fungsi utama() -> Math.akar(16) rampung");
    if (r1.has_value() && std::holds_alternative<double>(r1->value()) && std::get<double>(r1->value()) == 4.0) {
        std::cout << "[Test 1] Math.akar(16) -> 4.0 [PASS]\n";
    } else {
        std::cout << "[Test 1] Math.akar -> [FAIL]\n";
        failures++;
    }

    // Test 2: Math.pangkat (pow)
    auto r2 = engine.eval("modul TM2 fungsi utama() -> Math.pangkat(2, 8) rampung");
    if (r2.has_value() && std::holds_alternative<double>(r2->value()) && std::get<double>(r2->value()) == 256.0) {
        std::cout << "[Test 2] Math.pangkat(2, 8) -> 256.0 [PASS]\n";
    } else {
        std::cout << "[Test 2] Math.pangkat -> [FAIL]\n";
        failures++;
    }

    // Test 3: Math.abs
    auto r3 = engine.eval("modul TM3 fungsi utama() -> Math.abs(-42) rampung");
    if (r3.has_value() && std::holds_alternative<int64_t>(r3->value()) && std::get<int64_t>(r3->value()) == 42) {
        std::cout << "[Test 3] Math.abs(-42) -> 42 [PASS]\n";
    } else {
        std::cout << "[Test 3] Math.abs -> [FAIL]\n";
        failures++;
    }

    // Test 4: Math.bulat & Math.pi
    auto r4 = engine.eval("modul TM4 fungsi utama() -> Math.bulat(Math.pi()) rampung");
    if (r4.has_value() && std::holds_alternative<int64_t>(r4->value()) && std::get<int64_t>(r4->value()) == 3) {
        std::cout << "[Test 4] Math.bulat(Math.pi()) -> 3 [PASS]\n";
    } else {
        std::cout << "[Test 4] Math.bulat -> [FAIL]\n";
        failures++;
    }

    if (failures > 0) {
        std::cout << "\n❌ " << failures << " STDLIB MATH TESTS FAILED!\n";
        return 1;
    }

    std::cout << "\n========================================\n";
    std::cout << "FASE 15.5 - STDLIB MATH SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
