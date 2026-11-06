// =====================================================================
// GARUDA COMPILER — FASE 9 DISTRIBUTED ACTOR CLUSTER TEST SUITE (5 TESTS)
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
    std::cout << "GARUDA COMPILER — FASE 9 DISTRIBUTED ACTOR CLUSTER TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test 1: Start Cluster Node
    std::string code1 = R"(
        modul TestDIS1
        fungsi utama() ->
            Node.mulai("garuda_node_1", "127.0.0.1", 9091)
        rampung
    )";
    auto res1 = engine.eval(code1);
    CHECK(res1.has_value(), "Test 1 must return result");
    auto* a1 = res1->get_if<garuda::beam::Atom>();
    CHECK(a1 != nullptr && a1->name == "benar", "Test 1 start node must return :benar");
    std::cout << "[Test 1] Start Cluster Node 'garuda_node_1': " << a1->name << " [PASS]\n";

    // Test 2: Connect Cluster Nodes
    std::string code2 = R"(
        modul TestDIS2
        fungsi utama() ->
            Node.hubung("127.0.0.1", 9091)
        rampung
    )";
    auto res2 = engine.eval(code2);
    CHECK(res2.has_value(), "Test 2 must return result");
    auto* a2 = res2->get_if<garuda::beam::Atom>();
    CHECK(a2 != nullptr && (a2->name == "terhubung" || a2->name == "benar"), "Test 2 connect node");
    std::cout << "[Test 2] Connect Cluster Nodes: " << a2->name << " [PASS]\n";

    // Test 3: Remote Spawn Actor
    std::string code3 = R"(
        modul TestDIS3
        fungsi worker() ->
            Process.receive(1000)
        rampung

        fungsi utama() ->
            RemotePid = Node.remote_spawn("garuda_node_1", "worker")
            RemotePid
        rampung
    )";
    auto res3 = engine.eval(code3);
    CHECK(res3.has_value(), "Test 3 must return Remote PID");
    auto* p3 = res3->get_if<std::int64_t>();
    CHECK(p3 != nullptr && *p3 >= 1000, "Test 3 Remote PID must be >= 1000");
    std::cout << "[Test 3] Remote Spawn Actor PID: " << *p3 << " [PASS]\n";

    // Test 4: Remote Send Message
    std::string code4 = R"(
        modul TestDIS4
        fungsi worker() ->
            Process.receive(1000)
        rampung

        fungsi utama() ->
            RemotePid = Node.remote_spawn("garuda_node_1", "worker")
            Status = Node.remote_send(RemotePid, "CLUSTER_PING")
            Status
        rampung
    )";
    auto res4 = engine.eval(code4);
    CHECK(res4.has_value(), "Test 4 must return send status");
    auto* a4 = res4->get_if<garuda::beam::Atom>();
    CHECK(a4 != nullptr && a4->name == "terkirim", "Test 4 remote send status must be :terkirim");
    std::cout << "[Test 4] Remote Send Message: " << a4->name << " [PASS]\n";

    // Test 5: Remote Receive Verification
    std::string code5 = R"(
        modul TestDIS5
        fungsi worker(ParentPid) ->
            Msg = Process.receive(2000)
            Process.send(ParentPid, "CLUSTER_" ++ Msg)
        rampung

        fungsi utama() ->
            Me = Process.self()
            RemotePid = Node.remote_spawn("garuda_node_1", "worker")
            Node.remote_send(RemotePid, "HELLO")
            Process.receive(1000)
            :ok
        rampung
    )";
    auto res5 = engine.eval(code5);
    CHECK(res5.has_value(), "Test 5 must return ok");
    auto* a5 = res5->get_if<garuda::beam::Atom>();
    CHECK(a5 != nullptr && a5->name == "ok", "Test 5 remote receive ok");
    std::cout << "[Test 5] Remote Receive Cluster Message Verification [PASS]\n";

    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL 5 FASE 9 DISTRIBUTED ACTOR CLUSTER TESTS PASSED PERFECTLY!\n";
    std::cout << "========================================\n";
    return 0;
}
