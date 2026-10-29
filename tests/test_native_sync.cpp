// =====================================================================
// FASE D.4.3 - RICHAT NATIVE SYNC TESTS
// Memverifikasi native_sync.h & native_sync.cpp
// =====================================================================
#include "../android/app/src/main/cpp/native_sync.h"
#include <iostream>
#include <string>
#include <cstdlib>

using namespace richat::native;

#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

int main() {
    std::cout << "========================================\n";
    std::cout << "FASE D.4.3 - NATIVE SYNC TESTS\n";
    std::cout << "========================================\n\n";

    NativeSync sync(":memory:");

    // Test 1: Save Messages
    std::cout << "[Test 1] Save Messages...\n";
    {
        NativeMessage m1{"m101", "usr_a", "usr_b", "Hello Native", 1700000000, 0};
        NativeMessage m2{"m102", "usr_a", "usr_b", "Message 2", 1700000001, 0};
        NativeMessage m3{"m103", "usr_a", "usr_c", "Message 3", 1700000002, 0};

        CHECK(sync.save_message(m1), "save_message m1 harus return true");
        CHECK(sync.save_message(m2), "save_message m2 harus return true");
        CHECK(sync.save_message(m3), "save_message m3 harus return true");

        CHECK(sync.total_count() == 3, "total_count() harus 3");
        CHECK(sync.pending_count() == 3, "pending_count() harus 3");
    }
    std::cout << "PASS\n\n";

    // Test 2: Get Pending Messages
    std::cout << "[Test 2] Get Pending Messages...\n";
    {
        auto pending = sync.get_pending_messages();
        CHECK(pending.size() == 3, "Pending messages size harus 3");
        CHECK(pending[0].id == "m101", "Pesan 1 ID = m101");
    }
    std::cout << "PASS\n\n";

    // Test 3: Mark Sent
    std::cout << "[Test 3] Mark Sent...\n";
    {
        bool ok = sync.mark_sent("m101");
        CHECK(ok, "mark_sent m101 harus return true");
        CHECK(sync.pending_count() == 2, "pending_count() berkurang jadi 2");
    }
    std::cout << "PASS\n\n";

    // Test 4: Mark Delivered & Read
    std::cout << "[Test 4] Mark Delivered & Read...\n";
    {
        CHECK(sync.mark_delivered("m102"), "mark_delivered m102 = true");
        CHECK(sync.mark_read("m103"), "mark_read m103 = true");
        CHECK(sync.pending_count() == 0, "pending_count() harus 0 setelah semua ter-sync");
        CHECK(sync.total_count() == 3, "total_count() tetap 3");
    }
    std::cout << "PASS\n\n";

    // Test 5: Save Empty ID Rejection
    std::cout << "[Test 5] Save Empty ID Rejection...\n";
    {
        NativeMessage m_bad{"", "a", "b", "test", 0, 0};
        CHECK(!sync.save_message(m_bad), "save_message dengan ID kosong harus return false");
    }
    std::cout << "PASS\n\n";

    std::cout << "========================================\n";
    std::cout << "FASE D.4.3 - NATIVE SYNC SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
