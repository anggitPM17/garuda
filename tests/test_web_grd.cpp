// Garuda Language Compiler: Web Framework Test Suite (`test_web_grd.cpp`)
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
    std::cout << "GARUDA COMPILER — WEB FRAMEWORK TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test WEB1: Web.daftar_route mengembalikan :ok
    std::string code1 = R"(
        modul TestWEB1
        impor "std/web.grd"
        fungsi utama() ->
            Web.daftar_route("GET", "/test", "handler_test")
        rampung
    )";
    auto res1 = engine.eval(code1);
    bool is_ok1 = res1 && garuda::beam::to_string(*res1) == "ok";
    CHECK(is_ok1, "Test WEB1: Web.daftar_route('GET', '/test', 'handler_test') == :ok");

    // Test WEB2: Web.get shorthand
    std::string code2 = R"(
        modul TestWEB2
        impor "std/web.grd"
        fungsi utama() ->
            Web.get("/halo", "handler_halo")
        rampung
    )";
    auto res2 = engine.eval(code2);
    bool is_ok2 = res2 && garuda::beam::to_string(*res2) == "ok";
    CHECK(is_ok2, "Test WEB2: Web.get('/halo', 'handler_halo') == :ok");

    // Test WEB3: Web.post shorthand
    std::string code3 = R"(
        modul TestWEB3
        impor "std/web.grd"
        fungsi utama() ->
            Web.post("/kirim", "handler_kirim")
        rampung
    )";
    auto res3 = engine.eval(code3);
    bool is_ok3 = res3 && garuda::beam::to_string(*res3) == "ok";
    CHECK(is_ok3, "Test WEB3: Web.post('/kirim', 'handler_kirim') == :ok");

    // Test WEB4: Jalankan Web server & tes HTTP GET /halo dan POST /kirim
    std::string code4 = R"(
        modul TestWEB4
        impor "std/web.grd"
        fungsi utama() ->
            Web.jalankan(8091)
        rampung
    )";
    auto res4 = engine.eval(code4);
    bool is_ok4 = res4 && garuda::beam::to_string(*res4) == "ok";
    CHECK(is_ok4, "Test WEB4: Web.jalankan(8091) == :ok");

    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    // Test GET /halo
    httplib::Client cli("http://127.0.0.1:8091");
    cli.set_connection_timeout(5, 0);
    cli.set_read_timeout(5, 0);

    auto res_get = cli.Get("/halo");
    CHECK(res_get && res_get->status == 200 && res_get->body == "Route GET -> handler_halo",
          "Test WEB4 GET: /halo == 'Route GET -> handler_halo'");

    // Test POST /kirim
    auto res_post = cli.Post("/kirim", "data_payload", "text/plain");
    CHECK(res_post && res_post->status == 200 && res_post->body == "Route POST data_payload -> handler_kirim",
          "Test WEB4 POST: /kirim == 'Route POST data_payload -> handler_kirim'");

    // Stop server
    std::string codeStop = R"(
        modul TestStop
        impor "std/http_server.grd"
        fungsi utama() ->
            HttpServer.stop()
        rampung
    )";
    engine.eval(codeStop);

    std::cout << "\n🎉 ALL WEB FRAMEWORK TESTS PASSED PERFECTLY!\n";
    return 0;
}
