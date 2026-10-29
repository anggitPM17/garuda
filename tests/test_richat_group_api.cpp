// =====================================================================
// FASE A.2 - RICHAT GROUP CHAT API TESTS
// Memverifikasi richat_group_api.hpp
// =====================================================================
#include "../include/garuda/richat/richat_group_api.hpp"
#include <iostream>
#include <string>
#include <cstdlib>

using namespace garuda::richat;

#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

int main() {
    std::cout << "========================================\n";
    std::cout << "FASE A.2 - RICHAT GROUP CHAT API TESTS\n";
    std::cout << "========================================\n\n";

    RichatGroupApi gapi;

    // Test 1: Create Group
    std::cout << "[Test 1] Create Group...\n";
    {
        auto grp_opt = gapi.create_group("usr_owner", "Komunitas Garuda BEAM", {"usr_b", "usr_c"});
        CHECK(grp_opt.has_value(), "create_group harus return RichatGroup");
        CHECK(grp_opt->name == "Komunitas Garuda BEAM", "Group name match");
        CHECK(grp_opt->member_ids.size() == 3, "Member count harus 3 (termasuk owner)");
    }
    std::cout << "PASS\n\n";

    // Test 2: Join Group
    std::cout << "[Test 2] Join Group...\n";
    {
        auto list = gapi.list_groups("usr_owner");
        CHECK(!list.empty(), "usr_owner harus punya 1 grup");

        std::string grp_id = list[0].id;
        bool joined = gapi.join_group(grp_id, "usr_d");
        CHECK(joined, "join_group usr_d harus return true");

        auto list_d = gapi.list_groups("usr_d");
        CHECK(list_d.size() == 1, "usr_d sekarang anggota grup");
    }
    std::cout << "PASS\n\n";

    // Test 3: Leave Group
    std::cout << "[Test 3] Leave Group...\n";
    {
        auto list_d = gapi.list_groups("usr_d");
        std::string grp_id = list_d[0].id;

        bool left = gapi.leave_group(grp_id, "usr_d");
        CHECK(left, "leave_group usr_d harus return true");

        auto list_d_after = gapi.list_groups("usr_d");
        CHECK(list_d_after.empty(), "usr_d sudah bukan anggota grup lagi");
    }
    std::cout << "PASS\n\n";

    // Test 4: List Groups
    std::cout << "[Test 4] List Groups...\n";
    {
        gapi.create_group("usr_b", "Grup Kedua B", {"usr_c"});
        auto list_b = gapi.list_groups("usr_b");
        CHECK(list_b.size() == 2, "usr_b harus terdaftar di 2 grup");
    }
    std::cout << "PASS\n\n";

    // Test 5: Send & Get Group Messages
    std::cout << "[Test 5] Send & Get Group Messages...\n";
    {
        auto list_b = gapi.list_groups("usr_b");
        std::string grp_id = list_b[0].id;

        auto gmsg_opt = gapi.send_group_message(grp_id, "usr_b", "Halo teman-teman grup!");
        CHECK(gmsg_opt.has_value(), "send_group_message harus return ChatMessage");

        auto msgs = gapi.get_group_messages(grp_id);
        CHECK(msgs.size() == 1, "Pesan grup harus bernilai 1");
        CHECK(msgs[0].content == "Halo teman-teman grup!", "Content match");
    }
    std::cout << "PASS\n\n";

    std::cout << "========================================\n";
    std::cout << "FASE A.2 - RICHAT GROUP CHAT API SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
