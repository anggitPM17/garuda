// =====================================================================
// FASE G3.2 - GARUDA MNESIA TIER TESTS
// Memverifikasi mnesia_tier.hpp
// =====================================================================
#include "../include/garuda/beam/otp/mnesia_tier.hpp"
#include <iostream>
#include <string>
#include <cstdlib>

using namespace garuda::beam;

#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

int main() {
    std::cout << "========================================\n";
    std::cout << "FASE G3.2 - GARUDA MNESIA TIER TESTS\n";
    std::cout << "========================================\n\n";

    TieredStorage<std::string, std::string> storage;

    // Test 1: Write to Hot & Read
    std::cout << "[Test 1] Write to Hot & Read...\n";
    {
        storage.write("active_msg_1", "Hello Hot Tier", Tier::HOT);
        auto val = storage.read("active_msg_1");
        CHECK(val.has_value() && *val == "Hello Hot Tier", "Read dari Hot Tier harus sesuai");
        CHECK(storage.get_tier("active_msg_1") == Tier::HOT, "Tier harus HOT");
        CHECK(storage.hot_count() == 1, "Hot count harus 1");
    }
    std::cout << "PASS\n\n";

    // Test 2: Demote Hot to Warm
    std::cout << "[Test 2] Demote Hot to Warm...\n";
    {
        bool demoted = storage.demote_to_warm("active_msg_1");
        CHECK(demoted, "Demote hot to warm harus return true");
        CHECK(storage.get_tier("active_msg_1") == Tier::WARM, "Tier harus WARM");
        CHECK(storage.hot_count() == 0, "Hot count harus 0");
        CHECK(storage.warm_count() == 1, "Warm count harus 1");
    }
    std::cout << "PASS\n\n";

    // Test 3: Demote Warm to Cold
    std::cout << "[Test 3] Demote Warm to Cold...\n";
    {
        bool demoted = storage.demote_to_cold("active_msg_1");
        CHECK(demoted, "Demote warm to cold harus return true");
        CHECK(storage.get_tier("active_msg_1") == Tier::COLD, "Tier harus COLD");
        CHECK(storage.warm_count() == 0, "Warm count harus 0");
        CHECK(storage.cold_count() == 1, "Cold count harus 1");
    }
    std::cout << "PASS\n\n";

    // Test 4: Promote Cold to Hot
    std::cout << "[Test 4] Promote Cold to Hot...\n";
    {
        bool promoted = storage.promote_to_hot("active_msg_1");
        CHECK(promoted, "Promote cold to hot harus return true");
        CHECK(storage.get_tier("active_msg_1") == Tier::HOT, "Tier harus kembali ke HOT");
        CHECK(storage.hot_count() == 1, "Hot count harus 1");
        CHECK(storage.cold_count() == 0, "Cold count harus 0");
    }
    std::cout << "PASS\n\n";

    // Test 5: Read Across Tiers
    std::cout << "[Test 5] Read Across Tiers...\n";
    {
        storage.write("k_hot", "v_hot", Tier::HOT);
        storage.write("k_warm", "v_warm", Tier::WARM);
        storage.write("k_cold", "v_cold", Tier::COLD);

        CHECK(storage.read("k_hot") == "v_hot", "Read k_hot harus v_hot");
        CHECK(storage.read("k_warm") == "v_warm", "Read k_warm harus v_warm");
        CHECK(storage.read("k_cold") == "v_cold", "Read k_cold harus v_cold");
        CHECK(!storage.read("k_missing").has_value(), "Read missing key harus std::nullopt");
    }
    std::cout << "PASS\n\n";

    std::cout << "========================================\n";
    std::cout << "FASE G3.2 - MNESIA TIER SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
