#include "../include/garuda/jalur_langit/schema_migrator.hpp"
#include "../include/garuda/jalur_langit/jalur_langit_ultimate.hpp"
#include <iostream>
#include <filesystem>
#include <cstdlib>
#include <system_error>

using namespace garuda::jalur_langit;
#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

int main() {
    std::string dir = "./test_migrator_data";
    std::error_code ec;
    std::filesystem::remove_all(dir, ec);

    {
        JalurLangitUltimate db(dir);

        SchemaMigrator migrator;
        migrator.add_migration(1, "Buat tabel users", [](JalurLangitUltimate& db) {
            db.create_table("users", {{"id", "INT", true}, {"nama", "STRING", false}});
        });
        migrator.add_migration(2, "Buat tabel posts", [](JalurLangitUltimate& db) {
            db.create_table("posts", {{"id", "INT", true}, {"title", "STRING", false}});
        });

        migrator.run_migrations(db);
        CHECK(migrator.current_version() == 2, "Versi harus 2 setelah migrasi");
    }

    std::filesystem::remove_all(dir, ec);
    std::cout << "SCHEMA MIGRATOR TESTS PASSED!\n";
    return 0;
}
