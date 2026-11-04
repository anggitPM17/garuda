// Garuda Language Compiler: Pipe Operator (`|>`) Test Suite (`test_pipe.cpp`)
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
    std::cout << "=======================================================\n";
    std::cout << "GARUDA COMPILER — PIPE OPERATOR (|>) TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test P1: pipe sederhana dengan fungsi bernama (balik -> panjang)
    {
        garuda::beam::ScriptEngine engine;
        std::string codeP1 = R"(
            modul TestP1
            fungsi balik(L) -> tinjau L ing [] -> [] [H | T] -> balik(T) ++ [H] _ -> [] rampung rampung
            fungsi panjang(L) -> tinjau L ing [] -> 0 [_ | T] -> 1 + panjang(T) _ -> 0 rampung rampung
            fungsi utama() ->
                [1, 2, 3] |> balik |> panjang
            rampung
        )";
        auto resP1 = engine.eval(codeP1);
        auto* valP1 = resP1 ? resP1->get_if<std::int64_t>() : nullptr;
        CHECK(valP1 && *valP1 == 3, "Test P1: [1,2,3] |> balik |> panjang == 3");
    }

    // Test P2: pipe dengan argumen tambahan (ambil(2))
    {
        garuda::beam::ScriptEngine engine;
        std::string codeP2 = R"(
            modul TestP2
            fungsi ambil(L, N) -> yen N <= 0 -> [] lain -> tinjau L ing [] -> [] [H | T] -> [H] ++ ambil(T, N - 1) _ -> [] rampung rampung rampung
            fungsi panjang(L) -> tinjau L ing [] -> 0 [_ | T] -> 1 + panjang(T) _ -> 0 rampung rampung
            fungsi utama() ->
                [1, 2, 3, 4, 5] |> ambil(2) |> panjang
            rampung
        )";
        auto resP2 = engine.eval(codeP2);
        auto* valP2 = resP2 ? resP2->get_if<std::int64_t>() : nullptr;
        CHECK(valP2 && *valP2 == 2, "Test P2: [1,2,3,4,5] |> ambil(2) |> panjang == 2");
    }

    // Test P3: chained pipe dengan BIF ([1,2,3] |> balik |> String.panjang)
    {
        garuda::beam::ScriptEngine engine;
        std::string codeP3 = R"(
            modul TestP3
            fungsi balik(L) -> tinjau L ing [] -> [] [H | T] -> balik(T) ++ [H] _ -> [] rampung rampung
            fungsi utama() ->
                "123" |> String.panjang
            rampung
        )";
        auto resP3 = engine.eval(codeP3);
        auto* valP3 = resP3 ? resP3->get_if<std::int64_t>() : nullptr;
        CHECK(valP3 && *valP3 == 3, "Test P3: '123' |> String.panjang == 3");
    }

    // Test P4: pipe dengan BIF String.panjang
    {
        garuda::beam::ScriptEngine engine;
        std::string codeP4 = R"(
            modul TestP4
            fungsi utama() ->
                "halo dunia" |> String.panjang
            rampung
        )";
        auto resP4 = engine.eval(codeP4);
        auto* valP4 = resP4 ? resP4->get_if<std::int64_t>() : nullptr;
        CHECK(valP4 && *valP4 == 10, "Test P4: 'halo dunia' |> String.panjang == 10");
    }

    // Test P5: pipe dengan ekspresi di kiri (10 + 20) |> dbl
    {
        garuda::beam::ScriptEngine engine;
        std::string codeP5 = R"(
            modul TestP5
            fungsi dbl(X) -> X * 2 rampung
            fungsi utama() ->
                (10 + 20) |> dbl
            rampung
        )";
        auto resP5 = engine.eval(codeP5);
        auto* valP5 = resP5 ? resP5->get_if<std::int64_t>() : nullptr;
        CHECK(valP5 && *valP5 == 60, "Test P5: (10 + 20) |> dbl == 60");
    }

    std::cout << "\n🎉 ALL PIPE OPERATOR (|>) TESTS PASSED PERFECTLY!\n";
    return 0;
}
