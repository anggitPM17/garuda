// =====================================================================
// GARUDA COMPILER — FASE 9 SUPERVISOR TREE TEST SUITE (5 TESTS)
// =====================================================================

#include "../include/garuda/beam/tools/script_engine.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <cstdlib>

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        std::cerr << "[FAIL] " << msg << "\n"; \
        std::exit(1); \
    } \
} while(0)

int main() {
    garuda::beam::ScriptEngine engine;

    std::cout << "=======================================================\n";
    std::cout << "GARUDA COMPILER — FASE 9 SUPERVISOR TREE TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test 1: Supervisor Start
    std::string code1 = R"(
        modul TestSUP1
        fungsi worker() ->
            Process.receive(1000)
        rampung

        fungsi utama() ->
            SupPid = Supervisor.mulai("worker", "satu_untuk_satu")
            SupPid
        rampung
    )";
    auto res1 = engine.eval(code1);
    CHECK(res1.has_value(), "Test 1 must return Supervisor PID");
    auto* p1 = res1->get_if<std::int64_t>();
    CHECK(p1 != nullptr && *p1 >= 1000, "Test 1 Supervisor PID must be >= 1000");
    std::cout << "[Test 1] Supervisor Start PID: " << *p1 << " [PASS]\n";

    // Test 2: Child Worker Restart
    std::string code2 = R"(
        modul TestSUP2
        fungsi worker() ->
            Process.receive(1000)
        rampung

        fungsi utama() ->
            SupPid = Supervisor.mulai("worker", "satu_untuk_satu")
            NewChildPid = Supervisor.restart(SupPid)
            NewChildPid
        rampung
    )";
    auto res2 = engine.eval(code2);
    CHECK(res2.has_value(), "Test 2 must return new child PID");
    auto* p2 = res2->get_if<std::int64_t>();
    CHECK(p2 != nullptr && *p2 > 1000, "Test 2 new child PID must be > 1000");
    std::cout << "[Test 2] Child Worker Restart PID: " << *p2 << " [PASS]\n";

    // Test 3: Supervisor Health Query
    std::string code3 = R"(
        modul TestSUP3
        fungsi worker() ->
            Process.receive(1000)
        rampung

        fungsi utama() ->
            SupPid = Supervisor.mulai("worker", "satu_untuk_satu")
            Supervisor.restart(SupPid)
            Health = Supervisor.health(SupPid)
            Map.ambil(Health, "restarts")
        rampung
    )";
    auto res3 = engine.eval(code3);
    CHECK(res3.has_value(), "Test 3 must return restarts count");
    auto* r3 = res3->get_if<std::int64_t>();
    CHECK(r3 != nullptr && *r3 == 1, "Test 3 restarts count must be 1");
    std::cout << "[Test 3] Supervisor Health Query Restarts: " << *r3 << " [PASS]\n";

    // Test 4: Restart Limit Escalation
    std::string code4 = R"(
        modul TestSUP4
        fungsi worker() ->
            Process.receive(1000)
        rampung

        fungsi utama() ->
            SupPid = Supervisor.mulai("worker", "satu_untuk_satu")
            Supervisor.restart(SupPid)
            Supervisor.restart(SupPid)
            Supervisor.restart(SupPid)
            Result = Supervisor.restart(SupPid)
            Result
        rampung
    )";
    auto res4 = engine.eval(code4);
    CHECK(res4.has_value(), "Test 4 must return escalation status");
    auto* a4 = res4->get_if<garuda::beam::Atom>();
    CHECK(a4 != nullptr && a4->name == "eskalasi", "Test 4 4th restart must escalate");
    std::cout << "[Test 4] Restart Limit Escalation: " << a4->name << " [PASS]\n";

    // Test 5: Strategy Specification Check
    std::string code5 = R"(
        modul TestSUP5
        fungsi worker() ->
            Process.receive(1000)
        rampung

        fungsi utama() ->
            SupPid = Supervisor.mulai("worker", "satu_untuk_semua")
            Health = Supervisor.health(SupPid)
            Map.ambil(Health, "strategi")
        rampung
    )";
    auto res5 = engine.eval(code5);
    CHECK(res5.has_value(), "Test 5 must return strategy string");
    auto* s5 = res5->get_if<std::string>();
    CHECK(s5 != nullptr && *s5 == "satu_untuk_semua", "Test 5 strategy must be 'satu_untuk_semua'");
    std::cout << "[Test 5] Supervisor Strategy 'satu_untuk_semua': " << *s5 << " [PASS]\n";

    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL 5 FASE 9 SUPERVISOR TREE TESTS PASSED PERFECTLY!\n";
    std::cout << "========================================\n";
    return 0;
}
