#include "../include/garuda/jalur_langit/connection_pool.hpp"
#include <iostream>
#include <filesystem>
#include <cstdlib>
#include <system_error>

using namespace garuda::jalur_langit;
#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

int main() {
    std::cout << "=== CONNECTION POOL TESTS ===\n";
    std::string dir = "./test_pool_data";
    std::error_code ec;
    std::filesystem::remove_all(dir, ec);

    {
        ConnectionPool pool(3, dir);
        std::cout << "[1] Pool dengan 3 koneksi dibuat.\n";

        auto c1 = pool.acquire();
        auto c2 = pool.acquire();
        auto c3 = pool.acquire();
        CHECK(c1 && c2 && c3, "3 koneksi harus bisa dipinjam");
        CHECK(pool.available_connections() == 0, "Pool harus kosong");
        std::cout << "PASS: 3 koneksi dipinjam bersamaan.\n";

        pool.release(c1);
        CHECK(pool.available_connections() == 1, "1 koneksi kembali ke pool");
        std::cout << "PASS: 1 koneksi dikembalikan.\n";

        pool.release(c2);
        pool.release(c3);
        CHECK(pool.available_connections() == 3, "3 koneksi kembali ke pool");
        std::cout << "PASS: 3 koneksi dikembalikan.\n";
    }

    std::filesystem::remove_all(dir, ec);
    std::cout << "CONNECTION POOL TESTS PASSED!\n";
    return 0;
}
