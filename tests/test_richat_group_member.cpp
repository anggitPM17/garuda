// =====================================================================
// FASE G8.2 - RICHAT GROUP MEMBER TESTS
// Memverifikasi richat_group_member.hpp
// =====================================================================
#include "../include/garuda/richat/richat_group_member.hpp"
#include <iostream>
#include <string>
#include <cstdlib>

using namespace garuda::richat;

#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

int main() {
    std::cout << "========================================\n";
    std::cout << "FASE G8.2 - RICHAT GROUP MEMBER TESTS\n";
    std::cout << "========================================\n\n";

    RichatGroupMember rgm;

    // Test 1: Add & Check Member
    std::cout << "[Test 1] Add & Check Member...\n";
    {
        bool added = rgm.add_member("grp_1", "user_101", MemberRole::OWNER);
        CHECK(added, "add_member user_101 harus return true");
        CHECK(rgm.is_member("grp_1", "user_101"), "user_101 harus is_member grp_1");
        CHECK(rgm.member_count("grp_1") == 1, "member_count grp_1 harus 1");
    }
    std::cout << "PASS\n\n";

    // Test 2: Duplicate Member Rejection
    std::cout << "[Test 2] Duplicate Member Rejection...\n";
    {
        bool dup = rgm.add_member("grp_1", "user_101", MemberRole::MEMBER);
        CHECK(!dup, "Duplikat add_member harus return false");
        CHECK(rgm.member_count("grp_1") == 1, "member_count grp_1 tetap 1");
    }
    std::cout << "PASS\n\n";

    // Test 3: Remove Member
    std::cout << "[Test 3] Remove Member...\n";
    {
        rgm.add_member("grp_1", "user_102", MemberRole::MEMBER);
        CHECK(rgm.is_member("grp_1", "user_102"), "user_102 ditambahkan");

        bool removed = rgm.remove_member("grp_1", "user_102");
        CHECK(removed, "remove_member user_102 harus return true");
        CHECK(!rgm.is_member("grp_1", "user_102"), "user_102 terhapus");
    }
    std::cout << "PASS\n\n";

    // Test 4: List Members
    std::cout << "[Test 4] List Members...\n";
    {
        rgm.add_member("grp_2", "u1", MemberRole::OWNER);
        rgm.add_member("grp_2", "u2", MemberRole::ADMIN);
        rgm.add_member("grp_2", "u3", MemberRole::MEMBER);

        auto members = rgm.list_members("grp_2");
        CHECK(members.size() == 3, "List members grp_2 size = 3");
    }
    std::cout << "PASS\n\n";

    // Test 5: Role Management & Moderation
    std::cout << "[Test 5] Role Management & Moderation...\n";
    {
        CHECK(rgm.can_moderate("grp_2", "u1"), "u1 (OWNER) bisa moderate");
        CHECK(rgm.can_moderate("grp_2", "u2"), "u2 (ADMIN) bisa moderate");
        CHECK(!rgm.can_moderate("grp_2", "u3"), "u3 (MEMBER) tidak bisa moderate");

        rgm.set_role("grp_2", "u3", MemberRole::ADMIN);
        CHECK(rgm.can_moderate("grp_2", "u3"), "u3 setelah promosi ke ADMIN bisa moderate");
    }
    std::cout << "PASS\n\n";

    std::cout << "========================================\n";
    std::cout << "FASE G8.2 - RICHAT GROUP MEMBER SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
