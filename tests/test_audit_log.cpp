// =====================================================================
// FASE C.5.4 - GARUDA AUDIT LOG TESTS
// Memverifikasi audit_log.hpp
// =====================================================================
#include "../include/garuda/security/audit_log.hpp"
#include <iostream>
#include <string>
#include <cstdlib>

using namespace garuda::security;

#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

int main() {
    std::cout << "========================================\n";
    std::cout << "FASE C.5.4 - AUDIT LOG TESTS\n";
    std::cout << "========================================\n\n";

    AuditLog log(100);

    // Test 1: Record Entries
    std::cout << "[Test 1] Record Entries...\n";
    {
        log.record_action("usr_a", "login", "auth", "success", "192.168.1.1");
        log.record_action("usr_a", "delete_msg", "msg:100", "success", "192.168.1.1");
        log.record_action("usr_b", "login", "auth", "failure", "10.0.0.5");

        CHECK(log.entry_count() == 3, "entry_count() harus 3");
    }
    std::cout << "PASS\n\n";

    // Test 2: Query By User
    std::cout << "[Test 2] Query By User...\n";
    {
        auto entries_a = log.get_entries_by_user("usr_a");
        CHECK(entries_a.size() == 2, "User usr_a harus punya 2 audit entries");

        auto entries_b = log.get_entries_by_user("usr_b");
        CHECK(entries_b.size() == 1, "User usr_b harus punya 1 audit entry");
        CHECK(entries_b[0].status == "failure", "Status entry usr_b harus failure");
    }
    std::cout << "PASS\n\n";

    // Test 3: Query By Action
    std::cout << "[Test 3] Query By Action...\n";
    {
        auto logins = log.get_entries_by_action("login");
        CHECK(logins.size() == 2, "Action 'login' harus ada 2 entries");
    }
    std::cout << "PASS\n\n";

    // Test 4: Export JSON
    std::cout << "[Test 4] Export JSON...\n";
    {
        std::string json = log.export_json();
        CHECK(!json.empty(), "export_json() tidak boleh kosong");
        CHECK(json.find("{\"entries\":[") == 0, "JSON prefix harus {\"entries\":[");
        CHECK(json.find("usr_a") != std::string::npos, "JSON harus memuat usr_a");
    }
    std::cout << "PASS\n\n";

    // Test 5: Ring Buffer Limit Enforcement
    std::cout << "[Test 5] Ring Buffer Limit Enforcement...\n";
    {
        AuditLog small_log(3); // Max 3 entries
        small_log.record_action("u1", "act1", "res", "ok");
        small_log.record_action("u2", "act2", "res", "ok");
        small_log.record_action("u3", "act3", "res", "ok");
        small_log.record_action("u4", "act4", "res", "ok");

        CHECK(small_log.entry_count() == 3, "entry_count() harus dibatasi max 3");
        auto entries = small_log.get_entries();
        CHECK(entries[0].user_id == "u4", "Entry terbaru u4 harus berada di urutan pertama");
    }
    std::cout << "PASS\n\n";

    std::cout << "========================================\n";
    std::cout << "FASE C.5.4 - AUDIT LOG SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
