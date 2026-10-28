// =====================================================================
// FASE G3.3 - GARUDA MNESIA SWEEP TESTS
// Memverifikasi mnesia_sweep.hpp
// =====================================================================
#include "../include/garuda/beam/otp/mnesia_sweep.hpp"
#include <iostream>
#include <string>
#include <cstdlib>

using namespace garuda::beam;

#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

int main() {
    std::cout << "========================================\n";
    std::cout << "FASE G3.3 - GARUDA MNESIA SWEEP TESTS\n";
    std::cout << "========================================\n\n";

    MnesiaSweep ms;

    // Test 1: Write Permanent Row
    std::cout << "[Test 1] Write Permanent Row...\n";
    {
        ms.write("perm_1", "Permanent Data", 0);
        auto val = ms.read("perm_1");
        CHECK(val.has_value() && *val == "Permanent Data", "Read permanent row harus berhasil");
        CHECK(ms.total_rows() == 1, "Total rows harus 1");
        CHECK(ms.expired_rows() == 0, "Expired rows harus 0");
    }
    std::cout << "PASS\n\n";

    // Test 2: Write with Valid TTL
    std::cout << "[Test 2] Write with Valid TTL...\n";
    {
        ms.write("temp_1", "Live Data", 60000); // TTL 60s
        auto val = ms.read("temp_1");
        CHECK(val.has_value() && *val == "Live Data", "Read live temp row harus berhasil");
    }
    std::cout << "PASS\n\n";

    // Test 3: Read Expired Row Returns Nullopt
    std::cout << "[Test 3] Read Expired Row Returns Nullopt...\n";
    {
        ms.write_expired("exp_1", "Old Data");
        auto val = ms.read("exp_1");
        CHECK(!val.has_value(), "Read expired row harus std::nullopt");
        CHECK(ms.expired_rows() == 1, "Expired rows count harus 1");
    }
    std::cout << "PASS\n\n";

    // Test 4: Sweep Expired Rows
    std::cout << "[Test 4] Sweep Expired Rows...\n";
    {
        ms.write_expired("exp_2", "Old Data 2");
        CHECK(ms.expired_rows() == 2, "Expired rows count = 2 sebelum sweep");

        std::size_t removed = ms.sweep();
        CHECK(removed == 2, "Sweep harus menghapus 2 expired rows");
        CHECK(ms.expired_rows() == 0, "Expired rows count = 0 setelah sweep");
        CHECK(ms.total_rows() == 2, "Total rows tersisa harus 2 (perm_1 dan temp_1)");
    }
    std::cout << "PASS\n\n";

    // Test 5: Sweep Statistics
    std::cout << "[Test 5] Sweep Statistics...\n";
    {
        CHECK(ms.sweep_count() == 1, "Sweep count harus 1x");
        ms.sweep();
        CHECK(ms.sweep_count() == 2, "Sweep count harus 2x setelah sweep kedua");
    }
    std::cout << "PASS\n\n";

    std::cout << "========================================\n";
    std::cout << "FASE G3.3 - MNESIA SWEEP SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
