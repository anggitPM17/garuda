// =====================================================================
// GARUDA COMPILER — FASE 12.1 JSON BIF TEST SUITE (RED -> GREEN)
// =====================================================================

#include "../include/garuda/beam/tools/script_engine.hpp"
#include <iostream>
#include <string>
#include <chrono>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#else
#include <sys/resource.h>
#endif

using namespace garuda::beam;

static int g_failures = 0;
static void CHECK(bool cond, const char* msg) {
    if (!cond) {
        std::cout << "[FAIL] " << msg << "\n";
        g_failures++;
    }
}

static double get_memory_mb() {
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        return static_cast<double>(pmc.WorkingSetSize) / (1024.0 * 1024.0);
    }
    return 0.0;
#else
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        return static_cast<double>(usage.ru_maxrss) / 1024.0;
    }
    return 0.0;
#endif
}

int main() {
    ScriptEngine engine;
    std::cout << "=======================================================\n";
    std::cout << "GARUDA COMPILER — FASE 12.1 JSON BIF TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test 1: Json.enkode("hello") -> "\"hello\""
    {
        auto ret = engine.eval(R"(
            modul TJson1
            fungsi utama() ->
                Json.enkode("hello")
            rampung
        )");
        bool ok = ret.has_value() &&
                  std::holds_alternative<std::string>(ret->value()) &&
                  std::get<std::string>(ret->value()) == "\"hello\"";
        if (ok) {
            std::cout << "[Test 1] Json.enkode(\"hello\") -> \"\\\"hello\\\"\" [PASS]\n";
        } else {
            std::cout << "[Test 1] Json.enkode(\"hello\") -> [FAIL]\n";
            g_failures++;
        }
    }

    // Test 2: Json.enkode(42) -> "42"
    {
        auto ret = engine.eval(R"(
            modul TJson2
            fungsi utama() ->
                Json.enkode(42)
            rampung
        )");
        bool ok = ret.has_value() &&
                  std::holds_alternative<std::string>(ret->value()) &&
                  std::get<std::string>(ret->value()) == "42";
        if (ok) {
            std::cout << "[Test 2] Json.enkode(42) -> \"42\" [PASS]\n";
        } else {
            std::cout << "[Test 2] Json.enkode(42) -> [FAIL]\n";
            g_failures++;
        }
    }

    // Test 3: Json.dekode("\"world\"") -> "world"
    {
        auto ret = engine.eval(R"(
            modul TJson3
            fungsi utama() ->
                Json.dekode("\"world\"")
            rampung
        )");
        bool ok = ret.has_value() &&
                  std::holds_alternative<std::string>(ret->value()) &&
                  std::get<std::string>(ret->value()) == "world";
        if (ok) {
            std::cout << "[Test 3] Json.dekode(\"\\\"world\\\"\") -> \"world\" [PASS]\n";
        } else {
            std::cout << "[Test 3] Json.dekode(\"\\\"world\\\"\") -> [FAIL]\n";
            g_failures++;
        }
    }

    // Test 4: Json.dekode("{...}") -> map dengan key yang bisa diakses
    {
        auto ret = engine.eval(R"(
            modul TJson4
            fungsi utama() ->
                M = Json.dekode("{\"nama\":\"Anggit\",\"usia\":25}"),
                Json.enkode(M)
            rampung
        )");
        bool ok = ret.has_value() &&
                  std::holds_alternative<std::string>(ret->value());
        if (ok) {
            std::string s = std::get<std::string>(ret->value());
            ok = s.find("Anggit") != std::string::npos &&
                 s.find("25") != std::string::npos;
        }
        if (ok) {
            std::cout << "[Test 4] Json round-trip (map) [PASS]\n";
        } else {
            std::cout << "[Test 4] Json round-trip (map) -> [FAIL]\n";
            g_failures++;
        }
    }

    // Test 5: Memori delta < 10MB setelah 1000 enkode/dekode
    {
        double before = get_memory_mb();
        for (int i = 0; i < 1000; ++i) {
            engine.eval(R"(
                modul TJson5
                fungsi utama() ->
                    Json.dekode(Json.enkode("test_payload_12345"))
                rampung
            )");
        }
        double after = get_memory_mb();
        double delta = after - before;
        bool ok = (g_failures == 0) && (delta < 10.0);
        if (ok) {
            std::cout << "[Test 5] Memory delta: " << delta << " MB < 10 MB [PASS]\n";
        } else {
            std::cout << "[Test 5] Memory delta: " << delta << " MB -> [FAIL]\n";
            g_failures++;
        }
    }

    if (g_failures > 0) {
        std::cout << "\n❌ " << g_failures << "/5 TESTS FAILED ON STUB CODE!\n";
        return 1;
    }
    std::cout << "\n🎉 ALL 5 FASE 12.1 JSON BIF TESTS PASSED!\n";
    return 0;
}
