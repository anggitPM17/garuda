// =====================================================================
// FASE D.2.3 - GARUDA NATIVE BINDING TESTS
// Memverifikasi native_binding.hpp
// =====================================================================
#include "../include/garuda/ffi/native_binding.hpp"
#include <iostream>
#include <string>
#include <cstdlib>

using namespace garuda::ffi;

#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

int main() {
    std::cout << "========================================\n";
    std::cout << "FASE D.2.3 - NATIVE BINDING TESTS\n";
    std::cout << "========================================\n\n";

    // Test 1: CurlBinding Availability Check
    std::cout << "[Test 1] CurlBinding Availability Check...\n";
    {
        bool avail = CurlBinding::is_available();
        std::cout << "  libcurl available: " << (avail ? "YES" : "NO") << "\n";
    }
    std::cout << "PASS\n\n";

    // Test 2: CurlBinding HTTP GET
    std::cout << "[Test 2] CurlBinding HTTP GET...\n";
    {
        auto res = CurlBinding::http_get("https://richat.my.id");
        if (CurlBinding::is_available()) {
            CHECK(res.has_value(), "http_get harus return response saat libcurl tersedia");
        } else {
            CHECK(!res.has_value(), "http_get harus return nullopt saat libcurl tidak tersedia");
        }
    }
    std::cout << "PASS\n\n";

    // Test 3: ZlibBinding Availability Check
    std::cout << "[Test 3] ZlibBinding Availability Check...\n";
    {
        bool avail = ZlibBinding::is_available();
        std::cout << "  zlib available: " << (avail ? "YES" : "NO") << "\n";
    }
    std::cout << "PASS\n\n";

    // Test 4: ZlibBinding Compress & Decompress
    std::cout << "[Test 4] ZlibBinding Compress & Decompress...\n";
    {
        std::vector<uint8_t> raw = {'G', 'A', 'R', 'U', 'D', 'A'};
        auto compressed = ZlibBinding::compress(raw);

        if (ZlibBinding::is_available()) {
            CHECK(!compressed.empty(), "Compressed vector tidak boleh kosong saat zlib tersedia");
            auto decompressed = ZlibBinding::decompress(compressed);
            CHECK(decompressed.has_value() && *decompressed == raw, "Decompressed vector harus match dengan raw");
        } else {
            CHECK(compressed.empty(), "Compressed vector harus empty saat zlib tidak tersedia");
        }
    }
    std::cout << "PASS\n\n";

    // Test 5: Empty Input Compression Rejection
    std::cout << "[Test 5] Empty Input Compression Rejection...\n";
    {
        auto compressed = ZlibBinding::compress({});
        CHECK(compressed.empty(), "Compress vector kosong harus return vector kosong");
    }
    std::cout << "PASS\n\n";

    std::cout << "========================================\n";
    std::cout << "FASE D.2.3 - NATIVE BINDING SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
