// =====================================================================
// GARUDA LANGUAGE (.grd) STRING MODULE UNIT TEST WITH REAL ASSERTS
// =====================================================================

#include "../include/garuda/beam/tools/script_engine.hpp"
#include <iostream>
#include <cassert>

int main() {
    garuda::beam::ScriptEngine engine;

    // Test 1: String Concat ++
    std::string code1 = "modul Test1\nfungsi utama() -> \"Garuda \" ++ \"BEAM\"\nrampung";
    auto res1 = engine.eval(code1);
    assert(res1.has_value());
    if (auto* s = res1->get_if<std::string>()) {
        std::cout << "[Test 1] Concat ++ Result: '" << *s << "'\n";
        assert(*s == "Garuda BEAM");
    } else {
        assert(false && "Test 1 must return string");
    }

    // Test 2: Native Panjang (Length)
    std::string code2 = "modul Test2\nfungsi utama() -> panjang(\"Garuda\")\nrampung";
    auto res2 = engine.eval(code2);
    assert(res2.has_value());
    if (auto* i = res2->get_if<std::int64_t>()) {
        std::cout << "[Test 2] Length Result: " << *i << "\n";
        assert(*i == 6);
    } else {
        assert(false && "Test 2 must return int 6");
    }

    // Test 3: Native Besar (Uppercase)
    std::string code3 = "modul Test3\nfungsi utama() -> besar(\"garuda\")\nrampung";
    auto res3 = engine.eval(code3);
    assert(res3.has_value());
    if (auto* s = res3->get_if<std::string>()) {
        std::cout << "[Test 3] Uppercase Result: '" << *s << "'\n";
        assert(*s == "GARUDA");
    } else {
        assert(false && "Test 3 must return 'GARUDA'");
    }

    // Test 4: Native Kecil (Lowercase)
    std::string code4 = "modul Test4\nfungsi utama() -> kecil(\"GARUDA\")\nrampung";
    auto res4 = engine.eval(code4);
    assert(res4.has_value());
    if (auto* s = res4->get_if<std::string>()) {
        std::cout << "[Test 4] Lowercase Result: '" << *s << "'\n";
        assert(*s == "garuda");
    } else {
        assert(false && "Test 4 must return 'garuda'");
    }

    // Test 5: Native Potong (Substring)
    std::string code5 = "modul Test5\nfungsi utama() -> potong(\"Garuda\", 0, 6)\nrampung";
    auto res5 = engine.eval(code5);
    assert(res5.has_value());
    if (auto* s = res5->get_if<std::string>()) {
        std::cout << "[Test 5] Substring Result: '" << *s << "'\n";
        assert(*s == "Garuda");
    } else {
        assert(false && "Test 5 must return 'Garuda'");
    }

    std::cout << "\n========================================\n";
    std::cout << "ALL G1 std/string.grd TESTS PASSED 100% WITH REAL ASSERTS!\n";
    std::cout << "========================================\n";
    return 0;
}
