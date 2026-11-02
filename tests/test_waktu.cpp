// Garuda Language Compiler: Waktu & Durasi Test Suite (`test_waktu.cpp`)
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
    std::cout << "GARUDA COMPILER — WAKTU & DURASI TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test W1: sekarang() mengembalikan integer > 0
    std::string codeW1 = R"(
        modul TestW1
        impor "std/waktu.grd"
        fungsi utama() ->
            Waktu.sekarang()
        rampung
    )";
    auto resW1 = engine.eval(codeW1);
    auto* valW1 = resW1 ? resW1->get_if<std::int64_t>() : nullptr;
    CHECK(valW1 && *valW1 > 0, "Test W1: Waktu.sekarang() > 0");

    // Test W2: tunggu(50) mengembalikan nil
    std::string codeW2 = R"(
        modul TestW2
        impor "std/waktu.grd"
        fungsi utama() ->
            Waktu.tunggu(50)
        rampung
    )";
    auto resW2 = engine.eval(codeW2);
    bool is_nilW2 = resW2 && garuda::beam::to_string(*resW2) == "nil";
    CHECK(is_nilW2, "Test W2: Waktu.tunggu(50) == nil");

    // Test W3: durasi_mulai dan durasi_selesai bekerja
    std::string codeW3 = R"(
        modul TestW3
        impor "std/waktu.grd"
        fungsi utama() ->
            Start = Waktu.durasi_mulai()
            Waktu.tunggu(50)
            Waktu.durasi_selesai(Start)
        rampung
    )";
    auto resW3 = engine.eval(codeW3);
    auto* valW3 = resW3 ? resW3->get_if<std::int64_t>() : nullptr;
    CHECK(valW3 && *valW3 >= 30, "Test W3: Waktu.durasi_selesai(Start) >= 30");

    // Test W4: dua panggilan sekarang() berurutan, b >= a
    std::string codeW4 = R"(
        modul TestW4
        impor "std/waktu.grd"
        fungsi utama() ->
            A = Waktu.sekarang()
            B = Waktu.sekarang()
            yen B >= A -> benar lain -> salah rampung
        rampung
    )";
    auto resW4 = engine.eval(codeW4);
    bool is_trueW4 = resW4 && garuda::beam::to_string(*resW4) == "true";
    CHECK(is_trueW4, "Test W4: B >= A untuk Waktu.sekarang()");

    std::cout << "\n🎉 ALL WAKTU & DURASI TESTS PASSED PERFECTLY!\n";
    return 0;
}
