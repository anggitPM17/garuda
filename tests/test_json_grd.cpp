// =====================================================================
// GVM G5 & G6 std/json.grd UNIT TEST WITH CHECK MACRO
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

    // Test 1: Json.encode_int(42)
    std::string code1 = R"(
        modul Test1
        impor "std/json.grd"
        fungsi utama() ->
            Json.encode_int(42)
        rampung
    )";
    auto res1 = engine.eval(code1);
    CHECK(res1.has_value(), "Test 1 must return value");
    auto* s1 = res1->get_if<std::string>();
    CHECK(s1 != nullptr && *s1 == "42", "Test 1 encode_int must be '42'");
    std::cout << "[Test 1] Json.encode_int(42): '" << *s1 << "' [PASS]\n";

    // Test 2: Json.encode_string("halo")
    std::string code2 = R"(
        modul Test2
        impor "std/json.grd"
        fungsi utama() ->
            Json.encode_string("halo")
        rampung
    )";
    auto res2 = engine.eval(code2);
    CHECK(res2.has_value(), "Test 2 must return value");
    auto* s2 = res2->get_if<std::string>();
    CHECK(s2 != nullptr && *s2 == "\"halo\"", "Test 2 encode_string must be '\"halo\"'");
    std::cout << "[Test 2] Json.encode_string(\"halo\"): '" << *s2 << "' [PASS]\n";

    // Test 3: Json.encode_bool(true/benar)
    std::string code3 = R"(
        modul Test3
        impor "std/json.grd"
        fungsi utama() ->
            Json.encode_bool(benar)
        rampung
    )";
    auto res3 = engine.eval(code3);
    CHECK(res3.has_value(), "Test 3 must return value");
    auto* s3 = res3->get_if<std::string>();
    CHECK(s3 != nullptr && *s3 == "true", "Test 3 encode_bool must be 'true'");
    std::cout << "[Test 3] Json.encode_bool(benar): '" << *s3 << "' [PASS]\n";

    // Test 4: Json.encode_list([1, 2, 3])
    std::string code4 = R"(
        modul Test4
        impor "std/json.grd"
        fungsi utama() ->
            Json.encode_list([1, 2, 3])
        rampung
    )";
    auto res4 = engine.eval(code4);
    CHECK(res4.has_value(), "Test 4 must return value");
    auto* s4 = res4->get_if<std::string>();
    CHECK(s4 != nullptr && *s4 == "[1,2,3]", "Test 4 encode_list must be '[1,2,3]'");
    std::cout << "[Test 4] Json.encode_list([1, 2, 3]): '" << *s4 << "' [PASS]\n";

    // Test 5a: Json.encode_map(Map.baru()) == "{}"
    std::string code5a = R"(
        modul Test5a
        impor "std/json.grd"
        fungsi utama() ->
            Json.encode_map(Map.baru())
        rampung
    )";
    auto res5a = engine.eval(code5a);
    CHECK(res5a.has_value(), "Test 5a must return value");
    auto* s5a = res5a->get_if<std::string>();
    CHECK(s5a != nullptr && *s5a == "{}", "Test 5a encode_map empty must be '{}'");
    std::cout << "[Test 5a] Json.encode_map(Map.baru()): '" << *s5a << "' [PASS]\n";

    // Test 5b: Json.encode_map with single entry {"a": 1}
    std::string code5b = R"(
        modul Test5b
        impor "std/json.grd"
        fungsi utama() ->
            m = Map.baru()
            m1 = Map.taruh(m, "a", 1)
            Json.encode_map(m1)
        rampung
    )";
    auto res5b = engine.eval(code5b);
    CHECK(res5b.has_value(), "Test 5b must return value");
    auto* s5b = res5b->get_if<std::string>();
    CHECK(s5b != nullptr && *s5b == "{\"a\":1}", "Test 5b encode_map single entry must be '{\"a\":1}'");
    std::cout << "[Test 5b] Json.encode_map single entry: '" << *s5b << "' [PASS]\n";

    // Test 5c: Json.encode_map with multiple entries {"a": 10, "b": 20}
    std::string code5c = R"(
        modul Test5c
        impor "std/json.grd"
        fungsi utama() ->
            m = Map.baru()
            m1 = Map.taruh(m, "a", 10)
            m2 = Map.taruh(m1, "b", 20)
            Json.encode_map(m2)
        rampung
    )";
    auto res5c = engine.eval(code5c);
    CHECK(res5c.has_value(), "Test 5c must return value");
    auto* s5c = res5c->get_if<std::string>();
    CHECK(s5c != nullptr && (s5c->find("\"a\":10") != std::string::npos) && (s5c->find("\"b\":20") != std::string::npos), "Test 5c encode_map multi entry must contain '\"a\":10' and '\"b\":20'");
    std::cout << "[Test 5c] Json.encode_map multi entry: '" << *s5c << "' [PASS]\n";

    std::cout << "\n========================================\n";
    std::cout << "ALL G5 & G6 std/json.grd TESTS PASSED 100% WITH REAL CHECK MACRO!\n";
    std::cout << "========================================\n";
    return 0;
}
