// =====================================================================
// FASE A.3 - RICHAT REALTIME API TESTS
// Memverifikasi richat_realtime.hpp
// =====================================================================
#include "../include/garuda/richat/richat_realtime.hpp"
#include <iostream>
#include <string>
#include <cstdlib>

using namespace garuda::richat;

#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

int main() {
    std::cout << "========================================\n";
    std::cout << "FASE A.3 - RICHAT REALTIME API TESTS\n";
    std::cout << "========================================\n\n";

    RichatRealtime rt;

    // Test 1: Presence
    std::cout << "[Test 1] Presence...\n";
    {
        rt.set_online("usr_a");
        rt.set_online("usr_b");

        auto online = rt.get_online_users();
        CHECK(online.size() == 2, "Jumlah user online harus 2");

        rt.set_offline("usr_a");
        auto online_after = rt.get_online_users();
        CHECK(online_after.size() == 1, "Jumlah user online berkurang jadi 1");
        CHECK(online_after[0] == "usr_b", "User online tersisa = usr_b");
    }
    std::cout << "PASS\n\n";

    // Test 2: Typing Indicator
    std::cout << "[Test 2] Typing Indicator...\n";
    {
        rt.start_typing("usr_b", "conv_100");
        auto typing = rt.get_typing_users("conv_100");
        CHECK(typing.size() == 1 && typing[0] == "usr_b", "usr_b sedang mengetik di conv_100");

        rt.stop_typing("usr_b", "conv_100");
        auto typing_after = rt.get_typing_users("conv_100");
        CHECK(typing_after.empty(), "Sudah tidak ada user mengetik");
    }
    std::cout << "PASS\n\n";

    // Test 3: Read Receipt State
    std::cout << "[Test 3] Read Receipt State...\n";
    {
        CHECK(rt.get_receipt_status("msg_99") == "sent", "Default receipt = sent");

        rt.mark_delivered("msg_99");
        CHECK(rt.get_receipt_status("msg_99") == "delivered", "Receipt ter-update 'delivered'");

        rt.mark_read("msg_99");
        CHECK(rt.get_receipt_status("msg_99") == "read", "Receipt ter-update 'read'");
    }
    std::cout << "PASS\n\n";

    // Test 4: Reaction Add & Remove
    std::cout << "[Test 4] Reaction Add & Remove...\n";
    {
        rt.add_reaction("msg_99", "usr_a", "👍");
        rt.add_reaction("msg_99", "usr_b", "❤️");

        auto react_map = rt.get_reactions("msg_99");
        CHECK(react_map.size() == 2, "Total reaksi untuk msg_99 = 2");
        CHECK(react_map["usr_a"] == "👍", "usr_a emoji = 👍");

        rt.remove_reaction("msg_99", "usr_a");
        auto react_after = rt.get_reactions("msg_99");
        CHECK(react_after.size() == 1, "Total reaksi tersisa = 1");
    }
    std::cout << "PASS\n\n";

    // Test 5: Isolation Between Conversations
    std::cout << "[Test 5] Isolation Between Conversations...\n";
    {
        rt.start_typing("usr_c", "conv_200");
        auto t100 = rt.get_typing_users("conv_100");
        auto t200 = rt.get_typing_users("conv_200");

        CHECK(t100.empty(), "conv_100 tidak memiliki typing user");
        CHECK(t200.size() == 1 && t200[0] == "usr_c", "conv_200 typing user = usr_c");
    }
    std::cout << "PASS\n\n";

    std::cout << "========================================\n";
    std::cout << "FASE A.3 - RICHAT REALTIME SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
