// Garuda Language Compiler: Queue Module Test Suite (`test_queue_grd.cpp`)
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
    std::cout << "GARUDA COMPILER — QUEUE MODULE TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test Q1: Queue.baru() kosong == 1
    std::string code1 = R"(
        modul TestQ1
        impor "std/queue.grd"
        fungsi utama() ->
            Queue.kosong(Queue.baru())
        rampung
    )";
    auto res1 = engine.eval(code1);
    auto* val1 = res1 ? res1->get_if<std::int64_t>() : nullptr;
    CHECK(val1 && *val1 == 1, "Test Q1: Queue.baru() kosong == 1");

    // Test Q2: masuk 3x -> panjang == 3
    std::string code2 = R"(
        modul TestQ2
        impor "std/queue.grd"
        fungsi utama() ->
            Q = Queue.baru()
            Q1 = Queue.masuk(Q, 10)
            Q2 = Queue.masuk(Q1, 20)
            Q3 = Queue.masuk(Q2, 30)
            Queue.panjang(Q3)
        rampung
    )";
    auto res2 = engine.eval(code2);
    auto* val2 = res2 ? res2->get_if<std::int64_t>() : nullptr;
    CHECK(val2 && *val2 == 3, "Test Q2: masuk 3x -> panjang == 3");

    // Test Q3: keluar -> FIFO
    std::string code3 = R"(
        modul TestQ3
        impor "std/queue.grd"
        fungsi utama() ->
            Q = Queue.masuk(Queue.masuk(Queue.baru(), 100), 200)
            tinjau Queue.keluar(Q) ing
                {Elemen, Qsisa} -> Elemen
                _ -> 0
            rampung
        rampung
    )";
    auto res3 = engine.eval(code3);
    auto* val3 = res3 ? res3->get_if<std::int64_t>() : nullptr;
    CHECK(val3 && *val3 == 100, "Test Q3: keluar -> FIFO == 100");

    // Test Q4: intip tanpa hapus
    std::string code4 = R"(
        modul TestQ4
        impor "std/queue.grd"
        fungsi utama() ->
            Q = Queue.masuk(Queue.masuk(Queue.baru(), 50), 60)
            V = Queue.intip(Q)
            P = Queue.panjang(Q)
            yen V == 50 ->
                yen P == 2 -> 1 lain -> 0 rampung
            lain -> 0
            rampung
        rampung
    )";
    auto res4 = engine.eval(code4);
    auto* val4 = res4 ? res4->get_if<std::int64_t>() : nullptr;
    CHECK(val4 && *val4 == 1, "Test Q4: intip(Q) == 50 dan panjang tetap 2");

    std::cout << "\n🎉 ALL QUEUE MODULE TESTS PASSED PERFECTLY!\n";
    return 0;
}
