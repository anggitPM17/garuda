// Garuda Language Compiler: HTTP Server Test Suite (`test_http_server.cpp`)
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
    std::cout << "GARUDA COMPILER — HTTP SERVER TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test S1: mulai server di port 8090
    std::string codeS1 = R"(
        modul TestS1
        impor "std/http_server.grd"
        fungsi utama() ->
            HttpServer.mulai(8090)
        rampung
    )";
    auto resS1 = engine.eval(codeS1);
    bool is_okS1 = resS1 && garuda::beam::to_string(*resS1) == "ok";
    CHECK(is_okS1, "Test S1: HttpServer.mulai(8090) == :ok");

    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    // Test S2: client C++ GET /hello
    httplib::Client cli("http://127.0.0.1:8090");
    cli.set_connection_timeout(5, 0);
    cli.set_read_timeout(5, 0);
    auto res_get = cli.Get("/hello");
    CHECK(res_get && res_get->status == 200 && res_get->body == "Halo dari Garuda Server!",
          "Test S2: GET /hello == 'Halo dari Garuda Server!'");

    // Test S3: client C++ POST /echo
    auto res_post = cli.Post("/echo", "halo server", "text/plain");
    CHECK(res_post && res_post->status == 200 && res_post->body == "Echo: halo server",
          "Test S3: POST /echo == 'Echo: halo server'");

    // Test S4: stop server
    std::string codeS4 = R"(
        modul TestS4
        impor "std/http_server.grd"
        fungsi utama() ->
            HttpServer.stop()
        rampung
    )";
    auto resS4 = engine.eval(codeS4);
    bool is_okS4 = resS4 && garuda::beam::to_string(*resS4) == "ok";
    CHECK(is_okS4, "Test S4: HttpServer.stop() == :ok");

    std::cout << "\n🎉 ALL HTTP SERVER TESTS PASSED PERFECTLY!\n";
    return 0;
}
