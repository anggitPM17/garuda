// =====================================================================
// GARUDA COMPILER — FASE 15.4 STDLIB FILE TEST SUITE
// =====================================================================

#include "../include/garuda/beam/tools/script_engine.hpp"
#include <iostream>
#include <cassert>
#include <string>
#include <filesystem>

int main() {
    garuda::beam::ScriptEngine engine;
    int failures = 0;

    std::cout << "========================================\n";
    std::cout << "FASE 15.4 - STDLIB FILE TESTS\n";
    std::cout << "========================================\n\n";

    std::string test_file = "temp_stdlib_file_test.txt";
    std::filesystem::remove(test_file);

    // Test 1: File.tulis & File.ada
    std::string code1 = R"(
        modul TF1
        fungsi utama() ->
            File.tulis("temp_stdlib_file_test.txt", "Pesan Rahasia Garuda")
            File.ada("temp_stdlib_file_test.txt")
        rampung
    )";
    auto r1 = engine.eval(code1);
    if (r1.has_value() && (garuda::beam::is_atom(*r1, "benar") || garuda::beam::is_atom(*r1, "true") || garuda::beam::to_string(*r1) == "true")) {
        std::cout << "[Test 1] File.tulis & File.ada -> benar/true [PASS]\n";
    } else {
        std::cout << "[Test 1] File.tulis & File.ada -> [FAIL] got: " << (r1.has_value() ? garuda::beam::to_string(*r1) : "none") << "\n";
        failures++;
    }

    // Test 2: File.baca
    std::string code2 = R"(
        modul TF2
        fungsi utama() ->
            File.baca("temp_stdlib_file_test.txt")
        rampung
    )";
    auto r2 = engine.eval(code2);
    if (r2.has_value() && std::holds_alternative<std::string>(r2->value()) && std::get<std::string>(r2->value()) == "Pesan Rahasia Garuda") {
        std::cout << "[Test 2] File.baca -> \"Pesan Rahasia Garuda\" [PASS]\n";
    } else {
        std::cout << "[Test 2] File.baca -> [FAIL]\n";
        failures++;
    }

    // Test 3: File.hapus
    std::string code3 = R"(
        modul TF3
        fungsi utama() ->
            File.hapus("temp_stdlib_file_test.txt")
            File.ada("temp_stdlib_file_test.txt")
        rampung
    )";
    auto r3 = engine.eval(code3);
    if (r3.has_value() && (garuda::beam::is_atom(*r3, "salah") || garuda::beam::is_atom(*r3, "false") || garuda::beam::to_string(*r3) == "false")) {
        std::cout << "[Test 3] File.hapus & File.ada -> salah/false [PASS]\n";
    } else {
        std::cout << "[Test 3] File.hapus -> [FAIL] got: " << (r3.has_value() ? garuda::beam::to_string(*r3) : "none") << "\n";
        failures++;
    }

    std::filesystem::remove(test_file);

    if (failures > 0) {
        std::cout << "\n❌ " << failures << " STDLIB FILE TESTS FAILED!\n";
        return 1;
    }

    std::cout << "\n========================================\n";
    std::cout << "FASE 15.4 - STDLIB FILE SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
