// =====================================================================
// FASE G5.5 - GARUDA REACTION TESTS
// Memverifikasi reaction.hpp
// =====================================================================
#include "../include/garuda/beam/webrtc/reaction.hpp"
#include <iostream>
#include <string>
#include <cstdlib>

using namespace garuda::beam;

#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

int main() {
    std::cout << "========================================\n";
    std::cout << "FASE G5.5 - GARUDA REACTION TESTS\n";
    std::cout << "========================================\n\n";

    ReactionStore store;

    // Test 1: Add Reaction
    std::cout << "[Test 1] Add Reaction...\n";
    {
        store.add_reaction("msg_101", "user_alice", "❤️");
        CHECK(store.total_reaction_count("msg_101") == 1, "Reaction count msg_101 harus 1");
    }
    std::cout << "PASS\n\n";

    // Test 2: Multiple Users Reactions
    std::cout << "[Test 2] Multiple Users Reactions...\n";
    {
        store.add_reaction("msg_101", "user_bob", "🔥");
        store.add_reaction("msg_101", "user_charlie", "👍");
        CHECK(store.total_reaction_count("msg_101") == 3, "Reaction count msg_101 harus 3");
    }
    std::cout << "PASS\n\n";

    // Test 3: Same User Reaction Update
    std::cout << "[Test 3] Same User Reaction Update...\n";
    {
        store.add_reaction("msg_101", "user_alice", "🚀"); // Alice ganti emoji dari ❤️ ke 🚀
        CHECK(store.total_reaction_count("msg_101") == 3, "Total reaction count tetap 3");

        auto map = store.get_reactions("msg_101");
        CHECK(map["user_alice"] == "🚀", "Emoji Alice ter-update menjadi 🚀");
    }
    std::cout << "PASS\n\n";

    // Test 4: Remove Reaction
    std::cout << "[Test 4] Remove Reaction...\n";
    {
        bool removed = store.remove_reaction("msg_101", "user_charlie");
        CHECK(removed, "Remove reaction user_charlie harus return true");
        CHECK(store.total_reaction_count("msg_101") == 2, "Reaction count sisa 2");
    }
    std::cout << "PASS\n\n";

    // Test 5: Get Reactions Listing
    std::cout << "[Test 5] Get Reactions Listing...\n";
    {
        auto map = store.get_reactions("msg_101");
        CHECK(map.size() == 2, "Get reactions map size = 2");
        CHECK(map["user_alice"] == "🚀", "Alice reaction = 🚀");
        CHECK(map["user_bob"] == "🔥", "Bob reaction = 🔥");
    }
    std::cout << "PASS\n\n";

    std::cout << "========================================\n";
    std::cout << "FASE G5.5 - REACTION SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
