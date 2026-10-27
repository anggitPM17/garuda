#include "../include/garuda/beam/io/protobuf_minimal.hpp"
#include <chrono>
#include <future>
#include <iostream>

using namespace garuda::beam;

#define GARUDA_CHECK(cond, msg) \
    if (!(cond)) { \
        std::cerr << "  ❌ ASSERTION FAILED: " << msg << " (" << #cond << ")\n"; \
        std::exit(1); \
    }

void run_test_with_timeout(const std::string& test_name, std::function<void()> test_fn, int timeout_seconds = 5) {
    auto future = std::async(std::launch::async, test_fn);
    if (future.wait_for(std::chrono::seconds(timeout_seconds)) == std::future_status::timeout) {
        std::cerr << "  ❌ " << test_name << " FAILED (TIMEOUT > " << timeout_seconds << "s)\n";
        std::exit(1); \
    }
}

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA PROTOBUF MINIMAL WIRE FORMAT SUITE\n";
    std::cout << "=======================================================\n\n";

    // === Test 1: Varint Roundtrip ===
    run_test_with_timeout("Test 1: Varint Roundtrip", []() {
        ProtoWriter writer;
        writer.write_varint(1, 300);
        writer.write_varint(2, 150);
        writer.write_varint(3, 1000000);

        ProtoReader reader(writer.get_buffer());
        std::uint64_t v1 = reader.read_varint(1);
        std::uint64_t v2 = reader.read_varint(2);
        std::uint64_t v3 = reader.read_varint(3);

        GARUDA_CHECK(v1 == 300, "Varint field 1 must be 300");
        GARUDA_CHECK(v2 == 150, "Varint field 2 must be 150");
        GARUDA_CHECK(v3 == 1000000, "Varint field 3 must be 1000000");

        std::cout << "  ✓ Test 1: Varint Roundtrip PASS (300, 150, 1000000)\n";
    }, 5);

    // === Test 2: String Roundtrip ===
    run_test_with_timeout("Test 2: String Roundtrip", []() {
        ProtoWriter writer;
        writer.write_string(1, "Halo Garuda");

        ProtoReader reader(writer.get_buffer());
        std::string str = reader.read_string(1);

        GARUDA_CHECK(str == "Halo Garuda", "Decoded string must match 'Halo Garuda'");

        std::cout << "  ✓ Test 2: String Roundtrip PASS (Decoded: " << str << ")\n";
    }, 5);

    // === Test 3: Boolean Roundtrip ===
    run_test_with_timeout("Test 3: Boolean Roundtrip", []() {
        ProtoWriter writer;
        writer.write_bool(1, true);
        writer.write_bool(2, false);

        ProtoReader reader(writer.get_buffer());
        bool b1 = reader.read_bool(1);
        bool b2 = reader.read_bool(2);

        GARUDA_CHECK(b1 == true, "Field 1 bool must be true");
        GARUDA_CHECK(b2 == false, "Field 2 bool must be false");

        std::cout << "  ✓ Test 3: Boolean Roundtrip PASS (true, false)\n";
    }, 5);

    // === Test 4: Empty String Roundtrip ===
    run_test_with_timeout("Test 4: Empty String Roundtrip", []() {
        ProtoWriter writer;
        writer.write_string(2, "");

        ProtoReader reader(writer.get_buffer());
        std::string empty_str = reader.read_string(2);

        GARUDA_CHECK(empty_str.empty(), "Decoded string must be empty");

        std::cout << "  ✓ Test 4: Empty String Roundtrip PASS\n";
    }, 5);

    std::cout << "\n🎉 ALL PROTOBUF MINIMAL WIRE FORMAT TESTS PASSED PERFECTLY!\n";
    return 0;
}
