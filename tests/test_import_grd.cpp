// =====================================================================
// GVM G4 IMPOR MODULE UNIT TEST WITH CHECK MACRO
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

    // Test 1: Import helper.grd and call function
    std::string code1 = R"(
        modul Main1
        impor "tests/import_test/helper.grd"
        fungsi utama() ->
            tambah_sepuluh(32)
        rampung
    )";
    auto res1 = engine.eval(code1);
    CHECK(res1.has_value(), "Test 1 must return value");
    auto* i1 = res1->get_if<std::int64_t>();
    CHECK(i1 != nullptr && *i1 == 42, "Test 1 result must be 42");
    std::cout << "[Test 1] impor \"tests/import_test/helper.grd\": " << *i1 << " [PASS]\n";

    // Test 2: Import non-existent file (should return nullopt safely)
    std::string code2 = R"(
        modul Main2
        impor "non_existent_file_12345.grd"
        fungsi utama() ->
            100
        rampung
    )";
    auto res2 = engine.eval(code2);
    CHECK(!res2.has_value(), "Test 2 missing file import must return nullopt safely");
    std::cout << "[Test 2] Missing file import safety check: [PASS]\n";

    // Test 3: Circular Import Safety (circular_a imports circular_b which imports circular_a)
    std::string code3 = R"(
        modul Main3
        impor "tests/import_test/circular_a.grd"
        fungsi utama() ->
            fn_a(10) + fn_b(20)
        rampung
    )";
    auto res3 = engine.eval(code3);
    CHECK(res3.has_value(), "Test 3 circular import must return value without infinite loop");
    auto* i3 = res3->get_if<std::int64_t>();
    CHECK(i3 != nullptr && *i3 == 33, "Test 3 circular import result must be 33");
    std::cout << "[Test 3] Circular Import (A <-> B) prevention: " << *i3 << " [PASS]\n";

    // Test 4: Idempotent Duplicate Import (Import same file twice)
    std::string code4 = R"(
        modul Main4
        impor "tests/import_test/helper.grd"
        impor "tests/import_test/helper.grd"
        fungsi utama() ->
            tambah_sepuluh(100)
        rampung
    )";
    auto res4 = engine.eval(code4);
    CHECK(res4.has_value(), "Test 4 duplicate import must return value");
    auto* i4 = res4->get_if<std::int64_t>();
    CHECK(i4 != nullptr && *i4 == 110, "Test 4 duplicate import result must be 110");
    std::cout << "[Test 4] Idempotent Duplicate Import: " << *i4 << " [PASS]\n";

    std::cout << "\n========================================\n";
    std::cout << "ALL 4 G4 IMPOR MODULE TESTS PASSED 100% WITH REAL CHECK MACRO!\n";
    std::cout << "========================================\n";
    return 0;
}
