#include "../include/garuda/jalur_langit/auth_manager.hpp"
#include <iostream>
#include <cstdlib>

using namespace garuda::jalur_langit;
#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

int main() {
    AuthManager auth;
    auth.grant("budi", "users", Permission::READ);
    auth.grant("siti", "users", Permission::WRITE);
    auth.grant("admin", "users", Permission::ADMIN);

    CHECK(auth.check_permission("budi", "users", Permission::READ), "budi harus bisa READ");
    CHECK(!auth.check_permission("budi", "users", Permission::WRITE), "budi tidak boleh WRITE");

    CHECK(auth.check_permission("siti", "users", Permission::READ), "siti bisa READ (WRITE > READ)");
    CHECK(auth.check_permission("siti", "users", Permission::WRITE), "siti bisa WRITE");

    CHECK(auth.check_permission("admin", "users", Permission::ADMIN), "admin bisa ADMIN");
    CHECK(auth.check_permission("admin", "users", Permission::WRITE), "admin bisa WRITE");

    CHECK(!auth.check_permission("budi", "posts", Permission::READ), "budi tidak punya akses ke posts");

    auth.revoke("budi", "users");
    CHECK(!auth.check_permission("budi", "users", Permission::READ), "setelah revoke, budi tidak bisa READ");

    std::cout << "AUTH MANAGER TESTS PASSED!\n";
    return 0;
}
