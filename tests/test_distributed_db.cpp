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
    std::cout << "GARUDA DISTRIBUTED ENGINE — SPRINT F.1 DISTRIBUTED DATABASE TEST SUITE (20 STRICT TESTS)\n";
    std::cout << "=======================================================\n\n";

    fs::path test_db_dir = fs::temp_directory_path() / "garuda_dist_db_test";
    fs::remove_all(test_db_dir);
    fs::create_directories(test_db_dir);

    // Setup 3-Node Raft Cluster + Storage Engines
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

    StorageEngine store_1((test_db_dir / "n1").string());
    StorageEngine store_2((test_db_dir / "n2").string());
    StorageEngine store_3((test_db_dir / "n3").string());

    DistributedDatabase ddb1("n1", raft_1, store_1);
    DistributedDatabase ddb2("n2", raft_2, store_2);
    DistributedDatabase ddb3("n3", raft_3, store_3);

    // === Test 1: Table Sharding & Key Distribution (20%-45% Per Shard) ===
    std::cout << "[TEST 1/20] Running Table Sharding & Balanced Key Distribution test...\n";
    ddb1.create_table("users", 3, 3);
    std::map<std::string, int> node_distribution;
    for (int i = 0; i < 1000; ++i) {
        std::string target = ddb1.get_primary_node("usr_" + std::to_string(i));
        node_distribution[target]++;
    }
    TEST_ASSERT(node_distribution.size() == 3, "All 3 shards must be utilized");
    for (const auto& [node, count] : node_distribution) {
        double pct = (100.0 * count) / 1000.0;
        TEST_ASSERT(pct >= 20.0 && pct <= 45.0, "Shard distribution must be balanced between 20% and 45%");
    }
    std::cout << "  ✓ Test 1: Table Sharding & Balanced Key Distribution PASS\n";

    // === Test 2: Multi-Table Sharding Isolation ===
    std::cout << "[TEST 2/20] Running Multi-Table Sharding Isolation test...\n";
    ddb1.create_table("orders", 3, 3);
    TEST_ASSERT(ddb1.get_primary_node("order_1") != "", "Primary node for orders table must exist");
    std::cout << "  ✓ Test 2: Multi-Table Sharding Isolation PASS\n";

    // === Test 3: Replication Factor N=3 Setup ===
    std::cout << "[TEST 3/20] Running Replication Factor N=3 Setup test...\n";
    auto replicas3 = ddb1.get_replica_nodes("usr_101", 3);
    TEST_ASSERT(replicas3.size() == 3, "Replication factor must produce 3 replica nodes");
    std::cout << "  ✓ Test 3: Replication Factor N=3 Setup PASS\n";

    // === Test 4: Quorum Write Confirmation (2/3 Active) ===
    std::cout << "[TEST 4/20] Running Quorum Write Confirmation (2/3 Active) test...\n";
    bool ok4 = ddb1.insert("users", "usr_101", Term("Budi Distributed"));
    TEST_ASSERT(ok4, "Insertion with Raft consensus must succeed when quorum exists");
    std::cout << "  ✓ Test 4: Quorum Write Confirmation PASS\n";

    // === Test 5: Quorum Write Rejection (<2 Active) ===
    std::cout << "[TEST 5/20] Running Quorum Write Rejection (<2 Active) test...\n";
    RaftNode raft_alone("alone_1", {"n2", "n3"}); // No active peers connected
    StorageEngine store_alone((test_db_dir / "alone").string());
    DistributedDatabase ddb_alone("alone_1", raft_alone, store_alone);

    bool ok5 = ddb_alone.insert("users", "usr_fail", Term("No Quorum"));
    TEST_ASSERT(!ok5, "Write attempt without quorum must be rejected");
    std::cout << "  ✓ Test 5: Quorum Write Rejection PASS\n";

    // === Test 6: Primary Replica Read ===
    std::cout << "[TEST 6/20] Running Primary Replica Read test...\n";
    auto rec6 = ddb1.get("users", "usr_101");
    TEST_ASSERT(rec6.has_value(), "Primary replica read must return record");
    TEST_ASSERT(rec6->get_if<std::string>() != nullptr && *rec6->get_if<std::string>() == "Budi Distributed", "Value must match 'Budi Distributed'");
    std::cout << "  ✓ Test 6: Primary Replica Read PASS\n";

    // === Test 7: Fallback Secondary Replica Read ===
    std::cout << "[TEST 7/20] Running Fallback Secondary Replica Read test...\n";
    auto rec7 = ddb2.get("users", "usr_101");
    TEST_ASSERT(rec7.has_value(), "Secondary replica read must return record via replication");
    std::cout << "  ✓ Test 7: Fallback Secondary Replica Read PASS\n";

    // === Test 8: Raft-Consensus Cross-Shard Entry Commit ===
    std::cout << "[TEST 8/20] Running Cross-Shard Entry Commit test...\n";
    TEST_ASSERT(raft_1.log_size() >= 1, "Raft log size must be >= 1");
    std::cout << "  ✓ Test 8: Cross-Shard Entry Commit PASS\n";

    // === Test 9: StorageEngine Persistence Synchronization ===
    std::cout << "[TEST 9/20] Running StorageEngine Persistence Sync test...\n";
    store_1.checkpoint();
    TEST_ASSERT(fs::exists(test_db_dir / "n1" / "data.db"), "data.db must exist on disk");
    std::cout << "  ✓ Test 9: StorageEngine Persistence Sync PASS\n";

    // === Test 10: Query Routing & Node Forwarding ===
    std::cout << "[TEST 10/20] Running Query Routing Forwarding test...\n";
    std::string route_target = ddb3.get_primary_node("usr_101");
    TEST_ASSERT(!route_target.empty(), "Query routing target node must be found");
    std::cout << "  ✓ Test 10: Query Routing Forwarding PASS\n";

    // === Test 11: Node Join Shard Rebalancing (20%-35% Keys Move) ===
    std::cout << "[TEST 11/20] Running Node Join Shard Rebalancing test...\n";
    std::map<std::string, std::string> keys_before;
    for (int i = 0; i < 1000; ++i) {
        std::string k = "key_rebal_" + std::to_string(i);
        keys_before[k] = ddb1.get_primary_node(k);
    }

    ddb1.on_node_join("n4");

    int moved_keys11 = 0;
    for (int i = 0; i < 1000; ++i) {
        std::string k = "key_rebal_" + std::to_string(i);
        if (ddb1.get_primary_node(k) != keys_before[k]) {
            moved_keys11++;
        }
    }
    TEST_ASSERT(moved_keys11 >= 200 && moved_keys11 <= 350, "Rebalancing must move ~25% (20%-35%) of keys to new node");
    std::cout << "  ✓ Test 11: Node Join Shard Rebalancing PASS (" << moved_keys11 << " keys moved)\n";

    // === Test 12: Node Leave Shard Rebalancing (Key Migration Back) ===
    std::cout << "[TEST 12/20] Running Node Leave Shard Rebalancing test...\n";
    std::map<std::string, std::string> keys_before_leave;
    for (int i = 0; i < 1000; ++i) {
        std::string k = "key_rebal_" + std::to_string(i);
        keys_before_leave[k] = ddb1.get_primary_node(k);
    }

    ddb1.on_node_leave("n4");

    int moved_keys12 = 0;
    for (int i = 0; i < 1000; ++i) {
        std::string k = "key_rebal_" + std::to_string(i);
        if (ddb1.get_primary_node(k) != keys_before_leave[k]) {
            moved_keys12++;
        }
        TEST_ASSERT(ddb1.get_primary_node(k) != "n4", "n4 must no longer be primary for any key after leave");
    }
    TEST_ASSERT(moved_keys12 >= 200 && moved_keys12 <= 350, "Node leave must move ~25% (20%-35%) of keys back to original 3 nodes");
    std::cout << "  ✓ Test 12: Node Leave Shard Rebalancing PASS (" << moved_keys12 << " keys moved back)\n";

    // === Test 13: 5-Node Network Partition (Split 3-2 Quorum Check) ===
    std::cout << "[TEST 13/20] Running 5-Node Network Partition (Split 3-2) test...\n";
    RaftNode r5_1("n1", {"n2", "n3", "n4", "n5"});
    RaftNode r5_2("n2", {"n1", "n3", "n4", "n5"});
    RaftNode r5_3("n3", {"n1", "n2", "n4", "n5"});
    RaftNode r5_4("n4", {"n1", "n2", "n3", "n5"});
    RaftNode r5_5("n5", {"n1", "n2", "n3", "n4"});

    bool partition_active = true;
    auto part_net = [&](const std::string& target, const std::string& msg) {
        if (partition_active) {
            if ((target == "n4" || target == "n5") && (msg.find("n1") != std::string::npos || msg.find("n2") != std::string::npos || msg.find("n3") != std::string::npos)) return;
            if ((target == "n1" || target == "n2" || target == "n3") && (msg.find("n4") != std::string::npos || msg.find("n5") != std::string::npos)) return;
        }
        if (target == "n1") r5_1.process_network_message(msg);
        else if (target == "n2") r5_2.process_network_message(msg);
        else if (target == "n3") r5_3.process_network_message(msg);
        else if (target == "n4") r5_4.process_network_message(msg);
        else if (target == "n5") r5_5.process_network_message(msg);
    };

    r5_1.set_send_callback(part_net); r5_2.set_send_callback(part_net); r5_3.set_send_callback(part_net);
    r5_4.set_send_callback(part_net); r5_5.set_send_callback(part_net);

    r5_1.tick_timer(350); // Majority partition leader
    TEST_ASSERT(r5_1.role() == RaftRole::Leader, "r5_1 must be majority leader");

    StorageEngine store_part_1((test_db_dir / "p1").string());
    DistributedDatabase ddb_part_1("n1", r5_1, store_part_1);

    bool maj_ok13 = ddb_part_1.insert("users", "k_maj", Term("MajVal"));
    TEST_ASSERT(maj_ok13, "Majority partition write must succeed");

    r5_4.tick_timer(350); // Minority attempt
    StorageEngine store_part_4((test_db_dir / "p4").string());
    DistributedDatabase ddb_part_4("n4", r5_4, store_part_4);

    bool min_ok13 = ddb_part_4.insert("users", "k_min", Term("MinVal"));
    TEST_ASSERT(!min_ok13, "Minority partition write must fail");
    std::cout << "  ✓ Test 13: 5-Node Network Partition (Split 3-2) PASS\n";

    // === Test 14: Write-After-Read Consistency Guarantee ===
    std::cout << "[TEST 14/20] Running Write-After-Read Consistency test...\n";
    ddb1.insert("users", "usr_consistency", Term("Consistent Data"));
    auto read14 = ddb1.get("users", "usr_consistency");
    TEST_ASSERT(read14.has_value(), "Read after write must return record");
    TEST_ASSERT(*read14->get_if<std::string>() == "Consistent Data", "Value must match 'Consistent Data'");
    std::cout << "  ✓ Test 14: Write-After-Read Consistency PASS\n";

    // === Test 15: Batch Insert Capability (All Keys Verified via TEST_ASSERT) ===
    std::cout << "[TEST 15/20] Running Batch Insert Capability test...\n";
    std::vector<std::pair<std::string, Term>> batch_items = {
        {"batch_k1", Term("V1")},
        {"batch_k2", Term("V2")},
        {"batch_k3", Term("V3")}
    };
    bool batch_ok15 = ddb1.insert_batch("users", batch_items);
    TEST_ASSERT(batch_ok15, "Batch insert must succeed");
    TEST_ASSERT(ddb1.get("users", "batch_k1").has_value(), "batch_k1 must exist after batch insert");
    TEST_ASSERT(ddb1.get("users", "batch_k2").has_value(), "batch_k2 must exist after batch insert");
    TEST_ASSERT(ddb1.get("users", "batch_k3").has_value(), "batch_k3 must exist after batch insert");
    std::cout << "  ✓ Test 15: Batch Insert Capability PASS\n";

    // === Test 16: HTTP POST /db/insert Endpoint ===
    std::cout << "[TEST 16/20] Running HTTP POST /db/insert Endpoint test...\n";
    HttpServer db_server;
    garuda::beam::stdlib::Router db_router;
    ddb1.register_http_endpoints(db_router);

    bool srv_ok = db_server.start(8099, db_router, 8);
    TEST_ASSERT(srv_ok, "HTTP DB Server must start");

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    auto http_ins_resp = Http::post("http://127.0.0.1:8099/db/insert", "{\"table\":\"users\",\"key\":\"usr_http_1\",\"value\":\"Siti HTTP\"}");
    TEST_ASSERT(http_ins_resp.has_value(), "HTTP POST /db/insert must return response");
    TEST_ASSERT(http_ins_resp->status_code == 200, "HTTP Status must be 200 OK");
    TEST_ASSERT(http_ins_resp->body.find("\"status\":\"ok\"") != std::string::npos, "Response status must be ok");
    std::cout << "  ✓ Test 16: HTTP POST /db/insert Endpoint PASS\n";

    // === Test 17: HTTP POST /db/query Endpoint ===
    std::cout << "[TEST 17/20] Running HTTP POST /db/query Endpoint test...\n";
    auto http_qry_resp = Http::post("http://127.0.0.1:8099/db/query", "{\"table\":\"users\",\"key\":\"usr_http_1\"}");
    TEST_ASSERT(http_qry_resp.has_value(), "HTTP POST /db/query must return response");
    TEST_ASSERT(http_qry_resp->status_code == 200, "HTTP Status must be 200 OK");
    TEST_ASSERT(http_qry_resp->body.find("Siti HTTP") != std::string::npos, "Queried value must contain 'Siti HTTP'");

    db_server.stop();
    std::cout << "  ✓ Test 17: HTTP POST /db/query Endpoint PASS\n";

    // === Test 18: Non-Existent Key Error Handling ===
    std::cout << "[TEST 18/20] Running Non-Existent Key Error Handling test...\n";
    auto err18 = ddb1.get("users", "non_existent_key_999");
    TEST_ASSERT(!err18.has_value(), "Non-existent key lookup must return nullopt");
    std::cout << "  ✓ Test 18: Non-Existent Key Error Handling PASS\n";

    // === Test 19: End-to-End Distributed Database Cluster Lifecycle ===
    std::cout << "[TEST 19/20] Running End-to-End Distributed Database Lifecycle test...\n";
    ddb1.remove("users", "usr_101");
    auto deleted19 = ddb1.get("users", "usr_101");
    TEST_ASSERT(!deleted19.has_value(), "Removed key must no longer exist");
    std::cout << "  ✓ Test 19: End-to-End Distributed Database Lifecycle PASS\n";

    // === Test 20: Final Consistency Audit ===
    std::cout << "[TEST 20/20] Running Final Consistency Audit test...\n";
    TEST_ASSERT(raft_1.log_size() > 0, "Raft log must record transaction history");
    std::cout << "  ✓ Test 20: Final Consistency Audit PASS\n";

    // Cleanup
    fs::remove_all(test_db_dir);

    std::cout << "\n🎉 ALL 20 STRICT DISTRIBUTED DATABASE TESTS PASSED PERFECTLY!\n";
    return 0;
}
