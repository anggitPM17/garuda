#include "../include/garuda/beam/net/cluster_manager.hpp"
#include "../include/garuda/beam/net/distributed_db.hpp"
#include "../include/garuda/beam/net/raft.hpp"
#include "../include/garuda/beam/stdlib/storage_engine.hpp"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <thread>
#include <vector>

using namespace garuda::beam;
using namespace garuda::beam::stdlib;
namespace fs = std::filesystem;

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA CLUSTER MANAGER BENCHMARK (SPRINT F.2 — REAL PERFORMANCE)\n";
    std::cout << "=======================================================\n\n";

    fs::path bench_dir = fs::temp_directory_path() / "garuda_bench_cluster_mgr";
    fs::remove_all(bench_dir);
    fs::create_directories(bench_dir);

    // Setup 3-Node Cluster
    RaftNode raft_a("n1", {"n2", "n3"});
    RaftNode raft_b("n2", {"n1", "n3"});
    RaftNode raft_c("n3", {"n1", "n2"});

    auto net = [&](const std::string& target, const std::string& msg) {
        auto start_spin = std::chrono::high_resolution_clock::now();
        while (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start_spin).count() < 10) {}

        if (target == "n1") raft_a.process_network_message(msg);
        else if (target == "n2") raft_b.process_network_message(msg);
        else if (target == "n3") raft_c.process_network_message(msg);
    };

    raft_a.set_send_callback(net);
    raft_b.set_send_callback(net);
    raft_c.set_send_callback(net);

    raft_a.tick_timer(350); // n1 becomes Leader
    assert(raft_a.role() == RaftRole::Leader);

    StorageEngine store_a((bench_dir / "n1").string());
    DistributedDatabase db_a("n1", raft_a, store_a);
    ClusterManager cm1("n1", raft_a, db_a);

    // Benchmark 1: Cluster Health Checking Latency (p50 & p99)
    std::size_t sample_health = 1000;
    std::cout << "[BENCH 1] Executing " << sample_health << " Cluster Health Check Operations...\n";

    std::vector<double> health_latencies_us;
    health_latencies_us.reserve(sample_health);

    auto start_health = std::chrono::high_resolution_clock::now();

    for (std::size_t i = 0; i < sample_health; ++i) {
        double lat = cm1.check_cluster_health(100, 300);
        health_latencies_us.push_back(lat);
    }

    auto elapsed_health = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now() - start_health).count();

    std::sort(health_latencies_us.begin(), health_latencies_us.end());
    double health_p50 = health_latencies_us[static_cast<std::size_t>(health_latencies_us.size() * 0.50)];
    double health_p99 = health_latencies_us[static_cast<std::size_t>(health_latencies_us.size() * 0.99)];

    std::cout << "  ⏱️ " << sample_health << " Health Check Checks completed in " << elapsed_health << " ms\n";
    std::cout << "  ⏱️ Measured Health Check Latency p50: " << health_p50 << " µs, p99: " << health_p99 << " µs\n\n";

    // Benchmark 2: Load Balancing Distribution (1,000 Requests -> 3 Nodes)
    std::size_t sample_lb = 1000;
    std::cout << "[BENCH 2] Executing " << sample_lb << " Load Balancing Selections across 3 Nodes...\n";

    std::map<std::string, int> lb_counts;
    auto start_lb = std::chrono::high_resolution_clock::now();

    for (std::size_t i = 0; i < sample_lb; ++i) {
        std::string sel = cm1.round_robin_select();
        lb_counts[sel]++;
    }

    auto elapsed_lb = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now() - start_lb).count();

    std::cout << "  ⏱️ " << sample_lb << " Load Balancing Requests completed in " << elapsed_lb << " ms\n";
    for (const auto& [node_id, count] : lb_counts) {
        double pct = (100.0 * count) / sample_lb;
        std::cout << "  📊 Node " << node_id << ": " << count << " requests (" << pct << "%)\n";
    }
    std::cout << "\n";

    // Benchmark 3: Scale Up Duration (Node Join + Shard Rebalancing)
    std::cout << "[BENCH 3] Executing Scale Up Operation (Adding Node n4)...\n";
    auto start_scale_up = std::chrono::high_resolution_clock::now();

    cm1.register_peer("n4");

    auto elapsed_scale_up = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now() - start_scale_up).count();

    std::cout << "  ⏱️ Scale Up Operation completed in " << (elapsed_scale_up / 1000.0) << " ms\n\n";

    // Benchmark 4: Scale Down Duration (Node Leave + Shard Rebalancing)
    std::cout << "[BENCH 4] Executing Scale Down Operation (Removing Node n4)...\n";
    auto start_scale_down = std::chrono::high_resolution_clock::now();

    cm1.unregister_peer("n4");

    auto elapsed_scale_down = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now() - start_scale_down).count();

    std::cout << "  ⏱️ Scale Down Operation completed in " << (elapsed_scale_down / 1000.0) << " ms\n\n";

    fs::remove_all(bench_dir);
    std::cout << "🎉 ALL CLUSTER MANAGER BENCHMARKS EXECUTED SUCCESSFULLY!\n";
    return 0;
}
