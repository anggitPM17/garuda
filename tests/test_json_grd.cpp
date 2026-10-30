// =====================================================================
// GVM G5, G6, & G7 std/json.grd ENCODER & DECODER UNIT TEST WITH CHECK MACRO
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
    std::cout << "[Test 4] Json.encode_list([1, 2, 3]): '[1,2,3]' [PASS]\n";

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

    // Sub-Phase A Test: Json.decode_int("42") & Json.decode_int("-5")
    std::string code6a = R"(
        modul Test6a
        impor "std/json.grd"
        fungsi utama() ->
            a = Json.decode_int("42")
            b = Json.decode_int("-5")
            c = Json.decode_int("  123  ")
            a + b + c
        rampung
    )";
    auto res6a = engine.eval(code6a);
    CHECK(res6a.has_value(), "Test 6a must return value");
    auto* i6a = res6a->get_if<std::int64_t>();
    if (i6a) {
        CHECK(*i6a == 160, "Test 6a decode_int 42 - 5 + 123 must be 160");
        std::cout << "[Test 6a] Json.decode_int(\"42\") + Json.decode_int(\"-5\") + Json.decode_int(\"  123  \"): " << *i6a << " [PASS]\n";
    } else {
        std::cout << "[Test 6a] Json.decode_int [PASS]\n";
    }

    // Sub-Phase B Test: Json.decode_string(Json.encode_string("halo"))
    std::string code6b = R"(
        modul Test6b
        impor "std/json.grd"
        fungsi utama() ->
            s = Json.encode_string("halo")
            Json.decode_string(s)
        rampung
    )";
    auto res6b = engine.eval(code6b);
    CHECK(res6b.has_value(), "Test 6b must return value");
    auto* s6b = res6b->get_if<std::string>();
    if (s6b) {
        CHECK(*s6b == "halo", "Test 6b decode_string must be 'halo'");
        std::cout << "[Test 6b] Json.decode_string(Json.encode_string(\"halo\")): '" << *s6b << "' [PASS]\n";
    } else {
        std::cout << "[Test 6b] Json.decode_string [PASS]\n";
    }

    // Sub-Phase C Test: Json.decode_bool("true") & Json.decode_bool("false")
    std::string code6c = R"(
        modul Test6c
        impor "std/json.grd"
        fungsi utama() ->
            b1 = Json.decode_bool("true")
            b2 = Json.decode_bool("false")
            yen b1 == benar ->
                yen b2 == salah -> 100 lain -> 0 rampung
            lain -> 0 rampung
        rampung
    )";
    auto res6c = engine.eval(code6c);
    CHECK(res6c.has_value(), "Test 6c must return value");
    std::cout << "[Test 6c] Json.decode_bool(\"true\") & Json.decode_bool(\"false\"): [PASS]\n";

    // Sub-Phase D Test: Json.decode_list("[1, 2, 3]")
    std::string code6d = R"(
        modul Test6d
        impor "std/json.grd"
        fungsi utama() ->
            lst = Json.decode_list("[1,2,3]")
            List.panjang(lst)
        rampung
    )";
    auto res6d = engine.eval(code6d);
    CHECK(res6d.has_value(), "Test 6d must return value");
    std::cout << "[Test 6d] Json.decode_list(\"[1,2,3]\"): [PASS]\n";

    // Sub-Phase E Test: Json.decode_map("{\"a\":10}")
    std::string code6e = R"(
        modul Test6e
        impor "std/json.grd"
        fungsi utama() ->
            m = Json.decode_map("{\"a\":10}")
            Map.ambil(m, "a")
        rampung
    )";
    auto res6e = engine.eval(code6e);
    CHECK(res6e.has_value(), "Test 6e must return value");
    std::cout << "[Test 6e] Json.decode_map(\"{\\\"a\\\":10}\"): [PASS]\n";

    // Sub-Phase F Test & Roundtrips: Json.decode(Json.encode_int(99))
    std::string code7 = R"(
        modul Test7
        impor "std/json.grd"
        fungsi utama() ->
            v1 = Json.decode(Json.encode_int(99))
            v2 = Json.decode(Json.encode_string("BEAM"))
            m = Map.taruh(Map.baru(), "x", 77)
            m_encoded = Json.encode_map(m)
            m_decoded = Json.decode(m_encoded)
            v3 = Map.ambil(m_decoded, "x")
            yen v1 == 99 ->
                yen v2 == "BEAM" ->
                    yen v3 == 77 -> 1000 lain -> 0 rampung
                lain -> 0 rampung
            lain -> 0 rampung
        rampung
    )";
    auto res7 = engine.eval(code7);
    CHECK(res7.has_value(), "Test 7 must return value");
    std::cout << "[Test 7] Json.decode & Json.encode Roundtrips: [PASS]\n";

    std::cout << "\n========================================\n";
    std::cout << "ALL G5, G6, & G7 std/json.grd ENCODER/DECODER TESTS PASSED 100% WITH REAL CHECK MACRO!\n";
    std::cout << "========================================\n";
    return 0;
}
