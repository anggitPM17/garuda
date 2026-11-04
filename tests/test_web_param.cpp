// Garuda Language Compiler: Web Handler Parameter Test Suite (`test_web_param.cpp`)
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
    std::cout << "GARUDA COMPILER — WEB HANDLER PARAMETERS TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Daftarkan handler berparameter + route, jalankan server di port 8094
    std::string codeRun = R"(
        modul TestWebParamRun
        impor "std/web.grd"
        fungsi utama() ->
            Web.daftar_handler_param("h_user", "
                impor \"std/map.grd\"
                fungsi utama(Query, Body, Method, Path) ->
                    Map.ambil(Query, \"id\")
                rampung
            ")

            Web.daftar_handler_param("h_data", "
                fungsi utama(Query, Body, Method, Path) ->
                    \"diterima: \" ++ Body
                rampung
            ")

            Web.daftar_handler_param("h_info", "
                fungsi utama(Query, Body, Method, Path) ->
                    Method ++ \" \" ++ Path
                rampung
            ")

            Web.get("/user", "h_user")
            Web.post("/data", "h_data")
            Web.get("/info", "h_info")
            Web.jalankan(8094)
        rampung
    )";
    auto resRun = engine.eval(codeRun);
    bool is_okRun = resRun && garuda::beam::to_string(*resRun) == "ok";
    CHECK(is_okRun, "Test WPRun: Web.jalankan(8094) == :ok");

    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    httplib::Client cli("http://127.0.0.1:8094");
    cli.set_connection_timeout(5, 0);
    cli.set_read_timeout(5, 0);

    // Test W1: GET /user?id=1 -> handler menerima Query map dengan id="1"
    auto res_w1 = cli.Get("/user?id=1");
    CHECK(res_w1 && res_w1->status == 200 && res_w1->body == "1",
          "Test W1: GET /user?id=1 -> Query id == '1'");

    // Test W2: POST /data body="hello" -> handler menerima Body="hello"
    auto res_w2 = cli.Post("/data", "hello", "text/plain");
    CHECK(res_w2 && res_w2->status == 200 && res_w2->body == "diterima: hello",
          "Test W2: POST /data body='hello' -> Body == 'diterima: hello'");

    // Test W3: GET /info -> handler menerima Method="GET" dan Path="/info"
    auto res_w3 = cli.Get("/info");
    CHECK(res_w3 && res_w3->status == 200 && res_w3->body == "GET /info",
          "Test W3: GET /info -> Method & Path == 'GET /info'");

    // Stop server
    std::string codeStop = R"(
        modul TestStop
        impor "std/http_server.grd"
        fungsi utama() ->
            HttpServer.stop()
        rampung
    )";
    engine.eval(codeStop);

    std::cout << "\n🎉 ALL WEB HANDLER PARAMETER TESTS PASSED PERFECTLY!\n";
    return 0;
}
