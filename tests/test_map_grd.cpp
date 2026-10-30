// =====================================================================
// GVM G3 std/map.grd UNIT TEST WITH CHECK MACRO
// =====================================================================

#include "../include/garuda/beam/tools/script_engine.hpp"
#include <iostream>
#include <string>
#include <cstdlib>

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        std::cerr << "[FAIL] " << msg << "\n"; \
        std::exit(1); \
    } \
} while(0)

int main() {
    garuda::beam::ScriptEngine engine;

    // Test 1: Map.baru()
    std::string code1 = R"(
        modul Test1
        fungsi utama() ->
            Map.baru()
        rampung
    )";
    auto res1 = engine.eval(code1);
    CHECK(res1.has_value(), "Test 1 must return value");
    std::cout << "[Test 1] Map.baru(): OK [PASS]\n";

    // Test 2: Map.taruh() and Map.ambil()
    std::string code2 = R"(
        modul Test2
        fungsi utama() ->
            m = Map.baru()
            m2 = Map.taruh(m, "nama", "Garuda")
            Map.ambil(m2, "nama")
        rampung
    )";
    auto res2 = engine.eval(code2);
    CHECK(res2.has_value(), "Test 2 must return value");
    auto* s2 = res2->get_if<std::string>();
    CHECK(s2 != nullptr && *s2 == "Garuda", "Test 2 Map.ambil must return 'Garuda'");
    std::cout << "[Test 2] Map.taruh & Map.ambil ('nama' -> 'Garuda'): '" << *s2 << "' [PASS]\n";

    // Test 3: Map.hapus()
    std::string code3 = R"(
        modul Test3
        fungsi utama() ->
            m = Map.baru()
            m2 = Map.taruh(m, "nama", "Garuda")
            m3 = Map.hapus(m2, "nama")
            Map.ambil(m3, "nama")
        rampung
    )";
    auto res3 = engine.eval(code3);
    CHECK(res3.has_value(), "Test 3 must return value");
    CHECK(res3->get_if<std::string>() == nullptr || res3->get_if<std::string>()->empty(), "Test 3 Map.hapus must remove key");
    std::cout << "[Test 3] Map.hapus ('nama'): Key Removed [PASS]\n";

    std::cout << "\n========================================\n";
    std::cout << "ALL 3 G3 std/map.grd TESTS PASSED 100% WITH REAL CHECK MACRO!\n";
    std::cout << "========================================\n";
    return 0;
}
