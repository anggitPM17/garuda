// =====================================================================
// FASE G8.5 - RICHAT COLD DB TESTS
// Memverifikasi richat_cold_db.hpp
// =====================================================================
#include "../include/garuda/richat/richat_cold_db.hpp"
#include <iostream>
#include <string>
#include <filesystem>
#include <cstdlib>
#include <system_error>

using namespace garuda::richat;

#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

int main() {
    std::cout << "========================================\n";
    std::cout << "FASE G8.5 - RICHAT COLD DB TESTS\n";
    std::cout << "========================================\n\n";

    std::string test_dir = "./test_richat_cold_db_dir";
    std::error_code ec;
    std::filesystem::remove_all(test_dir, ec);

    {
        garuda::beam::Mnesia mnesia(test_dir);
        garuda::beam::TableConfig cfg{"old_messages", garuda::beam::TableType::Set, garuda::beam::StorageType::DiscCopies, {"id", "msg"}};
        mnesia.create_table(cfg);

        mnesia.write("old_messages", garuda::beam::Term(std::string("m1")), garuda::beam::Term(std::string("Old Msg 1")));
        mnesia.write("old_messages", garuda::beam::Term(std::string("m2")), garuda::beam::Term(std::string("Old Msg 2")));

        RichatColdDb cold_db(&mnesia);

        // Test 1: Init Cold DB & Archive Table
        std::cout << "[Test 1] Init Cold DB & Archive Table...\n";
        {
            auto res = cold_db.archive("old_messages", std::chrono::seconds(86400));
            CHECK(res.table == "old_messages", "Table name harus match");
            CHECK(res.archived_count == 2, "Archived count harus 2");
            CHECK(cold_db.cold_count() == 2, "Cold count total harus 2");
        }
        std::cout << "PASS\n\n";

        // Test 2: Archive Single Record
        std::cout << "[Test 2] Archive Single Record...\n";
        {
            bool ok = cold_db.archive_record("old_messages", "m3", "Old Msg 3");
            CHECK(ok, "archive_record harus return true");
            CHECK(cold_db.cold_count() == 3, "Cold count total harus 3");
        }
        std::cout << "PASS\n\n";

        // Test 3: Restore Record
        std::cout << "[Test 3] Restore Record...\n";
        {
            bool restored = cold_db.restore("old_messages", "m3");
            CHECK(restored, "restore harus return true");
            CHECK(cold_db.cold_count() == 2, "Cold count berkurang jadi 2 setelah restore");

            auto m3_val = mnesia.read("old_messages", garuda::beam::Term(std::string("m3")));
            CHECK(m3_val.has_value() && *m3_val->get_if<std::string>() == "Old Msg 3", "Mnesia read m3 harus Old Msg 3");
        }
        std::cout << "PASS\n\n";

        // Test 4: Restore Non-Existent
        std::cout << "[Test 4] Restore Non-Existent...\n";
        {
            bool restored = cold_db.restore("old_messages", "m_unknown");
            CHECK(!restored, "Restore unknown key harus return false");
        }
        std::cout << "PASS\n\n";

        // Test 5: Duration Tracking
        std::cout << "[Test 5] Duration Tracking...\n";
        {
            auto res = cold_db.archive("old_messages", std::chrono::seconds(86400));
            CHECK(res.duration.count() >= 0, "Archive duration us harus >= 0");
        }
        std::cout << "PASS\n\n";
    }

    std::filesystem::remove_all(test_dir, ec);

    std::cout << "========================================\n";
    std::cout << "FASE G8.5 - RICHAT COLD DB SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
