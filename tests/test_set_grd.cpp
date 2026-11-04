// Garuda Language Compiler: Set Module Test Suite (`test_set_grd.cpp`)
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
    std::cout << "GARUDA COMPILER — SET MODULE TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test S1: Set.baru() panjang == 0
    std::string code1 = R"(
        modul TestS1
        impor "std/set.grd"
        fungsi utama() ->
            Set.panjang(Set.baru())
        rampung
    )";
    auto res1 = engine.eval(code1);
    auto* val1 = res1 ? res1->get_if<std::int64_t>() : nullptr;
    CHECK(val1 && *val1 == 0, "Test S1: Set.baru() panjang == 0");

    // Test S2: Set.tambah 3x (1 duplicate) -> panjang == 2
    std::string code2 = R"(
        modul TestS2
        impor "std/set.grd"
        fungsi utama() ->
            S = Set.baru()
            S1 = Set.tambah(S, 10)
            S2 = Set.tambah(S1, 20)
            S3 = Set.tambah(S2, 10)
            Set.panjang(S3)
        rampung
    )";
    auto res2 = engine.eval(code2);
    auto* val2 = res2 ? res2->get_if<std::int64_t>() : nullptr;
    CHECK(val2 && *val2 == 2, "Test S2: Set.tambah (dedup) panjang == 2");

    // Test S3: Set.berisi -> 1 (benar) / 0 (salah)
    std::string code3 = R"(
        modul TestS3
        impor "std/set.grd"
        fungsi utama() ->
            S = Set.tambah(Set.tambah(Set.baru(), 10), 20)
            yen Set.berisi(S, 10) ->
                yen Set.berisi(S, 99) -> 0 lain -> 1 rampung
            lain -> 0
            rampung
        rampung
    )";
    auto res3 = engine.eval(code3);
    auto* val3 = res3 ? res3->get_if<std::int64_t>() : nullptr;
    CHECK(val3 && *val3 == 1, "Test S3: Set.berisi -> 1 (ada) / 0 (tidak ada)");

    // Test S4: Set.gabung -> union
    std::string code4 = R"(
        modul TestS4
        impor "std/set.grd"
        fungsi utama() ->
            S1 = Set.tambah(Set.tambah(Set.baru(), 1), 2)
            S2 = Set.tambah(Set.tambah(Set.baru(), 2), 3)
            S3 = Set.gabung(S1, S2)
            Set.panjang(S3)
        rampung
    )";
    auto res4 = engine.eval(code4);
    auto* val4 = res4 ? res4->get_if<std::int64_t>() : nullptr;
    CHECK(val4 && *val4 == 3, "Test S4: Set.gabung([1,2], [2,3]) panjang == 3");

    // Test S5: Set.irisan -> intersection
    std::string code5 = R"(
        modul TestS5
        impor "std/set.grd"
        fungsi utama() ->
            S1 = Set.tambah(Set.tambah(Set.baru(), 1), 2)
            S2 = Set.tambah(Set.tambah(Set.baru(), 2), 3)
            S3 = Set.irisan(S1, S2)
            Set.panjang(S3)
        rampung
    )";
    auto res5 = engine.eval(code5);
    auto* val5 = res5 ? res5->get_if<std::int64_t>() : nullptr;
    CHECK(val5 && *val5 == 1, "Test S5: Set.irisan([1,2], [2,3]) panjang == 1");

    std::cout << "\n🎉 ALL SET MODULE TESTS PASSED PERFECTLY!\n";
    return 0;
}
