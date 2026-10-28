// =====================================================================
// FASE G3.1 - GARUDA MNESIA TESTS
// Memverifikasi mnesia.hpp setara Erlang/OTP Mnesia
// =====================================================================
#include "../include/garuda/beam/otp/mnesia.hpp"
#include <iostream>
#include <filesystem>
#include <cstdlib>
#include <system_error>

using namespace garuda::beam;

#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

int main() {
    std::cout << "========================================\n";
    std::cout << "FASE G3.1 - GARUDA MNESIA TESTS\n";
    std::cout << "========================================\n\n";

    std::string test_dir = "./test_mnesia_db";
    std::error_code ec;
    std::filesystem::remove_all(test_dir, ec);

    {
        Mnesia mnesia(test_dir);

        // Test 1: Create Table
        std::cout << "[Test 1] Create Table...\n";
        {
            TableConfig cfg{"users", TableType::Set, StorageType::DiscCopies, {"id", "name", "email"}};
            bool created = mnesia.create_table(cfg);
            CHECK(created, "Table users harus berhasil dibuat");

            bool duplicate = mnesia.create_table(cfg);
            CHECK(!duplicate, "Table users duplikat harus ditolak");
        }
        std::cout << "PASS\n\n";

        // Test 2: Write & Read
        std::cout << "[Test 2] Write & Read...\n";
        {
            mnesia.write("users", Term(std::string("u100")), Term(std::string("Budi")));
            auto val = mnesia.read("users", Term(std::string("u100")));
            CHECK(val.has_value(), "Read u100 harus menemukan data");
            const std::string* str = val->get_if<std::string>();
            CHECK(str && *str == "Budi", "Nama user harus Budi");
        }
        std::cout << "PASS\n\n";

        // Test 3: Delete Key
        std::cout << "[Test 3] Delete Key...\n";
        {
            mnesia.write("users", Term(std::string("u200")), Term(std::string("Siti")));
            CHECK(mnesia.read("users", Term(std::string("u200"))).has_value(), "u200 harus ada sebelum delete");

            bool deleted = mnesia.delete_key("users", Term(std::string("u200")));
            CHECK(deleted, "delete_key u200 harus return true");
            CHECK(!mnesia.read("users", Term(std::string("u200"))).has_value(), "u200 harus std::nullopt setelah delete");
        }
        std::cout << "PASS\n\n";

        // Test 4: Select Query
        std::cout << "[Test 4] Select Query...\n";
        {
            mnesia.write("users", Term(std::string("p1")), Term(static_cast<std::int64_t>(10)));
            mnesia.write("users", Term(std::string("p2")), Term(static_cast<std::int64_t>(20)));
            mnesia.write("users", Term(std::string("p3")), Term(static_cast<std::int64_t>(30)));

            auto results = mnesia.select("users", [](const Term& /*key*/, const Term& val) {
                const std::int64_t* v = val.get_if<std::int64_t>();
                return v && *v >= 20;
            });

            CHECK(results.size() == 2, "Select dengan val >= 20 harus mengembalikan 2 rows");
        }
        std::cout << "PASS\n\n";

        // Test 5: Richat Schema Pre-Initialization
        std::cout << "[Test 5] Richat Schema Pre-Initialization...\n";
        {
            Mnesia richat_mnesia(test_dir + "_richat");
            richat_mnesia.init_richat_schema();

            richat_mnesia.write("richat_user", Term(std::string("usr_99")), Term(std::string("agaruda@richat.id")));
            auto res = richat_mnesia.read("richat_user", Term(std::string("usr_99")));
            CHECK(res.has_value(), "Read richat_user usr_99 harus berhasil");
        }
        std::cout << "PASS\n\n";
    }

    std::filesystem::remove_all(test_dir, ec);
    std::filesystem::remove_all(test_dir + "_richat", ec);

    std::cout << "========================================\n";
    std::cout << "FASE G3.1 - GARUDA MNESIA SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
