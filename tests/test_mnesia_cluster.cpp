// =====================================================================
// FASE G3.4 - GARUDA MNESIA CLUSTER TESTS
// Memverifikasi mnesia_cluster.hpp
// =====================================================================
#include "../include/garuda/beam/otp/mnesia_cluster.hpp"
#include <iostream>
#include <filesystem>
#include <cstdlib>
#include <system_error>

using namespace garuda::beam;

#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

int main() {
    std::cout << "========================================\n";
    std::cout << "FASE G3.4 - GARUDA MNESIA CLUSTER TESTS\n";
    std::cout << "========================================\n\n";

    std::string test_dir = "./test_mnesia_cluster_db";
    std::error_code ec;
    std::filesystem::remove_all(test_dir, ec);

    {
        RaftNode raft("node1", {});
        MnesiaCluster cluster("node1", raft, test_dir);

        // Test 1: Node Join
        std::cout << "[Test 1] Node Join...\n";
        {
            cluster.join_cluster_node("node2");
            cluster.join_cluster_node("node3");

            auto active = cluster.active_nodes();
            CHECK(active.size() == 3, "Harus ada 3 active cluster nodes");
        }
        std::cout << "PASS\n\n";

        // Test 2: Node Leave
        std::cout << "[Test 2] Node Leave...\n";
        {
            cluster.leave_cluster_node("node3");
            auto active = cluster.active_nodes();
            CHECK(active.size() == 2, "Active cluster nodes harus menjadi 2");
        }
        std::cout << "PASS\n\n";

        // Test 3: Local Table Operations
        std::cout << "[Test 3] Local Table Operations...\n";
        {
            TableConfig cfg{"cluster_table", TableType::Set, StorageType::DiscCopies, {"id", "val"}};
            bool created = cluster.local_mnesia().create_table(cfg);
            CHECK(created, "Table cluster_table harus berhasil dibuat");

            cluster.local_mnesia().write("cluster_table", Term(std::string("ck1")), Term(std::string("cv1")));
            auto res = cluster.local_mnesia().read("cluster_table", Term(std::string("ck1")));
            CHECK(res.has_value() && *res->get_if<std::string>() == "cv1", "Read cluster_table harus cv1");
        }
        std::cout << "PASS\n\n";

        // Test 4: Replicate Write
        std::cout << "[Test 4] Replicate Write...\n";
        {
            cluster.replicate_write("cluster_table", Term(std::string("ck2")), Term(std::string("cv2")));
            auto res = cluster.local_mnesia().read("cluster_table", Term(std::string("ck2")));
            CHECK(res.has_value() && *res->get_if<std::string>() == "cv2", "Replicate write harus menyimpan ke local Mnesia");
        }
        std::cout << "PASS\n\n";

        // Test 5: Cluster Health Summary
        std::cout << "[Test 5] Cluster Health Summary...\n";
        {
            auto nodes = cluster.active_nodes();
            CHECK(nodes.size() == 2, "Active cluster nodes = 2");
            CHECK(nodes[0] == "node1" || nodes[1] == "node1", "Local node1 harus ada di cluster");
        }
        std::cout << "PASS\n\n";
    }

    std::filesystem::remove_all(test_dir, ec);

    std::cout << "========================================\n";
    std::cout << "FASE G3.4 - MNESIA CLUSTER SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
