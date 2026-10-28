// =====================================================================
// FASE G6.2 - GARUDA RPC TESTS
// Memverifikasi rpc.hpp setara rpc.erl
// =====================================================================
#include "../include/garuda/beam/shell/rpc.hpp"
#include <iostream>
#include <string>
#include <cstdlib>
#include <thread>

using namespace garuda::beam;

#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

int main() {
    std::cout << "========================================\n";
    std::cout << "FASE G6.2 - GARUDA RPC TESTS\n";
    std::cout << "========================================\n\n";

    RpcServer local_srv("node1@127.0.0.1");
    RpcServer remote_srv("node2@127.0.0.1");

    RpcClient client;
    client.set_local_server(&local_srv);
    client.add_remote_server("node2@127.0.0.1", &remote_srv);

    // Register handlers
    local_srv.register_function("math", "add", [](const std::vector<std::string>& args) -> std::string {
        if (args.size() < 2) return "0";
        int a = std::stoi(args[0]);
        int b = std::stoi(args[1]);
        return std::to_string(a + b);
    });

    remote_srv.register_function("system", "uptime", [](const std::vector<std::string>&) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        return "3600_seconds";
    });

    remote_srv.register_function("string", "concat", [](const std::vector<std::string>& args) {
        std::string res;
        for (const auto& s : args) res += s;
        return res;
    });

    // Test 1: Register & Local Call
    std::cout << "[Test 1] Register & Local Call...\n";
    {
        auto resp = client.call("node1@127.0.0.1", "math", "add", {"10", "20"});
        CHECK(resp.success, "Local RPC call harus success");
        CHECK(resp.result == "30", "Result '10 + 20' harus '30'");
    }
    std::cout << "PASS\n\n";

    // Test 2: Remote Call
    std::cout << "[Test 2] Remote Call...\n";
    {
        auto resp = client.call("node2@127.0.0.1", "system", "uptime", {});
        CHECK(resp.success, "Remote RPC call harus success");
        CHECK(resp.result == "3600_seconds", "Result uptime harus '3600_seconds'");
        CHECK(resp.duration.count() >= 0, "Duration us harus >= 0");
    }
    std::cout << "PASS\n\n";

    // Test 3: Unknown Function / Module
    std::cout << "[Test 3] Unknown Function / Module...\n";
    {
        auto resp = client.call("node1@127.0.0.1", "unknown_mod", "fn", {});
        CHECK(!resp.success, "RPC ke modul tidak ada harus fail");
        CHECK(resp.error.find("badrpc: module_or_function_not_found") != std::string::npos, "Error badrpc match");

        auto resp_conn = client.call("ghost_node@127.0.0.1", "math", "add", {"1", "2"});
        CHECK(!resp_conn.success, "RPC ke node tidak ada harus fail");
        CHECK(resp_conn.error.find("badrpc: noconnection") != std::string::npos, "Error noconnection match");
    }
    std::cout << "PASS\n\n";

    // Test 4: Multiple Modules & Functions
    std::cout << "[Test 4] Multiple Modules & Functions...\n";
    {
        CHECK(remote_srv.module_count() == 2, "Remote server module count = 2 (system, string)");
        CHECK(remote_srv.function_count() == 2, "Remote server function count = 2");

        auto resp = client.call("node2@127.0.0.1", "string", "concat", {"Garuda", "BEAM"});
        CHECK(resp.success && resp.result == "GarudaBEAM", "Concat harus GarudaBEAM");
    }
    std::cout << "PASS\n\n";

    // Test 5: Client Remote Count
    std::cout << "[Test 5] Client Remote Count...\n";
    {
        CHECK(client.remote_count() == 1, "Client remote count = 1");
    }
    std::cout << "PASS\n\n";

    std::cout << "========================================\n";
    std::cout << "FASE G6.2 - RPC SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
