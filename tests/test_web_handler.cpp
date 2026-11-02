// Garuda Language Compiler: Web Handler Test Suite (`test_web_handler.cpp`)
#include "garuda/beam/tools/script_engine.hpp"
#include <httplib.h>

#include <iostream>
#include <string>
#include <thread>

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
    std::cout << "GARUDA COMPILER — WEB GARUDA HANDLER TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test WH1: daftar_handler mengembalikan :ok
    std::string code1 = R"(
        modul TestWH1
        impor "std/web.grd"
        fungsi utama() ->
            Web.daftar_handler("h_halo", "\"Halo dari Handler Garuda!\"")
        rampung
    )";
    auto res1 = engine.eval(code1);
    bool is_ok1 = res1 && garuda::beam::to_string(*res1) == "ok";
    CHECK(is_ok1, "Test WH1: Web.daftar_handler == :ok");

    // Test WH2 & WH3 & WH4: Daftarkan handler + route, jalankan server di port 8092
    std::string codeRun = R"(
        modul TestWHRun
        impor "std/web.grd"
        fungsi utama() ->
            Web.daftar_handler("h_calc", "40 + 2")
            Web.daftar_handler("h_a", "\"AAA\"")
            Web.daftar_handler("h_b", "\"BBB\"")
            Web.get("/halo", "h_halo")
            Web.get("/calc", "h_calc")
            Web.get("/a", "h_a")
            Web.get("/b", "h_b")
            Web.jalankan(8092)
        rampung
    )";
    auto resRun = engine.eval(codeRun);
    bool is_okRun = resRun && garuda::beam::to_string(*resRun) == "ok";
    CHECK(is_okRun, "Test WHRun: Web.jalankan(8092) == :ok");

    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    httplib::Client cli("http://127.0.0.1:8092");
    cli.set_connection_timeout(5, 0);
    cli.set_read_timeout(5, 0);

    // Test WH2: Eksekusi Handler Garuda String
    auto res_h1 = cli.Get("/halo");
    CHECK(res_h1 && res_h1->status == 200 && res_h1->body == "Halo dari Handler Garuda!",
          "Test WH2: GET /halo eksekusi Garuda code == 'Halo dari Handler Garuda!'");

    // Test WH3: Eksekusi Handler Garuda Expression
    auto res_calc = cli.Get("/calc");
    CHECK(res_calc && res_calc->status == 200 && res_calc->body == "42",
          "Test WH3: GET /calc eksekusi Garuda expression == '42'");

    // Test WH4: Multiple Handler
    auto res_a = cli.Get("/a");
    auto res_b = cli.Get("/b");
    CHECK(res_a && res_a->body == "AAA" && res_b && res_b->body == "BBB",
          "Test WH4: Multiple handlers GET /a == 'AAA', GET /b == 'BBB'");

    // Stop server
    std::string codeStop = R"(
        modul TestStop
        impor "std/http_server.grd"
        fungsi utama() ->
            HttpServer.stop()
        rampung
    )";
    engine.eval(codeStop);

    std::cout << "\n🎉 ALL FULL GARUDA WEB HANDLER TESTS PASSED PERFECTLY!\n";
    return 0;
}
