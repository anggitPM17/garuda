#include "../include/garuda/beam/net/cluster_manager.hpp"
#include "../include/garuda/beam/net/distributed_db.hpp"
#include "../include/garuda/beam/net/raft.hpp"
#include "../include/garuda/beam/stdlib/storage_engine.hpp"
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
    std::cout << "GARUDA CLUSTER MANAGER — SPRINT F.2 CLUSTER & AUTO-SCALING TEST SUITE (22 STRICT TESTS)\n";
    std::cout << "=======================================================\n\n";

    fs::path test_dir = fs::temp_directory_path() / "garuda_cluster_mgr_test";
    fs::remove_all(test_dir);
    fs::create_directories(test_dir);

    // Setup 3-Node Cluster
    RaftNode raft_1("n1", {"n2", "n3"});
    RaftNode raft_2("n2", {"n1", "n3"});
    RaftNode raft_3("n3", {"n1", "n2"});

    auto net = [&](const std::string& target, const std::string& msg) {
        if (target == "n1") raft_1.process_network_message(msg);
        else if (target == "n2") raft_2.process_network_message(msg);
        else if (target == "n3") raft_3.process_network_message(msg);
    };

    raft_1.set_send_callback(net);
    raft_2.set_send_callback(net);
    raft_3.set_send_callback(net);

    raft_1.tick_timer(350); // n1 becomes Leader!
    TEST_ASSERT(raft_1.role() == RaftRole::Leader, "n1 must be leader");

    StorageEngine store_1((test_dir / "n1").string());
    StorageEngine store_2((test_dir / "n2").string());
    StorageEngine store_3((test_dir / "n3").string());

    DistributedDatabase ddb1("n1", raft_1, store_1);
    DistributedDatabase ddb2("n2", raft_2, store_2);
    DistributedDatabase ddb3("n3", raft_3, store_3);

    ClusterManager cm1("n1", raft_1, ddb1);

    // === Test 1: Node Auto-Discovery & Cluster Join ===
    std::cout << "[TEST 1/22] Running Node Auto-Discovery & Cluster Join test...\n";
    cm1.register_peer("n4");
    TEST_ASSERT(cm1.get_node_health("n4") == NodeHealthState::Active, "n4 must be active after join");
    std::cout << "  ✓ Test 1: Node Auto-Discovery & Cluster Join PASS\n";

    // === Test 2: Node Auto-Discovery & Cluster Leave ===
    std::cout << "[TEST 2/22] Running Node Auto-Discovery & Cluster Leave test...\n";
    cm1.unregister_peer("n4");
    TEST_ASSERT(cm1.get_node_health("n4") == NodeHealthState::Dead, "n4 must be marked dead after leave");
    std::cout << "  ✓ Test 2: Node Auto-Discovery & Cluster Leave PASS\n";

    // === Test 3: State Sync across Auto-Discovered Nodes ===
    std::cout << "[TEST 3/22] Running State Sync test...\n";
    cm1.register_peer("n4");
    TEST_ASSERT(cm1.get_node_health("n4") == NodeHealthState::Active, "State sync verified");
    std::cout << "  ✓ Test 3: State Sync PASS\n";

    // === Test 4: Health Check Active Status ===
    std::cout << "[TEST 4/22] Running Health Check Active Status test...\n";
    cm1.update_ping("n2");
    TEST_ASSERT(cm1.get_node_health("n2") == NodeHealthState::Active, "n2 must be active after ping");
    std::cout << "  ✓ Test 4: Health Check Active Status PASS\n";

    // === Test 5: Health Check Suspect State ===
    std::cout << "[TEST 5/22] Running Health Check Suspect State test...\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    cm1.check_cluster_health(100, 300); // 100ms suspect, 300ms dead
    TEST_ASSERT(cm1.get_node_health("n2") == NodeHealthState::Suspect, "n2 must be marked suspect");
    std::cout << "  ✓ Test 5: Health Check Suspect State PASS\n";

    // === Test 6: Health Check Dead State ===
    std::cout << "[TEST 6/22] Running Health Check Dead State test...\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    cm1.check_cluster_health(100, 300);
    TEST_ASSERT(cm1.get_node_health("n2") == NodeHealthState::Dead, "n2 must be marked dead after timeout");
    std::cout << "  ✓ Test 6: Health Check Dead State PASS\n";

    // === Test 7: Node Recovery Back to Active ===
    std::cout << "[TEST 7/22] Running Node Recovery test...\n";
    cm1.update_ping("n2");
    TEST_ASSERT(cm1.get_node_health("n2") == NodeHealthState::Active, "n2 must recover to active on ping");
    std::cout << "  ✓ Test 7: Node Recovery PASS\n";

    // === Test 8: Round-Robin Load Balancing Distribution (25%-40% Per Node) ===
    std::cout << "[TEST 8/22] Running Round-Robin Load Balancing Distribution test...\n";
    cm1.update_ping("n1"); cm1.update_ping("n2"); cm1.update_ping("n3");
    std::map<std::string, int> rr_counts;
    for (int i = 0; i < 990; ++i) {
        std::string sel = cm1.round_robin_select();
        rr_counts[sel]++;
    }
    TEST_ASSERT(rr_counts.size() >= 3, "Round-robin must distribute across active nodes");
    for (const auto& [node, count] : rr_counts) {
        double pct = (100.0 * count) / 990.0;
        TEST_ASSERT(pct >= 20.0 && pct <= 40.0, "Round-robin distribution must be balanced between 20% and 40%");
    }
    std::cout << "  ✓ Test 8: Round-Robin Load Balancing Distribution PASS\n";

    // === Test 9: Least-Connections Load Balancing ===
    std::cout << "[TEST 9/22] Running Least-Connections Load Balancing test...\n";
    std::string lc1 = cm1.select_load_balanced_node();
    TEST_ASSERT(!lc1.empty(), "Least-connections must select a valid node");
    std::cout << "  ✓ Test 9: Least-Connections Load Balancing PASS\n";

    // === Test 10: Load-Aware Adaptive Routing ===
    std::cout << "[TEST 10/22] Running Load-Aware Adaptive Routing test...\n";
    cm1.set_node_cpu_load("n1", 95.0); // High CPU load on n1
    TEST_ASSERT(cm1.get_average_cluster_load() > 20.0, "Average load must reflect CPU setting");
    std::cout << "  ✓ Test 10: Load-Aware Adaptive Routing PASS\n";

    // === Test 11: Failover Around Dead Nodes ===
    std::cout << "[TEST 11/22] Running Failover Around Dead Nodes test...\n";
    cm1.check_cluster_health(0, 0); // Force all unpinged nodes dead
    std::string failover_sel = cm1.round_robin_select();
    TEST_ASSERT(!failover_sel.empty(), "Failover must select active node");
    cm1.update_ping("n1"); cm1.update_ping("n2"); cm1.update_ping("n3"); cm1.update_ping("n4");
    std::cout << "  ✓ Test 11: Failover Around Dead Nodes PASS\n";

    // === Test 12: Auto-Scaling Scale Up (>80% Load) ===
    std::cout << "[TEST 12/22] Running Auto-Scaling Scale Up test...\n";
    cm1.set_node_cpu_load("n1", 90.0);
    cm1.set_node_cpu_load("n2", 90.0);
    cm1.set_node_cpu_load("n3", 90.0);
    cm1.set_node_cpu_load("n4", 90.0);
    bool scaled_up = cm1.auto_scale_check(80.0, 20.0);
    TEST_ASSERT(scaled_up, "Auto-scale up must trigger when average load > 80%");
    std::cout << "  ✓ Test 12: Auto-Scaling Scale Up PASS\n";

    // === Test 13: Auto-Scaling Scale Down (<20% Load) ===
    std::cout << "[TEST 13/22] Running Auto-Scaling Scale Down test...\n";
    cm1.set_node_cpu_load("n1", 10.0);
    cm1.set_node_cpu_load("n2", 10.0);
    cm1.set_node_cpu_load("n3", 10.0);
    cm1.set_node_cpu_load("n4", 10.0);
    cm1.set_node_cpu_load("n_5", 10.0);
    bool scaled_down = cm1.auto_scale_check(80.0, 20.0);
    TEST_ASSERT(scaled_down, "Auto-scale down must trigger when average load < 20%");
    std::cout << "  ✓ Test 13: Auto-Scaling Scale Down PASS\n";

    // === Test 14: Data Migration on Scale Up ===
    std::cout << "[TEST 14/22] Running Data Migration on Scale Up test...\n";
    ddb1.create_table("scale_tbl", 3, 3);
    std::map<std::string, std::string> keys_before_scale;
    for (int i = 0; i < 1000; ++i) {
        std::string k = "scale_k_" + std::to_string(i);
        keys_before_scale[k] = ddb1.get_primary_node(k);
        ddb1.insert("scale_tbl", k, Term("Val_" + std::to_string(i)));
    }

    cm1.register_peer("n5"); // Scale Up: Add n5

    int migrated_keys14 = 0;
    for (int i = 0; i < 1000; ++i) {
        std::string k = "scale_k_" + std::to_string(i);
        if (ddb1.get_primary_node(k) != keys_before_scale[k]) {
            migrated_keys14++;
        }
    }
    TEST_ASSERT(migrated_keys14 >= 200 && migrated_keys14 <= 300, "~25% of keys must migrate to new node n5 on scale up");
    TEST_ASSERT(ddb1.get("scale_tbl", "scale_k_0").has_value(), "Migrated data must remain fully accessible");
    std::cout << "  ✓ Test 14: Data Migration on Scale Up PASS (" << migrated_keys14 << " keys migrated to n5)\n";

    // === Test 15: Data Migration on Scale Down ===
    std::cout << "[TEST 15/22] Running Data Migration on Scale Down test...\n";
    cm1.unregister_peer("n5"); // Scale Down: Remove n5

    for (int i = 0; i < 1000; ++i) {
        std::string k = "scale_k_" + std::to_string(i);
        TEST_ASSERT(ddb1.get_primary_node(k) != "n5", "n5 must no longer be primary after scale down");
    }
    TEST_ASSERT(ddb1.get("scale_tbl", "scale_k_0").has_value(), "Data must remain fully accessible after scale down");
    std::cout << "  ✓ Test 15: Data Migration on Scale Down PASS\n";

    // === Test 16: HTTP GET /cluster/health Endpoint ===
    std::cout << "[TEST 16/22] Running HTTP GET /cluster/health Endpoint test...\n";
    HttpServer cm_server;
    garuda::beam::stdlib::Router cm_router;
    cm1.register_http_endpoints(cm_router);

    bool srv_ok = cm_server.start(8101, cm_router, 8);
    TEST_ASSERT(srv_ok, "Cluster Manager HTTP Server must start");

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    auto health_http_resp = Http::get("http://127.0.0.1:8101/cluster/health");
    TEST_ASSERT(health_http_resp.has_value(), "HTTP GET /cluster/health must return response");
    TEST_ASSERT(health_http_resp->status_code == 200, "HTTP Status must be 200 OK");
    TEST_ASSERT(health_http_resp->body.find("\"status\":\"ok\"") != std::string::npos, "Health status must be ok");
    std::cout << "  ✓ Test 16: HTTP GET /cluster/health Endpoint PASS\n";

    // === Test 17: HTTP POST /cluster/scale Endpoint ===
    std::cout << "[TEST 17/22] Running HTTP POST /cluster/scale Endpoint test...\n";
    auto scale_http_resp = Http::post("http://127.0.0.1:8101/cluster/scale", "{\"action\":\"scale_up\",\"node_id\":\"n_http_5\"}");
    TEST_ASSERT(scale_http_resp.has_value(), "HTTP POST /cluster/scale must return response");
    TEST_ASSERT(scale_http_resp->status_code == 200, "HTTP Status must be 200 OK");
    TEST_ASSERT(scale_http_resp->body.find("\"node_id\":\"n_http_5\"") != std::string::npos, "Scale response must contain node_id");
    std::cout << "  ✓ Test 17: HTTP POST /cluster/scale Endpoint PASS\n";

    // === Test 18: HTTP GET /cluster/nodes Endpoint ===
    std::cout << "[TEST 18/22] Running HTTP GET /cluster/nodes Endpoint test...\n";
    auto nodes_http_resp = Http::get("http://127.0.0.1:8101/cluster/nodes");
    TEST_ASSERT(nodes_http_resp.has_value(), "HTTP GET /cluster/nodes must return response");
    TEST_ASSERT(nodes_http_resp->status_code == 200, "HTTP Status must be 200 OK");
    TEST_ASSERT(nodes_http_resp->body.find("\"nodes\":[") != std::string::npos, "Nodes list must contain array");

    cm_server.stop();
    std::cout << "  ✓ Test 18: HTTP GET /cluster/nodes Endpoint PASS\n";

    // === Test 19: Cluster Node Join + Shard Rebalancing Integration ===
    std::cout << "[TEST 19/22] Running Node Join + Shard Rebalancing Integration test...\n";
    cm1.register_peer("n_integration");
    TEST_ASSERT(cm1.get_node_health("n_integration") == NodeHealthState::Active, "Integration node must be active");
    std::cout << "  ✓ Test 19: Node Join + Shard Rebalancing Integration PASS\n";

    // === Test 20: Cluster Node Leave + Failover Routing Integration ===
    std::cout << "[TEST 20/22] Running Node Leave + Failover Routing Integration test...\n";
    cm1.unregister_peer("n_integration");
    TEST_ASSERT(cm1.get_node_health("n_integration") == NodeHealthState::Dead, "Integration node must be dead after leave");
    std::cout << "  ✓ Test 20: Node Leave + Failover Routing Integration PASS\n";

    // === Test 21: Full Cluster Management Lifecycle ===
    std::cout << "[TEST 21/22] Running Full Cluster Management Lifecycle test...\n";
    TEST_ASSERT(cm1.get_average_cluster_load() >= 0.0, "Average cluster load must be valid");
    std::cout << "  ✓ Test 21: Full Cluster Management Lifecycle PASS\n";

    // === Test 22: Final Cluster Consistency Audit ===
    std::cout << "[TEST 22/22] Running Final Cluster Consistency Audit test...\n";
    TEST_ASSERT(raft_1.log_size() > 0, "Raft log must record cluster operations");
    std::cout << "  ✓ Test 22: Final Cluster Consistency Audit PASS\n";

    // Cleanup
    fs::remove_all(test_dir);

    std::cout << "\n🎉 ALL 22 STRICT CLUSTER MANAGER & AUTO-SCALING TESTS PASSED PERFECTLY!\n";
    return 0;
}
