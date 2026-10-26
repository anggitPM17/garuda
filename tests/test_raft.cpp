#include "../include/garuda/beam/net/raft.hpp"
#include <cassert>
#include <chrono>
#include <iostream>
#include <map>
#include <queue>

using namespace garuda::beam;

#define TEST_ASSERT(cond, msg) do { \
    if (!(cond)) { \
        std::cerr << "\n❌ ASSERTION FAILED: " << (msg) << " at " << __FILE__ << ":" << __LINE__ << "\n"; \
        std::exit(1); \
    } \
} while(0)

int main(int argc, char* argv[]) {
    // Optional sanity check flag
    if (argc > 1 && std::string(argv[1]) == "--sanity-check") {
        std::cout << "Running TEST_ASSERT sanity check...\n";
        TEST_ASSERT(1 == 2, "Sanity Check Failure Verification");
        return 0;
    }

    std::cout << "=======================================================\n";
    std::cout << "GARUDA DISTRIBUTED — SPRINT 3.1 RAFT CONSENSUS TEST SUITE (5 TESTS)\n";
    std::cout << "=======================================================\n\n";

    // === Test 1: Basic Leader Election ===
    std::cout << "[TEST 1/5] Running Basic Leader Election test...\n";
    RaftNode node_a("node_a", {"node_b", "node_c"});
    RaftNode node_b("node_b", {"node_a", "node_c"});
    RaftNode node_c("node_c", {"node_a", "node_b"});

    std::map<std::string, RaftNode*> cluster = {
        {"node_a", &node_a},
        {"node_b", &node_b},
        {"node_c", &node_c}
    };

    std::queue<std::pair<std::string, std::string>> net_queue;

    auto wire_network = [&](RaftNode& node) {
        node.set_send_callback([&net_queue](const std::string& target, const std::string& msg) {
            net_queue.push({target, msg});
        });
    };

    wire_network(node_a);
    wire_network(node_b);
    wire_network(node_c);

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

    node_a.start_election();
    drain_network();

    assert(node_a.role() == RaftRole::Leader);
    assert(node_a.current_term() == 1);
    std::cout << "  ✓ Test 1: Leader Election Node A elected Leader for Term 1 PASS\n";

    // === Test 2: Log Replication ===
    std::cout << "[TEST 2/5] Running Log Replication test...\n";
    bool app_ok = node_a.append_command("SET_X_10");
    drain_network();

    assert(app_ok);
    assert(node_a.log_size() == 1);
    assert(node_b.log_size() == 1);
    assert(node_c.log_size() == 1);
    std::cout << "  ✓ Test 2: Log Replication across all 3 nodes PASS\n";

    // === Test 3: Kill Leader & Re-elect ===
    std::cout << "[TEST 3/5] Running Kill Leader & Re-elect test...\n";
    // Disconnect Node A (simulate crash/partition)
    node_a.set_send_callback([](const std::string&, const std::string&) {});

    node_b.start_election();
    drain_network();

    assert(node_b.role() == RaftRole::Leader);
    assert(node_b.current_term() == 2);
    std::cout << "  ✓ Test 3: Kill Leader & Re-elect Node B elected Leader for Term 2 PASS\n";

    // === Test 4: Partition Tolerance (2/3 Majority Operates) ===
    std::cout << "[TEST 4/5] Running Partition Tolerance test...\n";
    bool partition_app_ok = node_b.append_command("SET_Y_20");
    drain_network();

    assert(partition_app_ok);
    assert(node_b.log_size() == 2);
    assert(node_c.log_size() == 2);
    std::cout << "  ✓ Test 4: Partition Tolerance 2/3 Majority Commits Log PASS\n";

    // === Test 5: State Persistence (save_state & load_state) ===
    std::cout << "[TEST 5/5] Running State Persistence test...\n";
    std::string state_file = "./raft_node_b.state";
    node_b.save_state(state_file);

    RaftNode node_b_recovered("node_b", {"node_a", "node_c"});
    bool load_ok = node_b_recovered.load_state(state_file);

    assert(load_ok);
    assert(node_b_recovered.current_term() == 2);
    assert(node_b_recovered.voted_for() == "node_b");
    assert(node_b_recovered.log_size() == 2);
    std::cout << "  ✓ Test 5: State Persistence save_state & load_state PASS\n";

    std::cout << "\n🎉 ALL 5 RAFT CONSENSUS TESTS PASSED PERFECTLY!\n";
    return 0;
}
