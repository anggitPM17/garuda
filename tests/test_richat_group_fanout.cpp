// =====================================================================
// FASE G8.3 - RICHAT GROUP FANOUT TESTS
// Memverifikasi richat_group_fanout.hpp
// =====================================================================
#include "../include/garuda/richat/richat_group_fanout.hpp"
#include <iostream>
#include <string>
#include <cstdlib>

using namespace garuda::richat;

#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

int main() {
    std::cout << "========================================\n";
    std::cout << "FASE G8.3 - RICHAT GROUP FANOUT TESTS\n";
    std::cout << "========================================\n\n";

    RichatGroupMember rgm;
    rgm.add_member("grp_dev", "user_sender", MemberRole::OWNER);
    rgm.add_member("grp_dev", "user_receiver_1", MemberRole::MEMBER);
    rgm.add_member("grp_dev", "user_receiver_2", MemberRole::MEMBER);

    // Test 1: Simple Fanout (Sender Excluded)
    std::cout << "[Test 1] Simple Fanout (Sender Excluded)...\n";
    {
        RichatGroupFanout fanout(&rgm, [](const std::string& /*uid*/, const std::string& /*msg_id*/) {
            return true;
        });

        auto res = fanout.fanout("grp_dev", "user_sender", "msg_100");
        CHECK(res.target_count == 2, "Target count harus 2 (user_sender dieksklusi)");
        CHECK(res.delivered_count == 2, "Delivered count harus 2");
        CHECK(res.failed_count == 0, "Failed count harus 0");
    }
    std::cout << "PASS\n\n";

    // Test 2: Partial Delivery Failure
    std::cout << "[Test 2] Partial Delivery Failure...\n";
    {
        RichatGroupFanout fanout(&rgm, [](const std::string& uid, const std::string& /*msg_id*/) {
            if (uid == "user_receiver_2") return false; // Gagal
            return true;
        });

        auto res = fanout.fanout("grp_dev", "user_sender", "msg_101");
        CHECK(res.target_count == 2, "Target count = 2");
        CHECK(res.delivered_count == 1, "Delivered count = 1");
        CHECK(res.failed_count == 1, "Failed count = 1");
    }
    std::cout << "PASS\n\n";

    // Test 3: Batch Fanout
    std::cout << "[Test 3] Batch Fanout...\n";
    {
        RichatGroupFanout fanout(&rgm, [](const std::string& /*uid*/, const std::string& /*msg_id*/) {
            return true;
        });

        auto results = fanout.batch_fanout("grp_dev", "user_sender", {"batch_1", "batch_2", "batch_3"});
        CHECK(results.size() == 3, "Batch fanout result size = 3");
        CHECK(results[0].delivered_count == 2, "Batch 1 delivered = 2");
        CHECK(results[2].delivered_count == 2, "Batch 3 delivered = 2");
    }
    std::cout << "PASS\n\n";

    // Test 4: Duration Tracking
    std::cout << "[Test 4] Duration Tracking...\n";
    {
        RichatGroupFanout fanout(&rgm, [](const std::string& /*uid*/, const std::string& /*msg_id*/) {
            return true;
        });

        auto res = fanout.fanout("grp_dev", "user_sender", "msg_perf");
        CHECK(res.duration.count() >= 0, "Duration us harus >= 0");
    }
    std::cout << "PASS\n\n";

    // Test 5: Empty Group Fanout
    std::cout << "[Test 5] Empty Group Fanout...\n";
    {
        RichatGroupFanout fanout(&rgm, [](const std::string& /*uid*/, const std::string& /*msg_id*/) {
            return true;
        });

        auto res = fanout.fanout("empty_group", "user_sender", "msg_empty");
        CHECK(res.target_count == 0, "Target count empty group = 0");
        CHECK(res.delivered_count == 0, "Delivered count empty group = 0");
    }
    std::cout << "PASS\n\n";

    std::cout << "========================================\n";
    std::cout << "FASE G8.3 - RICHAT GROUP FANOUT SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
