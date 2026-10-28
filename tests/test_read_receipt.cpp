// =====================================================================
// FASE G5.4 - GARUDA READ RECEIPT TESTS
// Memverifikasi read_receipt.hpp
// =====================================================================
#include "../include/garuda/beam/webrtc/read_receipt.hpp"
#include <iostream>
#include <string>
#include <cstdlib>

using namespace garuda::beam;

#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

int main() {
    std::cout << "========================================\n";
    std::cout << "FASE G5.4 - GARUDA READ RECEIPT TESTS\n";
    std::cout << "========================================\n\n";

    ReadReceipt rr;

    // Test 1: Mark Delivered
    std::cout << "[Test 1] Mark Delivered...\n";
    {
        rr.mark_delivered("msg_101", "user_bob");
        auto rec = rr.get_receipt("msg_101", "user_bob");
        CHECK(rec.has_value(), "Receipt msg_101 user_bob harus ada");
        CHECK(rec->status == ReceiptStatus::DELIVERED, "Status harus DELIVERED");
        CHECK(rr.count_by_status(ReceiptStatus::DELIVERED) == 1, "Count DELIVERED = 1");
    }
    std::cout << "PASS\n\n";

    // Test 2: Mark Read
    std::cout << "[Test 2] Mark Read...\n";
    {
        rr.mark_read("msg_101", "user_bob");
        auto rec = rr.get_receipt("msg_101", "user_bob");
        CHECK(rec.has_value(), "Receipt msg_101 user_bob harus ada");
        CHECK(rec->status == ReceiptStatus::READ, "Status harus READ");
        CHECK(rr.count_by_status(ReceiptStatus::READ) == 1, "Count READ = 1");
        CHECK(rr.count_by_status(ReceiptStatus::DELIVERED) == 0, "Count DELIVERED = 0 setelah diubah ke READ");
    }
    std::cout << "PASS\n\n";

    // Test 3: Multiple Messages Read Count
    std::cout << "[Test 3] Multiple Messages Read Count...\n";
    {
        rr.mark_read("msg_102", "user_charlie");
        rr.mark_read("msg_103", "user_david");
        CHECK(rr.count_by_status(ReceiptStatus::READ) == 3, "Count READ = 3");
    }
    std::cout << "PASS\n\n";

    // Test 4: Get Receipt Verification
    std::cout << "[Test 4] Get Receipt Verification...\n";
    {
        auto rec = rr.get_receipt("msg_102", "user_charlie");
        CHECK(rec.has_value(), "Receipt msg_102 user_charlie ditemukan");
        CHECK(rec->recipient_id == "user_charlie", "Recipient ID cocok");
    }
    std::cout << "PASS\n\n";

    // Test 5: Unknown Receipt Query
    std::cout << "[Test 5] Unknown Receipt Query...\n";
    {
        auto rec = rr.get_receipt("msg_unknown", "user_unknown");
        CHECK(!rec.has_value(), "Unknown receipt query harus std::nullopt");
    }
    std::cout << "PASS\n\n";

    std::cout << "========================================\n";
    std::cout << "FASE G5.4 - READ RECEIPT SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
