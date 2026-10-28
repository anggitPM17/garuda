// =====================================================================
// FASE G5.2 - GARUDA PRESENCE TESTS
// Memverifikasi presence.hpp
// =====================================================================
#include "../include/garuda/beam/webrtc/presence.hpp"
#include <iostream>
#include <string>
#include <cstdlib>
#include <thread>

using namespace garuda::beam;

#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

int main() {
    std::cout << "========================================\n";
    std::cout << "FASE G5.2 - GARUDA PRESENCE TESTS\n";
    std::cout << "========================================\n\n";

    Presence p;

    // Test 1: Set Online
    std::cout << "[Test 1] Set Online...\n";
    {
        p.set_online("user_101", PresenceStatus::ONLINE);
        CHECK(p.is_online("user_101"), "user_101 harus online");
        CHECK(p.online_count() == 1, "Online count harus 1");
    }
    std::cout << "PASS\n\n";

    // Test 2: Set Offline
    std::cout << "[Test 2] Set Offline...\n";
    {
        p.set_offline("user_101");
        CHECK(!p.is_online("user_101"), "user_101 harus offline");
        CHECK(p.online_count() == 0, "Online count harus 0");
    }
    std::cout << "PASS\n\n";

    // Test 3: Status Message & Custom Status
    std::cout << "[Test 3] Status Message & Custom Status...\n";
    {
        p.set_online("user_102", PresenceStatus::BUSY, "In a meeting");
        auto rec = p.get_presence("user_102");
        CHECK(rec.has_value(), "Presence record user_102 ada");
        CHECK(rec->status == PresenceStatus::BUSY, "Status harus BUSY");
        CHECK(rec->status_message == "In a meeting", "Status message harus 'In a meeting'");
    }
    std::cout << "PASS\n\n";

    // Test 4: Multiple Users Online Count
    std::cout << "[Test 4] Multiple Users Online Count...\n";
    {
        p.set_online("u1", PresenceStatus::ONLINE);
        p.set_online("u2", PresenceStatus::AWAY);
        p.set_online("u3", PresenceStatus::BUSY);
        p.set_offline("u4");

        CHECK(p.online_count() == 4, "Total online count (u1, u2, u3, user_102) harus 4");
        CHECK(p.total_count() == 6, "Total count (termasuk offline) harus 6");

        auto online_users = p.get_online_users();
        CHECK(online_users.size() == 4, "Daftar online users size harus 4");
    }
    std::cout << "PASS\n\n";

    // Test 5: Heartbeat Update
    std::cout << "[Test 5] Heartbeat Update...\n";
    {
        auto rec1 = p.get_presence("u1");
        auto t1 = rec1->last_seen;

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        p.heartbeat("u1");

        auto rec2 = p.get_presence("u1");
        auto t2 = rec2->last_seen;

        CHECK(t2 > t1, "Heartbeat harus memperbarui last_seen timestamp");
    }
    std::cout << "PASS\n\n";

    std::cout << "========================================\n";
    std::cout << "FASE G5.2 - PRESENCE SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
