// =====================================================================
// GARUDA COMPILER — FASE 9 ACTOR CONCURRENCY TEST SUITE (5 TESTS)
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
    std::cout << "GARUDA COMPILER — FASE 9 ACTOR CONCURRENCY TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test 1: Actor Spawn & PID Generation
    std::string code1 = R"(
        modul TestACT1
        fungsi worker() ->
            W = Process.receive(1000)
            W
        rampung

        fungsi utama() ->
            Pid = Process.spawn("worker")
            Pid
        rampung
    )";
    auto res1 = engine.eval(code1);
    CHECK(res1.has_value(), "Test 1 must return PID");
    auto* p1 = res1->get_if<std::int64_t>();
    CHECK(p1 != nullptr && *p1 >= 1000, "Test 1 PID must be >= 1000");
    std::cout << "[Test 1] Actor Spawn PID: " << *p1 << " [PASS]\n";

    // Test 2: Message Passing & Response
    std::string code2 = R"(
        modul TestACT2
        fungsi worker(ParentPid) ->
            Msg = Process.receive(2000)
            Process.send(ParentPid, "PONG")
        rampung

        fungsi utama() ->
            Me = Process.self()
            Pid = Process.spawn("worker", Me)
            Process.send(Pid, "PING")
            Reply = Process.receive(2000)
            Reply
        rampung
    )";
    auto res2 = engine.eval(code2);
    CHECK(res2.has_value(), "Test 2 must return response");
    auto* s2 = res2->get_if<std::string>();
    CHECK(s2 != nullptr && *s2 == "PONG", "Test 2 reply must be 'PONG'");
    std::cout << "[Test 2] Message Passing PING-PONG: " << *s2 << " [PASS]\n";

    // Test 3: Self PID Check
    std::string code3 = R"(
        modul TestACT3
        fungsi utama() ->
            Me = Process.self()
            Me
        rampung
    )";
    auto res3 = engine.eval(code3);
    CHECK(res3.has_value(), "Test 3 must return Self PID");
    auto* p3 = res3->get_if<std::int64_t>();
    CHECK(p3 != nullptr && *p3 > 0, "Test 3 Self PID must be > 0");
    std::cout << "[Test 3] Process.self() PID: " << *p3 << " [PASS]\n";

    // Test 4: Mailbox FIFO Ordering
    std::string code4 = R"(
        modul TestACT4
        fungsi worker(ParentPid) ->
            M1 = Process.receive(1000)
            M2 = Process.receive(1000)
            Process.send(ParentPid, M1 ++ "_" ++ M2)
        rampung

        fungsi utama() ->
            Me = Process.self()
            Pid = Process.spawn("worker", Me)
            Process.send(Pid, "FIRST")
            Process.send(Pid, "SECOND")
            Result = Process.receive(2000)
            Result
        rampung
    )";
    auto res4 = engine.eval(code4);
    CHECK(res4.has_value(), "Test 4 must return FIFO result");
    auto* s4 = res4->get_if<std::string>();
    CHECK(s4 != nullptr && *s4 == "FIRST_SECOND", "Test 4 FIFO ordering must be 'FIRST_SECOND'");
    std::cout << "[Test 4] Mailbox FIFO Ordering: " << *s4 << " [PASS]\n";

    // Test 5: Actor Lifecycle & Alive Check
    std::string code5 = R"(
        modul TestACT5
        fungsi worker() ->
            Process.receive(100)
            :done
        rampung

        fungsi utama() ->
            Pid = Process.spawn("worker")
            IsAlive = Process.is_alive(Pid)
            Process.send(Pid, "STOP")
            IsAlive
        rampung
    )";
    auto res5 = engine.eval(code5);
    CHECK(res5.has_value(), "Test 5 must return alive status");
    auto* a5 = res5->get_if<garuda::beam::Atom>();
    CHECK(a5 != nullptr && a5->name == "benar", "Test 5 initial status must be 'benar'");
    std::cout << "[Test 5] Actor Lifecycle Alive Check: " << a5->name << " [PASS]\n";

    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL 5 FASE 9 ACTOR CONCURRENCY TESTS PASSED PERFECTLY!\n";
    std::cout << "========================================\n";
    return 0;
}
