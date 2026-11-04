// Garuda Language Compiler: Stack Module Test Suite (`test_stack_grd.cpp`)
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
    std::cout << "GARUDA COMPILER — STACK MODULE TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test ST1: Stack.baru() kosong == 1
    std::string code1 = R"(
        modul TestST1
        impor "std/stack.grd"
        fungsi utama() ->
            Stack.kosong(Stack.baru())
        rampung
    )";
    auto res1 = engine.eval(code1);
    auto* val1 = res1 ? res1->get_if<std::int64_t>() : nullptr;
    CHECK(val1 && *val1 == 1, "Test ST1: Stack.baru() kosong == 1");

    // Test ST2: dorong 3x -> panjang == 3
    std::string code2 = R"(
        modul TestST2
        impor "std/stack.grd"
        fungsi utama() ->
            S = Stack.baru()
            S1 = Stack.dorong(S, 10)
            S2 = Stack.dorong(S1, 20)
            S3 = Stack.dorong(S2, 30)
            Stack.panjang(S3)
        rampung
    )";
    auto res2 = engine.eval(code2);
    auto* val2 = res2 ? res2->get_if<std::int64_t>() : nullptr;
    CHECK(val2 && *val2 == 3, "Test ST2: dorong 3x -> panjang == 3");

    // Test ST3: pop -> LIFO
    std::string code3 = R"(
        modul TestST3
        impor "std/stack.grd"
        fungsi utama() ->
            S = Stack.dorong(Stack.dorong(Stack.baru(), 100), 200)
            tinjau Stack.pop(S) ing
                {Elemen, Ssisa} -> Elemen
                _ -> 0
            rampung
        rampung
    )";
    auto res3 = engine.eval(code3);
    auto* val3 = res3 ? res3->get_if<std::int64_t>() : nullptr;
    CHECK(val3 && *val3 == 200, "Test ST3: pop -> LIFO == 200");

    // Test ST4: intip tanpa hapus
    std::string code4 = R"(
        modul TestST4
        impor "std/stack.grd"
        fungsi utama() ->
            S = Stack.dorong(Stack.dorong(Stack.baru(), 50), 60)
            V = Stack.intip(S)
            P = Stack.panjang(S)
            yen V == 60 ->
                yen P == 2 -> 1 lain -> 0 rampung
            lain -> 0
            rampung
        rampung
    )";
    auto res4 = engine.eval(code4);
    auto* val4 = res4 ? res4->get_if<std::int64_t>() : nullptr;
    CHECK(val4 && *val4 == 1, "Test ST4: intip(S) == 60 dan panjang tetap 2");

    std::cout << "\n🎉 ALL STACK MODULE TESTS PASSED PERFECTLY!\n";
    return 0;
}
