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

    std::cout << "\n========================================\n";
    std::cout << "ALL G3 std/map.grd TESTS PASSED 100% WITH REAL CHECK MACRO!\n";
    std::cout << "========================================\n";
    return 0;
}
