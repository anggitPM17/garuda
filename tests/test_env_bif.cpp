// =====================================================================
// GARUDA COMPILER — FASE 12.3 ENV BIF TEST SUITE
// =====================================================================

#include "../include/garuda/beam/tools/script_engine.hpp"
#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>

using namespace garuda::beam;

static int g_failures = 0;
static void CHECK(bool cond, const char* msg) {
    if (!cond) { std::cout << "[FAIL] " << msg << "\n"; g_failures++; }
}

int main() {
    ScriptEngine engine;
    std::cout << "=======================================================\n";
    std::cout << "GARUDA COMPILER — FASE 12.3 ENV BIF TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Set environment variable untuk test
    #ifdef _WIN32
    _putenv_s("GARUDA_TEST_VAR", "hello_env");
    #else
    setenv("GARUDA_TEST_VAR", "hello_env", 1);
    #endif

    // Test 1: Env.baca("GARUDA_TEST_VAR") -> "hello_env"
    {
        auto ret = engine.eval(R"(
            modul TEnv1
            fungsi utama() ->
                Env.baca("GARUDA_TEST_VAR")
            rampung
        )");
        bool ok = ret.has_value() &&
                  std::holds_alternative<std::string>(ret->value()) &&
                  std::get<std::string>(ret->value()) == "hello_env";
        if (ok) {
            std::cout << "[Test 1] Env.baca(GARUDA_TEST_VAR) -> hello_env [PASS]\n";
        } else {
            std::cout << "[Test 1] Env.baca(GARUDA_TEST_VAR) -> [FAIL]\n";
            g_failures++;
        }
    }

    // Test 2: Env.baca("NONEXISTENT_VAR_XYZ") -> nil
    {
        auto ret = engine.eval(R"(
            modul TEnv2
            fungsi utama() ->
                Env.baca("NONEXISTENT_VAR_XYZ")
            rampung
        )");
        bool ok = ret.has_value() && is_atom(*ret, "nil");
        if (ok) {
            std::cout << "[Test 2] Env.baca(NONEXISTENT_VAR_XYZ) -> nil [PASS]\n";
        } else {
            std::cout << "[Test 2] Env.baca(NONEXISTENT_VAR_XYZ) -> [FAIL]\n";
            g_failures++;
        }
    }

    // Test 3: Env.baca dengan default fallback (2 arg)
    {
        auto ret = engine.eval(R"(
            modul TEnv3
            fungsi utama() ->
                Env.baca("NONEXISTENT_VAR_XYZ", "default_value")
            rampung
        )");
        bool ok = ret.has_value() &&
                  std::holds_alternative<std::string>(ret->value()) &&
                  std::get<std::string>(ret->value()) == "default_value";
        if (ok) {
            std::cout << "[Test 3] Env.baca(var, default) -> default_value [PASS]\n";
        } else {
            std::cout << "[Test 3] Env.baca(var, default) -> [FAIL]\n";
            g_failures++;
        }
    }

    // Test 4: Muat file .grd.config jika ada
    {
        // Tulis file config test
        std::ofstream cfg("test_grd.config");
        cfg << "APP_PORT=8080\nAPP_NAME=TodoDemo\n";
        cfg.close();

        auto ret = engine.eval(R"(
            modul TEnv4
            fungsi utama() ->
                Env.muat_config("test_grd.config")
            rampung
        )");
        bool ok = ret.has_value() && is_atom(*ret, "benar");
        if (ok) {
            std::cout << "[Test 4] Env.muat_config(test_grd.config) -> benar [PASS]\n";
        } else {
            std::cout << "[Test 4] Env.muat_config(test_grd.config) -> [FAIL]\n";
            g_failures++;
        }
        std::remove("test_grd.config");
    }

    // Test 5: Env.baca setelah muat_config -> nilai dari file
    {
        std::ofstream cfg("test_grd.config");
        cfg << "CUSTOM_VAR=custom_value_123\n";
        cfg.close();

        auto ret = engine.eval(R"(
            modul TEnv5
            fungsi utama() ->
                Env.muat_config("test_grd.config"),
                Env.baca("CUSTOM_VAR")
            rampung
        )");
        bool ok = ret.has_value() &&
                  std::holds_alternative<std::string>(ret->value()) &&
                  std::get<std::string>(ret->value()) == "custom_value_123";
        if (ok) {
            std::cout << "[Test 5] Env.baca after muat_config -> custom_value_123 [PASS]\n";
        } else {
            std::cout << "[Test 5] Env.baca after muat_config -> [FAIL]\n";
            g_failures++;
        }
        std::remove("test_grd.config");
    }

    if (g_failures > 0) {
        std::cout << "\n❌ " << g_failures << "/5 TESTS FAILED ON STUB CODE!\n";
        return 1;
    }
    std::cout << "\n🎉 ALL 5 FASE 12.3 ENV BIF TESTS PASSED!\n";
    return 0;
}
