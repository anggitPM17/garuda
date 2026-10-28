#include "../include/garuda/jalur_langit/query_cache.hpp"
#include <iostream>
#include <thread>
#include <cstdlib>

using namespace garuda::jalur_langit;
#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

int main() {
    std::cout << "=== QUERY CACHE TESTS ===\n";

    QueryCache cache(std::chrono::seconds(1));

    QueryResult res;
    res.success = true;
    res.affected_rows = 42;

    cache.put("SELECT * FROM users", res);
    auto cached = cache.get("SELECT * FROM users");
    CHECK(cached.has_value(), "Cache harus hit");
    CHECK(cached->affected_rows == 42, "Cache harus berisi data yang sama");
    std::cout << "PASS: Cache Hit bekerja.\n";

    auto miss = cache.get("SELECT * FROM nothing");
    CHECK(!miss.has_value(), "Cache harus miss");
    std::cout << "PASS: Cache Miss bekerja.\n";

    std::this_thread::sleep_for(std::chrono::milliseconds(1100));
    auto expired = cache.get("SELECT * FROM users");
    CHECK(!expired.has_value(), "Cache harus expired setelah TTL");
    std::cout << "PASS: Cache TTL kadaluarsa.\n";

    std::cout << "QUERY CACHE TESTS PASSED!\n";
    return 0;
}
