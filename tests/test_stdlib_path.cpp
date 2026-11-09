// =====================================================================
// GARUDA COMPILER — FASE 15.7 STDLIB PATH TEST SUITE
// =====================================================================

#include "../include/garuda/beam/tools/script_engine.hpp"
#include <iostream>
#include <cassert>
#include <string>

int main() {
    garuda::beam::ScriptEngine engine;
    int failures = 0;

    std::cout << "========================================\n";
    std::cout << "FASE 15.7 - STDLIB PATH TESTS\n";
    std::cout << "========================================\n\n";

    // Test 1: Path.gabung (join)
    auto r1 = engine.eval("modul TP1 fungsi utama() -> Path.gabung(\"folder\", \"file.grd\") rampung");
    if (r1.has_value() && std::holds_alternative<std::string>(r1->value()) && !std::get<std::string>(r1->value()).empty()) {
        std::cout << "[Test 1] Path.gabung(\"folder\", \"file.grd\") -> \"" << std::get<std::string>(r1->value()) << "\" [PASS]\n";
    } else {
        std::cout << "[Test 1] Path.gabung -> [FAIL]\n";
        failures++;
    }

    // Test 2: Path.nama_file (filename)
    auto r2 = engine.eval("modul TP2 fungsi utama() -> Path.nama_file(\"src/utama.grd\") rampung");
    if (r2.has_value() && std::holds_alternative<std::string>(r2->value()) && std::get<std::string>(r2->value()) == "utama.grd") {
        std::cout << "[Test 2] Path.nama_file(\"src/utama.grd\") -> \"utama.grd\" [PASS]\n";
    } else {
        std::cout << "[Test 2] Path.nama_file -> [FAIL]\n";
        failures++;
    }

    // Test 3: Path.ekstensi (extension)
    auto r3 = engine.eval("modul TP3 fungsi utama() -> Path.ekstensi(\"utama.grd\") rampung");
    if (r3.has_value() && std::holds_alternative<std::string>(r3->value()) && std::get<std::string>(r3->value()) == ".grd") {
        std::cout << "[Test 3] Path.ekstensi(\"utama.grd\") -> \".grd\" [PASS]\n";
    } else {
        std::cout << "[Test 3] Path.ekstensi -> [FAIL]\n";
        failures++;
    }

    if (failures > 0) {
        std::cout << "\n❌ " << failures << " STDLIB PATH TESTS FAILED!\n";
        return 1;
    }

    std::cout << "\n========================================\n";
    std::cout << "FASE 15.7 - STDLIB PATH SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
