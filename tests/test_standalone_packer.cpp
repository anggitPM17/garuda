#include <iostream>
#include <fstream>
#include <cassert>
#include <vector>
#include <string>
#include <cstdio>

#include "garuda/compiler/standalone_packer.hpp"

using namespace garuda::compiler;

void test_pack_and_extract() {
    std::string stub_path = "test_stub.bin";
    std::string packed_path = "test_packed.exe";

    // 1. Create a dummy stub file
    {
        std::ofstream f(stub_path, std::ios::binary);
        f << "STUB_EXECUTABLE_BINARY_DATA_HEADER";
    }

    // 2. Sample GVM bytecode
    std::vector<uint8_t> original_bytecode = {0x47, 0x56, 0x4D, 0x01, 0x10, 0x20, 0x30, 0x40, 0xFF};

    // 3. Pack executable
    bool pack_ok = StandalonePacker::pack_executable(stub_path, original_bytecode, packed_path);
    assert(pack_ok && "StandalonePacker::pack_executable failed");
    (void)pack_ok;

    // 4. Extract bytecode
    std::vector<uint8_t> extracted_bytecode;
    bool extract_ok = StandalonePacker::extract_embedded_bytecode(packed_path, extracted_bytecode);
    assert(extract_ok && "StandalonePacker::extract_embedded_bytecode failed");
    (void)extract_ok;

    // 5. Verify byte parity
    assert(extracted_bytecode.size() == original_bytecode.size() && "Bytecode size mismatch");
    assert(extracted_bytecode == original_bytecode && "Extracted bytecode content mismatch");

    // Clean up
    std::remove(stub_path.c_str());
    std::remove(packed_path.c_str());

    std::cout << "[PASS] test_pack_and_extract" << std::endl;
}

void test_invalid_magic() {
    std::string plain_path = "test_plain.bin";

    {
        std::ofstream f(plain_path, std::ios::binary);
        f << "THIS_IS_A_PLAIN_BINARY_WITHOUT_FOOTER_1234567890";
    }

    std::vector<uint8_t> extracted_bytecode;
    bool extract_ok = StandalonePacker::extract_embedded_bytecode(plain_path, extracted_bytecode);
    assert(!extract_ok && "extract_embedded_bytecode should return false for binary without valid footer magic");
    (void)extract_ok;

    std::remove(plain_path.c_str());
    std::cout << "[PASS] test_invalid_magic" << std::endl;
}

void test_small_file() {
    std::string small_path = "test_small.bin";

    {
        std::ofstream f(small_path, std::ios::binary);
        f << "TINY";
    }

    std::vector<uint8_t> extracted_bytecode;
    bool extract_ok = StandalonePacker::extract_embedded_bytecode(small_path, extracted_bytecode);
    assert(!extract_ok && "extract_embedded_bytecode should return false for small file");
    (void)extract_ok;

    std::remove(small_path.c_str());
    std::cout << "[PASS] test_small_file" << std::endl;
}

int main() {
    std::cout << "--- RUNNING STANDALONE PACKER TESTS ---" << std::endl;
    test_pack_and_extract();
    test_invalid_magic();
    test_small_file();
    std::cout << "--- ALL STANDALONE PACKER TESTS PASSED 100% ---" << std::endl;
    return 0;
}
