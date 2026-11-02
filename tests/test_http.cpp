// Garuda Language Compiler: HTTP Client Test Suite (`test_http.cpp`)
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
    // Spawn local test HTTP server on 127.0.0.1:8089 (offline-safe & deterministic)
    httplib::Server svr;
    svr.Get("/hello", [](const httplib::Request&, httplib::Response& res) {
        res.set_content("Halo dari GVM HTTP Server!", "text/plain");
    });
    svr.Post("/echo_json", [](const httplib::Request& req, httplib::Response& res) {
        res.set_content("Pesan diterima: " + req.body, "application/json");
    });

    std::thread svr_thread([&svr]() {
        svr.listen("127.0.0.1", 8089);
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    garuda::beam::ScriptEngine engine;

    std::cout << "=======================================================\n";
    std::cout << "GARUDA COMPILER — HTTP CLIENT TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test H1: Http.ambil("http://127.0.0.1:8089/hello") -> "Halo dari GVM HTTP Server!"
    std::string codeH1 = R"(
        modul TestH1
        impor "std/http.grd"
        fungsi utama() ->
            Http.ambil("http://127.0.0.1:8089/hello")
        rampung
    )";
    auto resH1 = engine.eval(codeH1);
    auto* sH1 = resH1 ? resH1->get_if<std::string>() : nullptr;
    CHECK(sH1 && *sH1 == "Halo dari GVM HTTP Server!", "Test H1: Http.ambil GET == 'Halo dari GVM HTTP Server!'");

    // Test H2: Http.kirim_json("http://127.0.0.1:8089/echo_json", "{\"test\":1}")
    std::string codeH2 = R"(
        modul TestH2
        impor "std/http.grd"
        fungsi utama() ->
            Http.kirim_json("http://127.0.0.1:8089/echo_json", "{\"test\":1}")
        rampung
    )";
    auto resH2 = engine.eval(codeH2);
    auto* sH2 = resH2 ? resH2->get_if<std::string>() : nullptr;
    CHECK(sH2 && sH2->find("Pesan diterima: {\"test\":1}") != std::string::npos, "Test H2: Http.kirim_json POST == 'Pesan diterima: {\"test\":1}'");

    // Test H3: Http.ambil URL invalid -> error_http atom
    std::string codeH3 = R"(
        modul TestH3
        impor "std/http.grd"
        fungsi utama() ->
            Http.ambil("http://127.0.0.1:59999/invalid")
        rampung
    )";
    auto resH3 = engine.eval(codeH3);
    bool is_errH3 = resH3 && garuda::beam::to_string(*resH3) == "error_http";
    CHECK(is_errH3, "Test H3: Http.ambil URL invalid == :error_http");

    svr.stop();
    if (svr_thread.joinable()) svr_thread.join();

    std::cout << "\n🎉 ALL HTTP CLIENT TESTS PASSED PERFECTLY!\n";
    return 0;
}
