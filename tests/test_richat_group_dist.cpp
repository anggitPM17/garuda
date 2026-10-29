// =====================================================================
// FASE G8.4 - RICHAT GROUP DIST TESTS
// Memverifikasi richat_group_dist.hpp
// =====================================================================
#include "../include/garuda/richat/richat_group_dist.hpp"
#include <iostream>
#include <string>
#include <cstdlib>

using namespace garuda::richat;

#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

int main() {
    std::cout << "========================================\n";
    std::cout << "FASE G8.4 - RICHAT GROUP DIST TESTS\n";
    std::cout << "========================================\n\n";

    RichatGroupDist dist;

    // Test 1: Distribute Key
    std::cout << "[Test 1] Distribute Key...\n";
    {
        auto sk = dist.distribute_key("grp_sec_1", "S2V5XzEyM19CYXNlNjQ=");
        CHECK(sk.epoch == "e1", "Epoch awal harus e1");

        auto key_opt = dist.get_key("grp_sec_1");
        CHECK(key_opt.has_value(), "get_key grp_sec_1 harus ada");
        CHECK(key_opt->key_b64 == "S2V5XzEyM19CYXNlNjQ=", "Key base64 harus cocok");
    }
    std::cout << "PASS\n\n";

    // Test 2: Rotate Key
    std::cout << "[Test 2] Rotate Key...\n";
    {
        auto sk_rot = dist.rotate_key("grp_sec_1", "TmV3S2V5XzQ1Nl9CYXNlNjQ=");
        CHECK(sk_rot.epoch == "e2", "Epoch setelah rotasi harus e2");

        auto key_opt = dist.get_key("grp_sec_1");
        CHECK(key_opt->key_b64 == "TmV3S2V5XzQ1Nl9CYXNlNjQ=", "Key ter-update ke new_key");
    }
    std::cout << "PASS\n\n";

    // Test 3: Current Epoch Check
    std::cout << "[Test 3] Current Epoch Check...\n";
    {
        CHECK(dist.is_current_epoch("grp_sec_1", "e2"), "e2 harus current epoch");
        CHECK(!dist.is_current_epoch("grp_sec_1", "e1"), "e1 (stale) harus return false");
    }
    std::cout << "PASS\n\n";

    // Test 4: Stale Epoch Rejection
    std::cout << "[Test 4] Stale Epoch Rejection...\n";
    {
        CHECK(!dist.is_current_epoch("grp_sec_1", "e0"), "e0 (invalid) return false");
        CHECK(!dist.is_current_epoch("non_existent_grp", "e1"), "Group tidak ada return false");
    }
    std::cout << "PASS\n\n";

    // Test 5: Multiple Groups Key Management
    std::cout << "[Test 5] Multiple Groups Key Management...\n";
    {
        dist.distribute_key("grp_sec_2", "S2V5X0dyb3VwMl9CYXNlNjQ=");
        CHECK(dist.key_count() == 2, "Total key_count harus 2");
        CHECK(dist.get_key("grp_sec_2")->epoch == "e1", "grp_sec_2 epoch = e1");
    }
    std::cout << "PASS\n\n";

    std::cout << "========================================\n";
    std::cout << "FASE G8.4 - RICHAT GROUP DIST SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
