// =====================================================================
// FASE G2.1 - GARUDA DISTRIBUTED NODE TESTS
// Memverifikasi dist_node.hpp setara Distributed Erlang
// =====================================================================
#include "../include/garuda/beam/net/dist_node.hpp"
#include <iostream>
#include <memory>
#include <string>
#include <cstdlib>

using namespace garuda::beam;

#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

int main() {
    std::cout << "========================================\n";
    std::cout << "FASE G2.1 - GARUDA DISTRIBUTED NODE TESTS\n";
    std::cout << "========================================\n\n";

    Runtime rt;
    EventLoop loop(rt);

    // Test 1: Node Registration & Info
    std::cout << "[Test 1] Node Registration & Info...\n";
    {
        ClusterManager cm(loop, "garuda1@127.0.0.1", "secret_cookie_123");
        CHECK(cm.node_name() == "garuda1@127.0.0.1", "Node name harus garuda1@127.0.0.1");
        CHECK(cm.cookie() == "secret_cookie_123", "Cookie harus secret_cookie_123");

        cm.register_remote_node("garuda2@127.0.0.1", "127.0.0.1", 9002);
        auto visible = cm.visible_nodes();
        CHECK(visible.size() == 1, "Harus ada 1 remote node terdaftar");
        CHECK(visible[0] == "garuda2@127.0.0.1", "Remote node name harus garuda2@127.0.0.1");
    }
    std::cout << "PASS\n\n";

    // Test 2: Node Discovery (3 nodes)
    std::cout << "[Test 2] Node Discovery (3 nodes)...\n";
    {
        ClusterManager cm(loop, "main@127.0.0.1");
        cm.register_remote_node("node1@127.0.0.1", "127.0.0.1", 9001);
        cm.register_remote_node("node2@127.0.0.1", "127.0.0.1", 9002);
        cm.register_remote_node("node3@127.0.0.1", "127.0.0.1", 9003);

        auto visible = cm.visible_nodes();
        CHECK(visible.size() == 3, "Harus ada 3 node yang ditemukan");
    }
    std::cout << "PASS\n\n";

    // Test 3: Ping / Heartbeat Check
    std::cout << "[Test 3] Ping / Heartbeat Check...\n";
    {
        ClusterManager cm(loop, "main@127.0.0.1");
        cm.register_remote_node("node1@127.0.0.1", "127.0.0.1", 9001);

        CHECK(cm.ping("node1@127.0.0.1") == true, "Ping ke node1 harus true");
        CHECK(cm.ping("unknown_node@127.0.0.1") == false, "Ping ke node tak terdaftar harus false");
    }
    std::cout << "PASS\n\n";

    // Test 4: RPC Call (Local vs Remote)
    std::cout << "[Test 4] RPC Call (Local vs Remote)...\n";
    {
        ClusterManager cm(loop, "main@127.0.0.1");
        cm.register_remote_node("remote@127.0.0.1", "127.0.0.1", 9001);

        // Local RPC execution
        auto local_res = cm.rpc_call("main@127.0.0.1", "erlang", "now", {});
        CHECK(local_res.has_value(), "Local RPC harus mengembalikan result");
        CHECK(tuple_size(*local_res) == 2 && is_atom(*tuple_at(*local_res, 0), "ok"), "Local RPC harus ok");

        // Remote RPC execution fallback
        auto remote_res = cm.rpc_call("remote@127.0.0.1", "math", "add", {});
        CHECK(remote_res.has_value(), "Remote RPC harus mengembalikan result tuple");
        CHECK(is_atom(*tuple_at(*remote_res, 0), "badrpc"), "Remote RPC tanpa socket harus badrpc");

        // Unknown node RPC
        auto unknown_res = cm.rpc_call("ghost@127.0.0.1", "math", "add", {});
        CHECK(!unknown_res.has_value(), "Unknown node RPC harus std::nullopt");
    }
    std::cout << "PASS\n\n";

    // Test 5: Event Subscription
    std::cout << "[Test 5] Event Subscription...\n";
    {
        ClusterManager cm(loop, "main@127.0.0.1");
        cm.subscribe_node_events(Pid{10});
        // Verifikasi subscribe tidak crash
    }
    std::cout << "PASS\n\n";

    std::cout << "========================================\n";
    std::cout << "FASE G2.1 - DISTRIBUTED NODE SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
