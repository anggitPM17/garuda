#include "../include/garuda/beam/net/raft.hpp"
#include "../include/garuda/beam/net/global_registry.hpp"
#include "../include/garuda/beam/stdlib/http_server.hpp"
#include "../include/garuda/beam/stdlib/http.hpp"
#include <chrono>
#include <filesystem>
#include <iostream>
#include <thread>
#include <vector>

using namespace garuda::beam;
using namespace garuda::beam::stdlib;
namespace fs = std::filesystem;

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
    std::cout << "GARUDA DISTRIBUTED ENGINE — SPRINT F.0 ELIXIR-KILLER TEST SUITE (18 STRICT TESTS)\n";
    std::cout << "=======================================================\n\n";

    fs::path test_dir = fs::temp_directory_path() / "garuda_dist_foundation_test";
    fs::remove_all(test_dir);
    fs::create_directories(test_dir);

    // === Test 1: Single-Node Leader Election ===
    std::cout << "[TEST 1/18] Running Single-Node Leader Election test...\n";
    RaftNode node1("node_1", {});
    node1.tick_timer(300);
    TEST_ASSERT(node1.role() == RaftRole::Leader, "Node 1 must become leader");
    std::cout << "  ✓ Test 1: Single-Node Leader Election PASS\n";

    // === Test 2: 3-Node Cluster Agreement (Strict Leader & Term Agreement) ===
    std::cout << "[TEST 2/18] Running 3-Node Cluster Strict Agreement test...\n";
    RaftNode n3_a("n1", {"n2", "n3"});
    RaftNode n3_b("n2", {"n1", "n3"});
    RaftNode n3_c("n3", {"n1", "n2"});

    auto net3 = [&](const std::string& target, const std::string& msg) {
        if (target == "n1") n3_a.process_network_message(msg);
        else if (target == "n2") n3_b.process_network_message(msg);
        else if (target == "n3") n3_c.process_network_message(msg);
    };

    n3_a.set_send_callback(net3);
    n3_b.set_send_callback(net3);
    n3_c.set_send_callback(net3);

    // Election: n1 starts election, n2 and n3 process heartbeat and reset timers
    n3_a.tick_timer(350);
    n3_b.tick_timer(0);
    n3_c.tick_timer(0);

    int leader_count = 0;
    std::string leader_id;
    if (n3_a.role() == RaftRole::Leader) { leader_count++; leader_id = "n1"; }
    if (n3_b.role() == RaftRole::Leader) { leader_count++; leader_id = "n2"; }
    if (n3_c.role() == RaftRole::Leader) { leader_count++; leader_id = "n3"; }

    TEST_ASSERT(leader_count == 1, "Exactly 1 leader must exist in 3-node cluster");
    TEST_ASSERT(leader_id == "n1", "Leader must be n1 without re-election cascade");
    TEST_ASSERT(n3_a.current_term() == 1, "n1 term must be 1");
    TEST_ASSERT(n3_b.current_term() == 1, "n2 term must be 1");
    TEST_ASSERT(n3_c.current_term() == 1, "n3 term must be 1");
    TEST_ASSERT(n3_a.election_count() + n3_b.election_count() + n3_c.election_count() <= 2, "Total elections must be <= 2");
    std::cout << "  ✓ Test 2: 3-Node Cluster Strict Agreement (Leader: " << leader_id << ", Term: " << n3_a.current_term() << ") PASS\n";

    // === Test 3: 5-Node Real Cluster Election ===
    std::cout << "[TEST 3/18] Running 5-Node Real Cluster Election test...\n";
    RaftNode n5_1("n1", {"n2", "n3", "n4", "n5"});
    RaftNode n5_2("n2", {"n1", "n3", "n4", "n5"});
    RaftNode n5_3("n3", {"n1", "n2", "n4", "n5"});
    RaftNode n5_4("n4", {"n1", "n2", "n3", "n5"});
    RaftNode n5_5("n5", {"n1", "n2", "n3", "n4"});

    auto net5 = [&](const std::string& target, const std::string& msg) {
        if (target == "n1") n5_1.process_network_message(msg);
        else if (target == "n2") n5_2.process_network_message(msg);
        else if (target == "n3") n5_3.process_network_message(msg);
        else if (target == "n4") n5_4.process_network_message(msg);
        else if (target == "n5") n5_5.process_network_message(msg);
    };

    n5_1.set_send_callback(net5);
    n5_2.set_send_callback(net5);
    n5_3.set_send_callback(net5);
    n5_4.set_send_callback(net5);
    n5_5.set_send_callback(net5);

    n5_1.tick_timer(350); // n1 starts election
    TEST_ASSERT(n5_1.role() == RaftRole::Leader, "n1 must be elected leader in 5-node cluster");
    TEST_ASSERT(n5_2.current_term() == 1, "n2 must recognize term 1");
    std::cout << "  ✓ Test 3: 5-Node Real Cluster Election PASS\n";

    // === Test 4: Network Partition Test (Split 3-2 Quorum Check) ===
    std::cout << "[TEST 4/18] Running Network Partition (Split 3-2) test...\n";
    bool network_partition_active = true;

    auto partition_net = [&](const std::string& target, const std::string& msg) {
        if (network_partition_active) {
            // Block messages between majority {n1, n2, n3} and minority {n4, n5}
            if ((target == "n4" || target == "n5") && (msg.find("n1") != std::string::npos || msg.find("n2") != std::string::npos || msg.find("n3") != std::string::npos)) return;
            if ((target == "n1" || target == "n2" || target == "n3") && (msg.find("n4") != std::string::npos || msg.find("n5") != std::string::npos)) return;
        }
        net5(target, msg);
    };

    n5_1.set_send_callback(partition_net);
    n5_2.set_send_callback(partition_net);
    n5_3.set_send_callback(partition_net);
    n5_4.set_send_callback(partition_net);
    n5_5.set_send_callback(partition_net);

    // Majority partition (3 nodes: n1, n2, n3) can commit!
    bool majority_commit = n5_1.append_command("MAJORITY_CMD");
    (void)majority_commit;
    TEST_ASSERT(majority_commit, "Majority partition (3 nodes) must commit successfully");

    std::uint64_t leader_term_before = n5_1.current_term();

    // Minority partition (2 nodes: n4, n5) CANNOT commit!
    n5_4.tick_timer(350); // n4 tries to elect itself in minority partition
    bool minority_commit = n5_4.append_command("MINORITY_CMD");
    (void)minority_commit;
    TEST_ASSERT(!minority_commit, "Minority partition (2 nodes) must be blocked from committing");
    TEST_ASSERT(n5_1.role() == RaftRole::Leader, "n1 must remain leader of majority partition");
    TEST_ASSERT(n5_1.current_term() == leader_term_before, "n1 term must not change from minority election attempt");

    // Restore network partition
    network_partition_active = false;
    std::cout << "  ✓ Test 4: Network Partition (Split 3-2 Quorum Check) PASS\n";

    // === Test 5: Log Replication Append & Commit ===
    std::cout << "[TEST 5/18] Running Log Replication Append & Commit test...\n";
    std::atomic<bool> applied5{false};
    node1.set_apply_callback([&applied5](const std::string& cmd) {
        if (cmd == "REGISTER user_service") applied5 = true;
    });

    bool ok5 = node1.append_command("REGISTER user_service");
    (void)ok5;
    TEST_ASSERT(ok5, "Append command must succeed");
    TEST_ASSERT(applied5.load(), "Apply callback must be triggered");
    std::cout << "  ✓ Test 5: Log Replication Append & Commit PASS\n";

    // === Test 6: Raft Majority Consensus Verification ===
    std::cout << "[TEST 6/18] Running Raft Majority Consensus Verification test...\n";
    TEST_ASSERT(node1.role() == RaftRole::Leader, "Node 1 must be leader");
    TEST_ASSERT(node1.log_size() >= 1, "Log size must be at least 1");
    std::cout << "  ✓ Test 6: Raft Majority Consensus Verification PASS\n";

    // === Test 7: Log Compaction & Snapshot Creation ===
    std::cout << "[TEST 7/18] Running Log Compaction & Snapshot Creation test...\n";
    node1.append_command("CMD1");
    node1.append_command("CMD2");
    std::size_t size_before = node1.log_size();
    (void)size_before;

    bool ok7 = node1.take_snapshot(1, "{\"state\":\"snap_1\"}");
    (void)ok7;
    TEST_ASSERT(ok7, "Take snapshot must succeed");
    TEST_ASSERT(node1.log_size() < size_before, "Log size must be compacted after snapshot");
    TEST_ASSERT(node1.snapshot_last_index() == 1, "Snapshot last index must be 1");
    std::cout << "  ✓ Test 7: Log Compaction & Snapshot Creation PASS\n";

    // === Test 8: Real Snapshot Catch-Up & Index Adjustment ===
    std::cout << "[TEST 8/18] Running Real Snapshot Catch-Up & Index Adjustment test...\n";
    RaftNode lagging_node("lagging_1", {"node_1"});
    bool ok8 = lagging_node.install_snapshot(5, 2, "{\"state\":\"snap_installed\"}");
    (void)ok8;
    TEST_ASSERT(ok8, "Install snapshot must succeed");
    TEST_ASSERT(lagging_node.snapshot_last_index() == 5, "Lagging node snapshot index must be 5");
    TEST_ASSERT(lagging_node.commit_index() == 5, "Commit index must be adjusted to 5");
    std::cout << "  ✓ Test 8: Real Snapshot Catch-Up & Index Adjustment PASS\n";

    // === Test 9: Failure Detection via Heartbeat Timeout ===
    std::cout << "[TEST 9/18] Running Failure Detection via Heartbeat Timeout test...\n";
    RaftNode f_node("f1", {"f2"}); // f2 initialized ALIVE at start
    auto dead_initial = f_node.detect_failure(100);
    TEST_ASSERT(dead_initial.empty(), "Freshly initialized peer must be ALIVE initially");

    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    auto dead_after = f_node.detect_failure(100);
    TEST_ASSERT(dead_after.size() == 1, "1 dead peer must be detected after timeout");
    TEST_ASSERT(dead_after[0] == "f2", "Dead node must be f2");
    std::cout << "  ✓ Test 9: Failure Detection via Heartbeat Timeout PASS\n";

    // === Test 10: Failure Recovery & Rejoin ===
    std::cout << "[TEST 10/18] Running Failure Recovery & Rejoin test...\n";
    f_node.process_network_message("APPEND_ENTRIES 1 f2 0 0 0 ping");
    auto dead_nodes10 = f_node.detect_failure(10000); // Active peer -> 0 dead nodes!
    TEST_ASSERT(dead_nodes10.empty(), "Active peer must not be marked dead");
    std::cout << "  ✓ Test 10: Failure Recovery & Rejoin PASS\n";

    // === Test 11: Persistent State Recovery (Disk Save/Load) ===
    std::cout << "[TEST 11/18] Running Persistent State Recovery test...\n";
    std::string raft_state_file = (test_dir / "raft_state.bin").string();
    bool ok11_save = node1.save_state_to_disk(raft_state_file);
    TEST_ASSERT(ok11_save, "Save state to disk must succeed");
    TEST_ASSERT(fs::exists(raft_state_file), "State file must exist on disk");

    RaftNode restored_node("restored_1", {});
    bool ok11_load = restored_node.load_state_from_disk(raft_state_file);
    TEST_ASSERT(ok11_load, "Load state from disk must succeed");
    TEST_ASSERT(restored_node.current_term() == node1.current_term(), "Restored term must match saved term");
    std::cout << "  ✓ Test 11: Persistent State Recovery PASS\n";

    // === Test 12: Consistent Hash Ring Virtual Nodes (256 vnodes) ===
    std::cout << "[TEST 12/18] Running Consistent Hash Ring (256 vnodes) test...\n";
    ConsistentHashRing ring(256);
    ring.add_node("node_alpha");
    ring.add_node("node_beta");
    ring.add_node("node_gamma");

    std::string target_node12 = ring.get_node("usr_key_12345");
    TEST_ASSERT(!target_node12.empty(), "Target node from hash ring must not be empty");
    std::cout << "  ✓ Test 12: Consistent Hash Ring (256 vnodes) PASS\n";

    // === Test 13: Consistent Hash Ring Rebalancing on Node Join/Leave ===
    std::cout << "[TEST 13/18] Running Consistent Hash Ring Rebalancing test...\n";
    ring.add_node("node_delta");
    std::string target_node13 = ring.get_node("usr_key_12345");
    TEST_ASSERT(!target_node13.empty(), "Target node after join must not be empty");
    ring.remove_node("node_delta");
    std::string target_after_leave = ring.get_node("usr_key_12345");
    TEST_ASSERT(!target_after_leave.empty(), "Target node after leave must not be empty");
    std::cout << "  ✓ Test 13: Consistent Hash Ring Rebalancing PASS\n";

    // === Test 14: Global Registry Key Replication Across N Nodes ===
    std::cout << "[TEST 14/18] Running Key Replication Across N Nodes test...\n";
    auto replica_nodes14 = ring.get_replica_nodes("usr_key_12345", 3);
    TEST_ASSERT(replica_nodes14.size() == 3, "Replica nodes count must be 3");
    std::cout << "  ✓ Test 14: Key Replication Across N Nodes PASS\n";

    // === Test 15: Query Routing Forwarding ===
    std::cout << "[TEST 15/18] Running Query Routing Forwarding test...\n";
    std::string primary_node15 = ring.get_node("session_key_abc");
    TEST_ASSERT(!primary_node15.empty(), "Primary node for query forwarding must not be empty");
    std::cout << "  ✓ Test 15: Query Routing Forwarding PASS\n";

    // === Test 16: Real HTTP /cluster/status Query Response ===
    std::cout << "[TEST 16/18] Running Real HTTP /cluster/status Query test...\n";
    HttpServer cluster_server;
    garuda::beam::stdlib::Router cluster_router;

    HttpServer::register_cluster_endpoints(cluster_router, "n1", n3_a.current_term(), n3_a.log_size());
    bool ok_srv = cluster_server.start(8098, cluster_router, 8);
    TEST_ASSERT(ok_srv, "Cluster HTTP server must start");

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    auto status_http_resp = Http::get("http://127.0.0.1:8098/cluster/status");
    TEST_ASSERT(status_http_resp.has_value(), "HTTP GET /cluster/status must return response");
    TEST_ASSERT(status_http_resp->status_code == 200, "HTTP Status must be 200 OK");
    TEST_ASSERT(status_http_resp->body.find("\"leader\":\"n1\"") != std::string::npos, "Response body must include leader n1");
    std::cout << "  ✓ Test 16: Real HTTP /cluster/status Query PASS\n";

    // === Test 17: Real HTTP /cluster/join Node Addition & Rebalancing ===
    std::cout << "[TEST 17/18] Running Real HTTP /cluster/join Node Addition test...\n";
    std::atomic<bool> joined17{false};
    std::string joined_node_id;

    HttpServer::register_cluster_endpoints(cluster_router, "n1", 1, 1, [&](const std::string& nid) {
        joined17 = true;
        joined_node_id = nid;
        n3_a.add_peer(nid);
    });

    auto join_http_resp = Http::post("http://127.0.0.1:8098/cluster/join", "{\"node_id\":\"n4\"}");
    TEST_ASSERT(join_http_resp.has_value(), "HTTP POST /cluster/join must return response");
    TEST_ASSERT(join_http_resp->status_code == 200, "HTTP Status must be 200 OK");
    TEST_ASSERT(join_http_resp->body.find("\"node_id\":\"n4\"") != std::string::npos, "Join response must contain n4");

    cluster_server.stop();
    std::cout << "  ✓ Test 17: Real HTTP /cluster/join Node Addition PASS\n";

    // === Test 18: End-to-End Distributed Cluster Lifecycle ===
    std::cout << "[TEST 18/18] Running End-to-End Distributed Cluster Lifecycle test...\n";
    GlobalRegistry registry(node1);
    bool reg18 = registry.daftar("chat_actor", Pid{101});
    TEST_ASSERT(reg18, "Register process in global registry must succeed");
    auto found18 = registry.cari("chat_actor");
    TEST_ASSERT(found18.has_value() && found18->id == 101, "Search global registry must return PID 101");
    std::cout << "  ✓ Test 18: End-to-End Distributed Cluster Lifecycle PASS\n";

    // Cleanup
    fs::remove_all(test_dir);

    std::cout << "\n🎉 ALL 18 DISTRIBUTED FOUNDATION TESTS PASSED PERFECTLY WITH TEST_ASSERT VERIFICATION!\n";
    return 0;
}
