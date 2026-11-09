// =====================================================================
// GARUDA COMPILER — FASE 15.2 STDLIB MAP TEST SUITE
// =====================================================================

#include "../include/garuda/beam/tools/script_engine.hpp"
#include <iostream>
#include <cassert>
#include <string>

int main() {
    garuda::beam::ScriptEngine engine;
    int failures = 0;

    std::cout << "========================================\n";
    std::cout << "FASE 15.2 - STDLIB MAP TESTS\n";
    std::cout << "========================================\n\n";

    // Test 1: Map.baru & Map.taruh & Map.ambil
    std::string code1 = R"(
        modul TM1
        fungsi utama() ->
            m = Map.baru()
            m2 = Map.taruh(m, "nama", "Anggit")
            Map.ambil(m2, "nama")
        rampung
    )";
    auto r1 = engine.eval(code1);
    if (r1.has_value() && std::holds_alternative<std::string>(r1->value()) && std::get<std::string>(r1->value()) == "Anggit") {
        std::cout << "[Test 1] Map.taruh & Map.ambil -> 'Anggit' [PASS]\n";
    } else {
        std::cout << "[Test 1] Map.taruh & Map.ambil -> [FAIL]\n";
        failures++;
    }

    // Test 2: Map.berisi
    std::string code2 = R"(
        modul TM2
        fungsi utama() ->
            m = Map.baru()
            m2 = Map.taruh(m, "kota", "Semarang")
            Map.berisi(m2, "kota")
        rampung
    )";
    auto r2 = engine.eval(code2);
    if (r2.has_value() && garuda::beam::is_atom(*r2, "benar")) {
        std::cout << "[Test 2] Map.berisi -> benar [PASS]\n";
    } else {
        std::cout << "[Test 2] Map.berisi -> [FAIL]\n";
        failures++;
    }

    // Test 3: Map.hapus
    std::string code3 = R"(
        modul TM3
        fungsi utama() ->
            m = Map.baru()
            m2 = Map.taruh(m, "k", "v")
            m3 = Map.hapus(m2, "k")
            Map.ambil(m3, "k")
        rampung
    )";
    auto r3 = engine.eval(code3);
    if (r3.has_value() && (garuda::beam::is_atom(*r3, "nil") || garuda::beam::to_string(*r3) == "undefined")) {
        std::cout << "[Test 3] Map.hapus -> nil/undefined [PASS]\n";
    } else {
        std::cout << "[Test 3] Map.hapus -> [FAIL] got: " << (r3.has_value() ? garuda::beam::to_string(*r3) : "none") << "\n";
        failures++;
    }

    if (failures > 0) {
        std::cout << "\n❌ " << failures << " STDLIB MAP TESTS FAILED!\n";
        return 1;
    }

    std::cout << "\n========================================\n";
    std::cout << "FASE 15.2 - STDLIB MAP SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
