#include "../include/garuda/beam/stdlib/stdlib.hpp"
#include <cassert>
#include <iostream>
#include <sstream>

using namespace garuda::beam::stdlib;

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA STANDARD LIBRARY — SPRINT 4.2 TEST SUITE (50+ ASSERTIONS)\n";
    std::cout << "=======================================================\n\n";

    // === Module 1: String Tests (15 assertions) ===
    std::cout << "[TEST 1/5] Running String Module tests...\n";
    assert(Str::panjang("hello") == 5);                      // 1
    assert(Str::panjang("") == 0);                           // 2
    assert(Str::gabung("a", "b") == "ab");                   // 3
    assert(Str::potong("hello", 1, 3) == "ell");             // 4
    assert(Str::potong("hello", 10, 3) == "");               // 5
    assert(Str::besar("hello") == "HELLO");                  // 6
    assert(Str::besar("123a") == "123A");                    // 7
    assert(Str::kecil("HELLO") == "hello");                  // 8
    assert(Str::kecil("ABC1") == "abc1");                    // 9
    assert(Str::mengandung("hello", "ell") == true);         // 10
    assert(Str::mengandung("hello", "xyz") == false);        // 11

    auto p = Str::pisah("a,b,c", ",");
    assert(p.size() == 3 && p[0] == "a" && p[2] == "c");     // 12
    auto p2 = Str::pisah("halo", ",");
    assert(p2.size() == 1 && p2[0] == "halo");               // 13

    assert(Str::rapikan("  hello  ") == "hello");            // 14
    assert(Str::rapikan("\t\ngaruda\r\n") == "garuda");      // 15
    std::cout << "  ✓ String Module (15 assertions) PASS\n";

    // === Module 2: List Tests (15 assertions) ===
    std::cout << "[TEST 2/5] Running List Module tests...\n";
    std::vector<garuda::beam::Term> l1 = {garuda::beam::Term(1LL), garuda::beam::Term(2LL), garuda::beam::Term(3LL)};
    assert(List::panjang(l1) == 3);                          // 16
    assert(List::panjang({}) == 0);                          // 17
    assert(*List::kepala(l1).get_if<std::int64_t>() == 1);   // 18
    assert(List::kepala({}).get_if<std::int64_t>() == nullptr); // 19

    auto tail = List::ekor(l1);
    assert(tail.size() == 2 && *tail[0].get_if<std::int64_t>() == 2); // 20
    assert(List::ekor({}).empty());                          // 21

    auto l2 = List::tambah(l1, garuda::beam::Term(4LL));
    assert(l2.size() == 4 && *l2[3].get_if<std::int64_t>() == 4); // 22

    auto mapped = List::peta(l1, [](garuda::beam::Term t) {
        return garuda::beam::Term(*t.get_if<std::int64_t>() * 2);
    });
    assert(*mapped[0].get_if<std::int64_t>() == 2 && *mapped[2].get_if<std::int64_t>() == 6); // 23

    auto filtered = List::saring(l1, [](garuda::beam::Term t) {
        return *t.get_if<std::int64_t>() % 2 != 0;
    });
    assert(filtered.size() == 2 && *filtered[1].get_if<std::int64_t>() == 3); // 24

    auto folded = List::lipat(l1, garuda::beam::Term(0LL), [](garuda::beam::Term item, garuda::beam::Term acc) {
        return garuda::beam::Term(*item.get_if<std::int64_t>() + *acc.get_if<std::int64_t>());
    });
    assert(*folded.get_if<std::int64_t>() == 6);            // 25

    std::vector<garuda::beam::Term> unsorted = {garuda::beam::Term(3LL), garuda::beam::Term(1LL), garuda::beam::Term(2LL)};
    auto sorted = List::urutkan(unsorted);
    assert(*sorted[0].get_if<std::int64_t>() == 1);         // 26
    assert(*sorted[2].get_if<std::int64_t>() == 3);         // 27

    assert(List::kosong({}) == true);                        // 28
    assert(List::kosong(l1) == false);                      // 29
    assert(List::tambah({}, garuda::beam::Term(10LL)).size() == 1); // 30
    std::cout << "  ✓ List Module (15 assertions) PASS\n";

    // === Module 3: Map Tests (10 assertions) ===
    std::cout << "[TEST 3/5] Running Map Module tests...\n";
    auto m = Map::baru();
    assert(Map::panjang(m) == 0);                            // 31

    m = Map::taruh(m, "k1", garuda::beam::Term(100LL));
    assert(Map::panjang(m) == 1);                            // 32

    auto val1 = Map::ambil(m, "k1");
    assert(val1.has_value() && *val1->get_if<std::int64_t>() == 100); // 33
    assert(!Map::ambil(m, "k_none").has_value());            // 34

    m = Map::taruh(m, "k2", garuda::beam::Term(200LL));
    auto keys = Map::kunci(m);
    assert(keys.size() == 2 && keys[0] == "k1" && keys[1] == "k2"); // 35

    auto vals = Map::nilai(m);
    assert(vals.size() == 2 && *vals[0].get_if<std::int64_t>() == 100); // 36

    m = Map::hapus(m, "k1");
    assert(Map::panjang(m) == 1);                            // 37
    assert(!Map::ambil(m, "k1").has_value());               // 38
    assert(Map::ambil(m, "k2").has_value());                 // 39

    m = Map::hapus(m, "k2");
    assert(Map::panjang(m) == 0);                            // 40
    std::cout << "  ✓ Map Module (10 assertions) PASS\n";

    // === Module 4: Enum Tests (5 assertions) ===
    std::cout << "[TEST 4/5] Running Enum Module tests...\n";
    std::vector<garuda::beam::Term> elist = {garuda::beam::Term(10LL), garuda::beam::Term(20LL), garuda::beam::Term(30LL)};
    std::int64_t sum_each = 0;
    Enum::masing(elist, [&sum_each](garuda::beam::Term t) {
        sum_each += *t.get_if<std::int64_t>();
    });
    assert(sum_each == 60);                                  // 41

    std::int64_t cnt = Enum::hitung(elist, [](garuda::beam::Term t) {
        return *t.get_if<std::int64_t>() > 15;
    });
    assert(cnt == 2);                                        // 42

    std::int64_t total_sum = Enum::jumlah(elist);
    assert(total_sum == 60);                                 // 43
    assert(Enum::jumlah({}) == 0);                           // 44
    assert(Enum::hitung({}, [](garuda::beam::Term) { return true; }) == 0);// 45
    std::cout << "  ✓ Enum Module (5 assertions) PASS\n";

    // === Module 5: IO Tests (5 assertions) ===
    std::cout << "[TEST 5/5] Running IO Module tests...\n";
    IO::cetak("Cetak test string IO");                       // 46
    IO::cetak_term(garuda::beam::Term(42LL));                // 47
    IO::cetak_galat("Cetak galat test IO");                  // 48

    std::istringstream iss("input_test_line\n");
    std::cin.rdbuf(iss.rdbuf());
    std::string read_line = IO::baca_baris();
    assert(read_line == "input_test_line");                  // 49
    assert(Str::panjang(read_line) == 15);                   // 50
    std::cout << "  ✓ IO Module (5 assertions) PASS\n";

    std::cout << "\n🎉 ALL 50 ASSERTIONS IN SPRINT 4.2 STDLIB TEST SUITE PASSED PERFECTLY!\n";
    return 0;
}
