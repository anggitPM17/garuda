// =====================================================================
// FASE A.4 - RICHAT OFFLINE QUEUE TESTS
// Memverifikasi richat_offline_queue.hpp
// =====================================================================
#include "../include/garuda/richat/richat_offline_queue.hpp"
#include <iostream>
#include <string>
#include <cstdlib>

using namespace garuda::richat;

#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

int main() {
    std::cout << "========================================\n";
    std::cout << "FASE A.4 - RICHAT OFFLINE QUEUE TESTS\n";
    std::cout << "========================================\n\n";

    RichatOfflineQueue oq(":memory:");

    // Test 1: Enqueue Offline Message
    std::cout << "[Test 1] Enqueue Offline Message...\n";
    {
        QueuedMessage qm1{"qm_1", "usr_a", "usr_b", "Pesan offline 1", 1700000000, 0};
        QueuedMessage qm2{"qm_2", "usr_a", "usr_c", "Pesan offline 2", 1700000001, 0};

        CHECK(oq.enqueue(qm1), "enqueue qm1 harus return true");
        CHECK(oq.enqueue(qm2), "enqueue qm2 harus return true");
        CHECK(oq.pending_count() == 2, "pending_count harus 2");
    }
    std::cout << "PASS\n\n";

    // Test 2: Get Pending
    std::cout << "[Test 2] Get Pending...\n";
    {
        auto pending = oq.get_pending();
        CHECK(pending.size() == 2, "get_pending() size harus 2");
        CHECK(pending[0].id == "qm_1", "Pesan pertama ID = qm_1");
    }
    std::cout << "PASS\n\n";

    // Test 3: Increment Retry
    std::cout << "[Test 3] Increment Retry...\n";
    {
        bool retried = oq.increment_retry("qm_1");
        CHECK(retried, "increment_retry qm_1 harus return true");

        auto pending = oq.get_pending();
        auto it = std::find_if(pending.begin(), pending.end(), [](const QueuedMessage& m) { return m.id == "qm_1"; });
        CHECK(it != pending.end() && it->retry_count == 1, "retry_count qm_1 harus 1");
    }
    std::cout << "PASS\n\n";

    // Test 4: Mark Sent (Dequeue)
    std::cout << "[Test 4] Mark Sent (Dequeue)...\n";
    {
        bool sent = oq.mark_sent("qm_1");
        CHECK(sent, "mark_sent qm_1 harus return true");
        CHECK(oq.pending_count() == 1, "pending_count berkurang jadi 1");
    }
    std::cout << "PASS\n\n";

    // Test 5: Clear
    std::cout << "[Test 5] Clear...\n";
    {
        oq.clear();
        CHECK(oq.pending_count() == 0, "pending_count harus 0 setelah clear()");
    }
    std::cout << "PASS\n\n";

    std::cout << "========================================\n";
    std::cout << "FASE A.4 - RICHAT OFFLINE QUEUE SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
