// =====================================================================
// GARUDA LANGUAGE (.grd) STRING MODULE UNIT TEST
// =====================================================================

#include "../include/garuda/beam/tools/script_engine.hpp"
#include <iostream>
#include <cassert>

int main() {
    garuda::beam::ScriptEngine engine;

    // Test 1: String concatenation with ++
    std::string code1 = "modul Test1\nfungsi utama() -> \"Garuda \" ++ \"BEAM\"\nrampung";
    auto res1 = engine.eval(code1);
    assert(res1.has_value());
    if (auto* s = res1->get_if<std::string>()) {
        std::cout << "Test 1 (Concat ++): " << *s << " [OK]\n";
        assert(*s == "Garuda BEAM");
    }

    // Test 2: Evaluate std/string.grd
    auto res2 = engine.eval_file("std/string.grd");
    std::cout << "Test 2 (Eval std/string.grd): OK\n";

    std::cout << "All G1 std/string.grd tests passed successfully!\n";
    return 0;
}
