// =====================================================================
// GARUDA COMPILER — FASE 16: MNESIA QUERY DSL & TRANSACTION TEST SUITE
// =====================================================================

#include "../include/garuda/beam/otp/mnesia_query.hpp"
#include <iostream>
#include <cassert>
#include <filesystem>

using namespace garuda::beam;
namespace fs = std::filesystem;

int main() {
    int failures = 0;

    std::cout << "=======================================================\n";
    std::cout << "GARUDA COMPILER — FASE 16: MNESIA QUERY & TX SUITE\n";
    std::cout << "=======================================================\n\n";

    fs::path tmp_db = fs::temp_directory_path() / "garuda_mnesia_query_test_db";
    fs::remove_all(tmp_db);

    TableConfig cfg;
    cfg.name = "pengguna";
    cfg.type = TableType::Set;
    cfg.storage = StorageType::DiscCopies;
    cfg.fields = {"id", "nama", "umur"};

    MnesiaTable tbl(cfg, tmp_db.string());

    // Insert test data
    tbl.write(Term(std::string("u1")), Term(int64_t(25)));
    tbl.write(Term(std::string("u2")), Term(int64_t(17)));
    tbl.write(Term(std::string("u3")), Term(int64_t(30)));

    // Test 1: Query DSL Saring (Filter > 18)
    {
        MnesiaQuery q(tbl);
        auto res = q.saring([](const Term& key, const Term& val) {
            if (auto* i = val.get_if<int64_t>()) {
                return *i > 18;
            }
            return false;
        }).eksekusi();

        if (res.size() == 2) {
            std::cout << "[Test 1] Query DSL Saring (Umur > 18) -> Returned 2 Rows [PASS]\n";
        } else {
            std::cout << "[Test 1] Query DSL Saring -> [FAIL] got size: " << res.size() << "\n";
            failures++;
        }
    }

    // Test 2: Query DSL Limit & Sort
    {
        MnesiaQuery q(tbl);
        auto res = q.saring([](const Term& k, const Term& v) { return true; })
                    .urutkan([](const Term& a, const Term& b) {
                        auto* ia = a.get_if<int64_t>();
                        auto* ib = b.get_if<int64_t>();
                        if (ia && ib) return *ia > *ib;
                        return false;
                    })
                    .limit(1)
                    .eksekusi();

        if (res.size() == 1 && res[0].get_if<int64_t>() && *res[0].get_if<int64_t>() == 30) {
            std::cout << "[Test 2] Query DSL Urutkan & Limit -> Highest Umur (30) [PASS]\n";
        } else {
            std::cout << "[Test 2] Query DSL Urutkan & Limit -> [FAIL]\n";
            failures++;
        }
    }

    // Test 3: ACID Transaction Commit
    {
        bool success = MnesiaTransaction::jalankan_transaksi(tbl, [](MnesiaTransaction& tx) {
            tx.write(Term(std::string("u4")), Term(int64_t(40)));
            return true; // Commit transaction
        });

        auto read_res = tbl.read(Term(std::string("u4")));
        if (success && read_res.has_value() && read_res->get_if<int64_t>() && *read_res->get_if<int64_t>() == 40) {
            std::cout << "[Test 3] ACID Transaction Commit -> 'u4' = 40 Persisted [PASS]\n";
        } else {
            std::cout << "[Test 3] ACID Transaction Commit -> [FAIL]\n";
            failures++;
        }
    }

    // Test 4: ACID Transaction Rollback
    {
        bool success = MnesiaTransaction::jalankan_transaksi(tbl, [](MnesiaTransaction& tx) {
            tx.write(Term(std::string("u5")), Term(int64_t(50)));
            return false; // Abort & rollback transaction
        });

        auto read_res = tbl.read(Term(std::string("u5")));
        if (!success && !read_res.has_value()) {
            std::cout << "[Test 4] ACID Transaction Rollback -> 'u5' Rollbacked [PASS]\n";
        } else {
            std::cout << "[Test 4] ACID Transaction Rollback -> [FAIL]\n";
            failures++;
        }
    }

    // Cleanup
    fs::remove_all(tmp_db);

    if (failures > 0) {
        std::cout << "\n❌ " << failures << " MNESIA QUERY TESTS FAILED!\n";
        return 1;
    }

    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL FASE 16 MNESIA QUERY & TX TESTS PASSED PERFECTLY!\n";
    std::cout << "========================================\n";
    return 0;
}
