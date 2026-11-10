#include "../include/garuda/beam/stdlib/orm.hpp"
#include "../include/garuda/beam/stdlib/db.hpp"
#include <cassert>
#include <filesystem>
#include <iostream>

using namespace garuda::beam::stdlib;
using namespace garuda::beam;
namespace fs = std::filesystem;

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA WEB FRAMEWORK — SPRINT D.4 NATIVE ORM TEST SUITE (10 TESTS)\n";
    std::cout << "=======================================================\n\n";

    fs::path tmp_db_file = fs::temp_directory_path() / "garuda_orm_test_dir";
    fs::remove_all(tmp_db_file);

    Database db(tmp_db_file.string());
    OrmEngine orm(db);

    // === Test 1: Model Definition & Schema Registration ===
    std::cout << "[TEST 1/10] Running Model Definition & Schema Registration test...\n";
    orm.define_model("User", {{"nama", "string"}, {"email", "string"}}, "Post", "", "user_id");
    orm.define_model("Post", {{"title", "string"}, {"user_id", "string"}}, "", "User", "user_id");
    assert(orm.schema_version() == 1);
    std::cout << "  ✓ Test 1: Model Definition & Schema Registration PASS\n";

    // === Test 2: ORM Record Insertion ===
    std::cout << "[TEST 2/10] Running ORM Record Insertion test...\n";
    Term u1_rec = list_of({tuple_of({Term("nama"), Term("Budi")}), tuple_of({Term("email"), Term("budi@garuda.org")})});
    bool ok2 = orm.insert("User", "usr_101", u1_rec);
    assert(ok2);
    std::cout << "  ✓ Test 2: ORM Record Insertion PASS\n";

    // === Test 3: ORM Record Find by Primary Key ===
    std::cout << "[TEST 3/10] Running ORM Record Find test...\n";
    auto rec3 = orm.find("User", "usr_101");
    assert(rec3.has_value());
    assert(rec3->get_if<garuda::beam::List>() != nullptr);
    std::cout << "  ✓ Test 3: ORM Record Find PASS\n";

    // === Test 4: ORM Record Update ===
    std::cout << "[TEST 4/10] Running ORM Record Update test...\n";
    Term u1_upd = list_of({tuple_of({Term("nama"), Term("Budi Utomo")}), tuple_of({Term("email"), Term("budi@garuda.org")})});
    bool ok4 = orm.update("User", "usr_101", u1_upd);
    assert(ok4);
    auto rec4 = orm.find("User", "usr_101");
    assert(rec4.has_value());
    std::cout << "  ✓ Test 4: ORM Record Update PASS\n";

    // === Test 5: ORM Record Deletion ===
    std::cout << "[TEST 5/10] Running ORM Record Deletion test...\n";
    orm.insert("User", "usr_temp", Term("TempUser"));
    assert(orm.find("User", "usr_temp").has_value());
    bool ok5 = orm.remove("User", "usr_temp");
    assert(ok5);
    assert(!orm.find("User", "usr_temp").has_value());
    std::cout << "  ✓ Test 5: ORM Record Deletion PASS\n";

    // === Test 6: Query Builder 'where' Filtering ===
    std::cout << "[TEST 6/10] Running Query Builder 'where' Filtering test...\n";
    Term p1_rec = list_of({tuple_of({Term("title"), Term("Aplikasi Pertama")}), tuple_of({Term("user_id"), Term("usr_101")})});
    Term p2_rec = list_of({tuple_of({Term("title"), Term("Aplikasi Kedua")}), tuple_of({Term("user_id"), Term("usr_101")})});
    orm.insert("Post", "post_1", p1_rec);
    orm.insert("Post", "post_2", p2_rec);

    auto matches6 = orm.where("Post", "user_id", "usr_101");
    assert(matches6.size() == 2);
    std::cout << "  ✓ Test 6: Query Builder 'where' Filtering PASS\n";

    // === Test 7: Query Builder 'limit' Constraint ===
    std::cout << "[TEST 7/10] Running Query Builder 'limit' Constraint test...\n";
    auto matches7 = orm.where_limit("Post", "user_id", "usr_101", 1);
    assert(matches7.size() == 1);
    std::cout << "  ✓ Test 7: Query Builder 'limit' Constraint PASS\n";

    // === Test 8: Relationship Has-Many Query ===
    std::cout << "[TEST 8/10] Running Relationship Has-Many Query test...\n";
    auto posts8 = orm.has_many_query("User", "usr_101");
    assert(posts8.size() == 2);
    std::cout << "  ✓ Test 8: Relationship Has-Many Query PASS\n";

    // === Test 9: Relationship Belongs-To Query ===
    std::cout << "[TEST 9/10] Running Relationship Belongs-To Query test...\n";
    auto parent_user = orm.belongs_to_query("Post", "post_1");
    assert(parent_user.has_value());
    std::cout << "  ✓ Test 9: Relationship Belongs-To Query PASS\n";

    // === Test 10: Schema Migration Versioning ===
    std::cout << "[TEST 10/10] Running Schema Migration Versioning test...\n";
    bool ok10 = orm.migrate_add_column("User", "status", "string");
    assert(ok10);
    assert(orm.schema_version() == 2);
    std::cout << "  ✓ Test 10: Schema Migration Versioning PASS\n";

    // Cleanup
    fs::remove_all(tmp_db_file);

    std::cout << "\n🎉 ALL 10 NATIVE ORM TESTS PASSED PERFECTLY!\n";
    return 0;
}
