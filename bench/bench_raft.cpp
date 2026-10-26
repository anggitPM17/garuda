#include "../include/garuda/beam/net/raft.hpp"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

using namespace garuda::beam;

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA RAFT CONSENSUS BENCHMARK (SPRINT F.0 — 3-NODE CLUSTER)\n";
    std::cout << "=======================================================\n\n";

    // Setup 3-Node Cluster
    RaftNode node_a("n1", {"n2", "n3"});
    RaftNode node_b("n2", {"n1", "n3"});
    RaftNode node_c("n3", {"n1", "n2"});

    auto net = [&](const std::string& target, const std::string& msg) {
        // High-precision 10µs CPU spin wait for simulated network latency
        auto start_spin = std::chrono::high_resolution_clock::now();
        while (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start_spin).count() < 10) {}

        if (target == "n1") node_a.process_network_message(msg);
        else if (target == "n2") node_b.process_network_message(msg);
        else if (target == "n3") node_c.process_network_message(msg);
    };

    node_a.set_send_callback(net);
    node_b.set_send_callback(net);
    node_c.set_send_callback(net);

    // Benchmark 1: Leader Election Time (< 100 ms)
    std::cout << "[BENCH 1] Measuring Leader Election Time across 3-Node Cluster...\n";
    auto start_elect = std::chrono::high_resolution_clock::now();

    node_a.tick_timer(350); // Trigger election

    auto elapsed_elect = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now() - start_elect).count();

    assert(node_a.role() == RaftRole::Leader);
    std::cout << "  ⏱️ 3-Node Leader Election completed in " << (elapsed_elect / 1000.0) << " ms\n\n";

    // Benchmark 2: 3-Node Consensus Throughput with 10µs High-Precision Latency
    std::size_t sample_ops = 1000;
    std::cout << "[BENCH 2] Appending " << sample_ops << " Replicated Raft Log Entries across 3 Nodes (10µs simulated delay)...\n";

    std::vector<double> latencies_us;
    latencies_us.reserve(sample_ops);

    auto start_append = std::chrono::high_resolution_clock::now();

    for (std::size_t i = 0; i < sample_ops; ++i) {
        auto t0 = std::chrono::high_resolution_clock::now();
        node_a.append_command("CMD_" + std::to_string(i));
        auto t1 = std::chrono::high_resolution_clock::now();
        latencies_us.push_back(std::chrono::duration<double, std::micro>(t1 - t0).count());
    }

    auto elapsed_append = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now() - start_append).count();

    std::sort(latencies_us.begin(), latencies_us.end());
    double p50_lat_us = latencies_us[static_cast<std::size_t>(latencies_us.size() * 0.50)];
    double p99_lat_us = latencies_us[static_cast<std::size_t>(latencies_us.size() * 0.99)];
    double ops_per_sec = sample_ops / (elapsed_append > 0 ? (elapsed_append / 1000.0) : 0.001);

    std::cout << "  ⏱️ " << sample_ops << " Replicated Appends completed in " << elapsed_append << " ms ("
              << ops_per_sec << " ops/sec)\n";
    std::cout << "  ⏱️ Measured Latency p50: " << p50_lat_us << " µs, p99: " << p99_lat_us << " µs\n\n";

    // Benchmark 3: Snapshot Creation & Log Compaction Speed
    std::cout << "[BENCH 3] Executing Log Compaction & Snapshot Creation for 250 Entries...\n";
    auto start_snap = std::chrono::high_resolution_clock::now();

    node_a.take_snapshot(250, "{\"snapshot_data\":\"compressed_state\"}");

    auto elapsed_snap = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now() - start_snap).count();

    std::cout << "  ⏱️ Snapshot & Log Compaction completed in " << (elapsed_snap / 1000.0) << " ms\n\n";

    // Benchmark 4: Snapshot Install Duration (Lagging Node Catch-Up)
    std::cout << "[BENCH 4] Installing Snapshot on Lagging Follower Node...\n";
    auto start_install = std::chrono::high_resolution_clock::now();

    RaftNode lagging_follower("n_lagging", {"n1"});
    lagging_follower.install_snapshot(250, 1, node_a.snapshot_bytes());

    auto elapsed_install = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now() - start_install).count();

    std::cout << "  ⏱️ Snapshot Installation completed in " << (elapsed_install / 1000.0) << " ms\n\n";

    std::cout << "🎉 ALL 3-NODE CLUSTER RAFT CONSENSUS BENCHMARKS EXECUTED SUCCESSFULLY!\n";
    return 0;
}
