// =====================================================================
// GARUDA LANGUAGE (.grd) STRING MODULE UNIT TEST WITH REAL CHECK MACRO
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

    // Test 1: String Concat ++
    std::string code1 = "modul Test1\nfungsi utama() -> \"Garuda \" ++ \"BEAM\"\nrampung";
    auto res1 = engine.eval(code1);
    CHECK(res1.has_value(), "Test 1 must return value");
    auto* s1 = res1->get_if<std::string>();
    CHECK(s1 != nullptr && *s1 == "Garuda BEAM", "Test 1 result must be 'Garuda BEAM'");
    std::cout << "[Test 1] Concat ++ Result: '" << *s1 << "' [PASS]\n";

    // Test 2: Native Panjang (Length)
    std::string code2 = "modul Test2\nfungsi utama() -> panjang(\"Garuda\")\nrampung";
    auto res2 = engine.eval(code2);
    CHECK(res2.has_value(), "Test 2 must return value");
    auto* i2 = res2->get_if<std::int64_t>();
    CHECK(i2 != nullptr && *i2 == 6, "Test 2 result must be 6");
    std::cout << "[Test 2] Length Result: " << *i2 << " [PASS]\n";

    // Test 3: Native Besar (Uppercase)
    std::string code3 = "modul Test3\nfungsi utama() -> besar(\"garuda\")\nrampung";
    auto res3 = engine.eval(code3);
    CHECK(res3.has_value(), "Test 3 must return value");
    auto* s3 = res3->get_if<std::string>();
    CHECK(s3 != nullptr && *s3 == "GARUDA", "Test 3 result must be 'GARUDA'");
    std::cout << "[Test 3] Uppercase Result: '" << *s3 << "' [PASS]\n";

    // Test 4: Native Kecil (Lowercase)
    std::string code4 = "modul Test4\nfungsi utama() -> kecil(\"GARUDA\")\nrampung";
    auto res4 = engine.eval(code4);
    CHECK(res4.has_value(), "Test 4 must return value");
    auto* s4 = res4->get_if<std::string>();
    CHECK(s4 != nullptr && *s4 == "garuda", "Test 4 result must be 'garuda'");
    std::cout << "[Test 4] Lowercase Result: '" << *s4 << "' [PASS]\n";

    // Test 5: Native Potong (Substring)
    std::string code5 = "modul Test5\nfungsi utama() -> potong(\"Garuda\", 0, 6)\nrampung";
    auto res5 = engine.eval(code5);
    CHECK(res5.has_value(), "Test 5 must return value");
    auto* s5 = res5->get_if<std::string>();
    CHECK(s5 != nullptr && *s5 == "Garuda", "Test 5 result must be 'Garuda'");
    std::cout << "[Test 5] Substring Result: '" << *s5 << "' [PASS]\n";

    // Test 6: Edge Cases
    std::string code6 = "modul Test6\nfungsi utama() -> besar(\"Halo123\")\nrampung";
    auto res6 = engine.eval(code6);
    CHECK(res6.has_value(), "Test 6 must return value");
    auto* s6 = res6->get_if<std::string>();
    CHECK(s6 != nullptr && *s6 == "HALO123", "Test 6 result must be 'HALO123'");
    std::cout << "[Test 6] Edge Case Uppercase Result: '" << *s6 << "' [PASS]\n";

    // Test 7: String.pisah("a,b,c", ",") -> length 3
    std::string code7 = "modul Test7\nfungsi utama() -> List.panjang(String.pisah(\"a,b,c\", \",\"))\nrampung";
    auto res7 = engine.eval(code7);
    CHECK(res7.has_value(), "Test 7 must return value");
    auto* i7 = res7->get_if<std::int64_t>();
    CHECK(i7 != nullptr && *i7 == 3, "Test 7 pisah length must be 3");
    std::cout << "[Test 7] String.pisah(\"a,b,c\", \",\") length: " << *i7 << " [PASS]\n";

    // Test 8: String.mengandung("Garuda", "rud") -> true / benar
    std::string code8 = "modul Test8\nfungsi utama() -> String.mengandung(\"Garuda\", \"rud\")\nrampung";
    auto res8 = engine.eval(code8);
    CHECK(res8.has_value(), "Test 8 must return value");
    auto* a8 = res8->get_if<garuda::beam::Atom>();
    CHECK(a8 != nullptr && (a8->name == "true" || a8->name == "benar"), "Test 8 mengandung must be true");
    std::cout << "[Test 8] String.mengandung(\"Garuda\", \"rud\"): true [PASS]\n";

    // Test 9: String.rapikan("  halo  ") -> "halo"
    std::string code9 = "modul Test9\nfungsi utama() -> String.rapikan(\"  halo  \")\nrampung";
    auto res9 = engine.eval(code9);
    CHECK(res9.has_value(), "Test 9 must return value");
    auto* s9 = res9->get_if<std::string>();
    CHECK(s9 != nullptr && *s9 == "halo", "Test 9 rapikan must be 'halo'");
    std::cout << "[Test 9] String.rapikan(\"  halo  \"): '" << *s9 << "' [PASS]\n";

    // Test 10: karakter_ke_int("a") == 97
    std::string code10 = "modul Test10\nfungsi utama() -> karakter_ke_int(\"a\")\nrampung";
    auto res10 = engine.eval(code10);
    CHECK(res10.has_value(), "Test 10 must return value");
    auto* i10 = res10->get_if<std::int64_t>();
    CHECK(i10 != nullptr && *i10 == 97, "Test 10 karakter_ke_int must be 97");
    std::cout << "[Test 10] karakter_ke_int(\"a\"): " << *i10 << " [PASS]\n";

    // Test 11: int_ke_karakter(97) == "a"
    std::string code11 = "modul Test11\nfungsi utama() -> int_ke_karakter(97)\nrampung";
    auto res11 = engine.eval(code11);
    CHECK(res11.has_value(), "Test 11 must return value");
    auto* s11 = res11->get_if<std::string>();
    CHECK(s11 != nullptr && *s11 == "a", "Test 11 int_ke_karakter must be 'a'");
    std::cout << "[Test 11] int_ke_karakter(97): '" << *s11 << "' [PASS]\n";

    std::cout << "\n========================================\n";
    std::cout << "ALL 11 G1 & G7 std/string.grd TESTS PASSED 100% WITH REAL CHECK MACRO!\n";
    std::cout << "========================================\n";
    return 0;
}
