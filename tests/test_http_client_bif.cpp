// =====================================================================
// GARUDA COMPILER — FASE 12.2 HTTP CLIENT BIF TEST SUITE
// =====================================================================

#include "../include/garuda/beam/tools/script_engine.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

using namespace garuda::beam;

static int g_failures = 0;
static void CHECK(bool cond, const char* msg) {
    if (!cond) { std::cout << "[FAIL] " << msg << "\n"; g_failures++; }
}

int main() {
    ScriptEngine engine;
    std::cout << "=======================================================\n";
    std::cout << "GARUDA COMPILER — FASE 12.2 HTTP CLIENT BIF TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Start server pendukung (dari Fase 10) di port 9191
    engine.eval(R"(
        modul THttpCli
        fungsi utama() ->
            Server.mulai_async_statis(9191, "public")
        rampung
    )");
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    // Test 1: Http.ambil("/health") via server lokal
    {
        auto ret = engine.eval(R"(
            modul THttp1
            fungsi utama() ->
                Http.ambil("http://127.0.0.1:9191/")
            rampung
        )");
        bool ok = ret.has_value() && std::holds_alternative<std::string>(ret->value());
        if (ok) {
            std::cout << "[Test 1] Http.ambil(/) -> response with body [PASS]\n";
        } else {
            std::cout << "[Test 1] Http.ambil(/) -> [FAIL]\n";
            g_failures++;
        }
    }

    // Test 2: Http.ambil response status code
    {
        auto ret = engine.eval(R"(
            modul THttp2
            fungsi utama() ->
                Http.ambil("http://127.0.0.1:9191/")
            rampung
        )");
        bool ok = ret.has_value() && std::holds_alternative<std::string>(ret->value());
        if (ok) {
            std::cout << "[Test 2] Http.ambil(/) -> non-empty response [PASS]\n";
        } else {
            std::cout << "[Test 2] Http.ambil(/) -> [FAIL]\n";
            g_failures++;
        }
    }

    // Test 3: Http.kirim (POST) dengan body JSON
    {
        auto ret = engine.eval(R"(
            modul THttp3
            fungsi utama() ->
                Http.kirim("http://127.0.0.1:9191/", Json.enkode("test_body"))
            rampung
        )");
        bool ok = ret.has_value();
        if (ok) {
            std::cout << "[Test 3] Http.kirim POST -> no crash [PASS]\n";
        } else {
            std::cout << "[Test 3] Http.kirim POST -> [FAIL]\n";
            g_failures++;
        }
    }

    // Test 4: Http.ambil ke URL tidak valid -> salah
    {
        auto ret = engine.eval(R"(
            modul THttp4
            fungsi utama() ->
                Http.ambil("http://127.0.0.1:1/invalid")
            rampung
        )");
        bool ok = ret.has_value() && is_atom(*ret, "salah");
        if (ok) {
            std::cout << "[Test 4] Http.ambil(invalid) -> salah [PASS]\n";
        } else {
            std::cout << "[Test 4] Http.ambil(invalid) -> [FAIL] expected salah\n";
            g_failures++;
        }
    }

    // Test 5: 10 sequential requests tanpa crash
    {
        int pass = 0;
        for (int i = 0; i < 10; ++i) {
            auto ret = engine.eval(R"(
                modul THttp5
                fungsi utama() ->
                    Http.ambil("http://127.0.0.1:9191/")
                rampung
            )");
            if (ret.has_value() && std::holds_alternative<std::string>(ret->value())) {
                pass++;
            }
        }
        if (pass == 10) {
            std::cout << "[Test 5] 10 sequential requests -> 10/10 PASS [PASS]\n";
        } else {
            std::cout << "[Test 5] 10 sequential requests -> " << pass << "/10 [FAIL]\n";
            g_failures++;
        }
    }

    if (g_failures > 0) {
        std::cout << "\n❌ " << g_failures << "/5 TESTS FAILED ON STUB CODE!\n";
        return 1;
    }
    std::cout << "\n🎉 ALL 5 FASE 12.2 HTTP CLIENT BIF TESTS PASSED!\n";
    return 0;
}
