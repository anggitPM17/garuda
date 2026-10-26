#include "../include/garuda/beam/net/global_registry.hpp"
#include <cassert>
#include <iostream>
#include <map>
#include <queue>

using namespace garuda::beam;

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA DISTRIBUTED — SPRINT 3.2 GLOBAL REGISTRY TEST SUITE (5 TESTS)\n";
    std::cout << "=======================================================\n\n";

    // === Test 1: Daftar dan Cari di Single Node ===
    std::cout << "[TEST 1/5] Running Single Node Register & Search test...\n";
    RaftNode node_single("node_single", {});
    GlobalRegistry reg_single(node_single);

    node_single.start_election();
    assert(node_single.role() == RaftRole::Leader);

    bool reg_ok1 = reg_single.daftar("kalkulator", Pid{42});
    assert(reg_ok1);

    auto pid1 = reg_single.cari("kalkulator");
    assert(pid1.has_value() && pid1->id == 42);
    std::cout << "  ✓ Test 1: Single Node Register & Search PASS\n";

    // === Test 2: Sync Antar 3 Node ===
    std::cout << "[TEST 2/5] Running Sync Across 3 Nodes test...\n";
    RaftNode n_a("A", {"B", "C"});
    RaftNode n_b("B", {"A", "C"});
    RaftNode n_c("C", {"A", "B"});

    GlobalRegistry r_a(n_a);
    GlobalRegistry r_b(n_b);
    GlobalRegistry r_c(n_c);

    std::map<std::string, RaftNode*> cluster = {
        {"A", &n_a},
        {"B", &n_b},
        {"C", &n_c}
    };

    std::queue<std::pair<std::string, std::string>> net_queue;

    auto wire_network = [&](RaftNode& node) {
        node.set_send_callback([&net_queue](const std::string& target, const std::string& msg) {
            net_queue.push({target, msg});
        });
    };

    wire_network(n_a);
    wire_network(n_b);
    wire_network(n_c);

    auto drain_network = [&]() {
        while (!net_queue.empty()) {
            auto [target, msg] = net_queue.front();
            net_queue.pop();
            auto it = cluster.find(target);
            if (it != cluster.end()) {
                it->second->process_network_message(msg);
            }
        }
    };

    n_a.start_election();
    drain_network();

    assert(n_a.role() == RaftRole::Leader);

    bool reg_ok2 = r_a.daftar("chat_room_1", Pid{100});
    drain_network();

    assert(reg_ok2);
    auto pid_b = r_b.cari("chat_room_1");
    auto pid_c = r_c.cari("chat_room_1");

    assert(pid_b.has_value() && pid_b->id == 100);
    assert(pid_c.has_value() && pid_c->id == 100);
    std::cout << "  ✓ Test 2: Sync Across 3 Nodes PASS\n";

    // === Test 3: Hapus Nama ===
    std::cout << "[TEST 3/5] Running Unregister Name test...\n";
    r_a.daftar("counter", Pid{42});
    drain_network();
    assert(r_b.cari("counter").has_value());

    bool del_ok = r_a.hapus("counter");
    drain_network();

    assert(del_ok);
    assert(!r_a.cari("counter").has_value());
    assert(!r_b.cari("counter").has_value());
    std::cout << "  ✓ Test 3: Unregister Name PASS\n";

    // === Test 4: Follower Tidak Bisa Daftar (Hanya Leader) ===
    std::cout << "[TEST 4/5] Running Follower Cannot Register test...\n";
    bool follower_reg = r_b.daftar("test_fail", Pid{99});
    assert(!follower_reg); // Follower attempt fails!

    bool leader_reg = r_a.daftar("test_success", Pid{88});
    drain_network();
    assert(leader_reg);
    assert(r_b.cari("test_success").has_value());
    std::cout << "  ✓ Test 4: Follower Cannot Register Directly PASS\n";

    // === Test 5: Kill Leader, Registry Survives on New Leader ===
    std::cout << "[TEST 5/5] Running Kill Leader & Registry Survives test...\n";
    // Disconnect Node A
    n_a.set_send_callback([](const std::string&, const std::string&) {});

    n_b.start_election();
    drain_network();

    assert(n_b.role() == RaftRole::Leader);
    assert(r_b.cari("chat_room_1").has_value());
    assert(r_b.cari("chat_room_1")->id == 100);
    std::cout << "  ✓ Test 5: Kill Leader & Registry Survives PASS\n";

    std::cout << "\n🎉 ALL 5 GLOBAL REGISTRY TESTS PASSED PERFECTLY!\n";
    return 0;
}
