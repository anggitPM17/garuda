// =====================================================================
// GARUDA COMPILER — FASE 15.3 STDLIB STRING TEST SUITE
// =====================================================================

#include "../include/garuda/beam/tools/script_engine.hpp"
#include <iostream>
#include <cassert>
#include <string>

int main() {
    garuda::beam::ScriptEngine engine;
    int failures = 0;

    std::cout << "========================================\n";
    std::cout << "FASE 15.3 - STDLIB STRING TESTS\n";
    std::cout << "========================================\n\n";

    // Test 1: String.besar & String.kecil
    auto r1 = engine.eval("modul TS1 fungsi utama() -> String.besar(\"garuda\") rampung");
    if (r1.has_value() && std::holds_alternative<std::string>(r1->value()) && std::get<std::string>(r1->value()) == "GARUDA") {
        std::cout << "[Test 1] String.besar(\"garuda\") -> \"GARUDA\" [PASS]\n";
    } else {
        std::cout << "[Test 1] String.besar -> [FAIL]\n";
        failures++;
    }

    // Test 2: String.pangkas
    auto r2 = engine.eval("modul TS2 fungsi utama() -> String.pangkas(\"   halo garuda   \") rampung");
    if (r2.has_value() && std::holds_alternative<std::string>(r2->value()) && std::get<std::string>(r2->value()) == "halo garuda") {
        std::cout << "[Test 2] String.pangkas -> \"halo garuda\" [PASS]\n";
    } else {
        std::cout << "[Test 2] String.pangkas -> [FAIL]\n";
        failures++;
    }

    // Test 3: String.mulai_dengan
    auto r3 = engine.eval("modul TS3 fungsi utama() -> String.mulai_dengan(\"Garuda BEAM\", \"Garuda\") rampung");
    if (r3.has_value() && (garuda::beam::is_atom(*r3, "benar") || garuda::beam::is_atom(*r3, "true") || garuda::beam::to_string(*r3) == "true")) {
        std::cout << "[Test 3] String.mulai_dengan -> benar/true [PASS]\n";
    } else {
        std::cout << "[Test 3] String.mulai_dengan -> [FAIL] got: " << (r3.has_value() ? garuda::beam::to_string(*r3) : "none") << "\n";
        failures++;
    }

    // Test 4: String.ganti
    auto r4 = engine.eval("modul TS4 fungsi utama() -> String.ganti(\"halo dunia\", \"dunia\", \"Garuda\") rampung");
    if (r4.has_value() && std::holds_alternative<std::string>(r4->value()) && std::get<std::string>(r4->value()) == "halo Garuda") {
        std::cout << "[Test 4] String.ganti -> \"halo Garuda\" [PASS]\n";
    } else {
        std::cout << "[Test 4] String.ganti -> [FAIL]\n";
        failures++;
    }

    // Test 5: String.cari
    auto r5 = engine.eval("modul TS5 fungsi utama() -> String.cari(\"Garuda Compiler\", \"Compiler\") rampung");
    if (r5.has_value() && std::holds_alternative<int64_t>(r5->value()) && std::get<int64_t>(r5->value()) == 7) {
        std::cout << "[Test 5] String.cari -> 7 [PASS]\n";
    } else {
        std::cout << "[Test 5] String.cari -> [FAIL]\n";
        failures++;
    }

    if (failures > 0) {
        std::cout << "\n❌ " << failures << " STDLIB STRING TESTS FAILED!\n";
        return 1;
    }

    std::cout << "\n========================================\n";
    std::cout << "FASE 15.3 - STDLIB STRING SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
