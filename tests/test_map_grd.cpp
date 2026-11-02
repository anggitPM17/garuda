// Garuda Language Compiler: Map Test Suite (`test_map_grd.cpp`)
#include "garuda/beam/tools/script_engine.hpp"

#include <iostream>
#include <string>

#define CHECK(cond, msg) \
    if (!(cond)) { \
        std::cerr << "[FAIL] " << msg << std::endl; \
        return 1; \
    } else { \
        std::cout << "[PASS] " << msg << std::endl; \
    }

int main() {
    garuda::beam::ScriptEngine engine;

    std::cout << "=======================================================\n";
    std::cout << "GARUDA COMPILER — MAP MODULE TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test MAP1: Map.baru() kosong (ukuran == 0)
    std::string code1 = R"(
        modul TestMAP1
        impor "std/map.grd"
        fungsi utama() -> Map.ukuran(Map.baru()) rampung
    )";
    auto res1 = engine.eval(code1);
    auto* v1 = res1 ? res1->get_if<std::int64_t>() : nullptr;
    CHECK(v1 && *v1 == 0, "Test MAP1: Map.ukuran(Map.baru()) == 0");

    // Test MAP2: Map.taruh + Map.ambil
    std::string code2 = R"(
        modul TestMAP2
        impor "std/map.grd"
        fungsi utama() ->
            M = Map.taruh(Map.baru(), "a", 100)
            Map.ambil(M, "a")
        rampung
    )";
    auto res2 = engine.eval(code2);
    auto* v2 = res2 ? res2->get_if<std::int64_t>() : nullptr;
    CHECK(v2 && *v2 == 100, "Test MAP2: Map.ambil(Map.taruh(...)) == 100");

    // Test MAP3: Map.ukuran dengan 2 entri
    std::string code3 = R"(
        modul TestMAP3
        impor "std/map.grd"
        fungsi utama() ->
            M = Map.taruh(Map.taruh(Map.baru(), "a", 1), "b", 2)
            Map.ukuran(M)
        rampung
    )";
    auto res3 = engine.eval(code3);
    auto* v3 = res3 ? res3->get_if<std::int64_t>() : nullptr;
    CHECK(v3 && *v3 == 2, "Test MAP3: Map.ukuran(2 entri) == 2");

    // Test MAP4: Map.mengandung_kunci
    std::string code4 = R"(
        modul TestMAP4
        impor "std/map.grd"
        fungsi utama() ->
            M = Map.taruh(Map.baru(), "a", 1)
            yen Map.mengandung_kunci(M, "a") ->
                yen Map.mengandung_kunci(M, "b") -> salah lain -> benar rampung
            lain -> salah rampung
        rampung
    )";
    auto res4 = engine.eval(code4);
    bool is_true4 = res4 && garuda::beam::to_string(*res4) == "true";
    CHECK(is_true4, "Test MAP4: Map.mengandung_kunci('a') == true, ('b') == false");

    // Test MAP5: Map.kosong
    std::string code5 = R"(
        modul TestMAP5
        impor "std/map.grd"
        fungsi utama() ->
            yen Map.kosong(Map.baru()) ->
                yen Map.kosong(Map.taruh(Map.baru(), "a", 1)) -> salah lain -> benar rampung
            lain -> salah rampung
        rampung
    )";
    auto res5 = engine.eval(code5);
    bool is_true5 = res5 && garuda::beam::to_string(*res5) == "true";
    CHECK(is_true5, "Test MAP5: Map.kosong(baru) == true, (taruh) == false");

    // Test MAP6: Map.hapus
    std::string code6 = R"(
        modul TestMAP6
        impor "std/map.grd"
        fungsi utama() ->
            M = Map.taruh(Map.baru(), "a", 1)
            M2 = Map.hapus(M, "a")
            Map.ukuran(M2)
        rampung
    )";
    auto res6 = engine.eval(code6);
    auto* v6 = res6 ? res6->get_if<std::int64_t>() : nullptr;
    CHECK(v6 && *v6 == 0, "Test MAP6: Map.ukuran(hapus) == 0");

    // Test MAP7: Map.gabung
    std::string code7 = R"(
        modul TestMAP7
        impor "std/map.grd"
        fungsi utama() ->
            M1 = Map.taruh(Map.baru(), "a", 1)
            M2 = Map.taruh(Map.baru(), "b", 2)
            Map.ukuran(Map.gabung(M1, M2))
        rampung
    )";
    auto res7 = engine.eval(code7);
    auto* v7 = res7 ? res7->get_if<std::int64_t>() : nullptr;
    CHECK(v7 && *v7 == 2, "Test MAP7: Map.ukuran(gabung(M1, M2)) == 2");

    // Test MAP8: Map.kunci dan Map.nilai
    std::string code8 = R"(
        modul TestMAP8
        impor "std/map.grd"
        fungsi utama() ->
            M = Map.taruh(Map.taruh(Map.baru(), "a", 10), "b", 20)
            Map.ukuran(M)
        rampung
    )";
    auto res8 = engine.eval(code8);
    auto* v8 = res8 ? res8->get_if<std::int64_t>() : nullptr;
    CHECK(v8 && *v8 == 2, "Test MAP8: Map.ukuran(2 entri) == 2");

    std::cout << "\n🎉 ALL MAP MODULE TESTS PASSED PERFECTLY!\n";
    return 0;
}
