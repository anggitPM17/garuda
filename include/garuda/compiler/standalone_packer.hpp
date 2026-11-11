#ifndef GARUDA_COMPILER_STANDALONE_PACKER_HPP
#define GARUDA_COMPILER_STANDALONE_PACKER_HPP

#include <string>
#include <vector>
#include <fstream>
#include <cstring>
#include <cstdint>
#include <iostream>

namespace garuda {
namespace compiler {

constexpr const char* GARUDA_STUB_MAGIC = "GARUDA_STUB_V1_FOOTER";
constexpr size_t GARUDA_MAGIC_LEN = 20; // "GARUDA_STUB_V1_FOOTER" length without null terminator

class StandalonePacker {
public:
    static bool pack_executable(const std::string& stub_path,
                               const std::vector<uint8_t>& bytecode,
                               const std::string& output_path) {
        std::ifstream stub_file(stub_path, std::ios::binary | std::ios::ate);
        if (!stub_file.is_open()) {
            std::cerr << "[StandalonePacker] Error: Cannot open stub binary at " << stub_path << std::endl;
            return false;
        }

        std::streamsize stub_size = stub_file.tellg();
        stub_file.seekg(0, std::ios::beg);

        std::vector<uint8_t> stub_data(static_cast<size_t>(stub_size));
        if (!stub_file.read(reinterpret_cast<char*>(stub_data.data()), stub_size)) {
            std::cerr << "[StandalonePacker] Error: Failed to read stub binary." << std::endl;
            return false;
        }
        stub_file.close();

        std::ofstream out_file(output_path, std::ios::binary);
        if (!out_file.is_open()) {
            std::cerr << "[StandalonePacker] Error: Cannot create output executable at " << output_path << std::endl;
            return false;
        }

        // 1. Write stub binary
        out_file.write(reinterpret_cast<const char*>(stub_data.data()), stub_data.size());

        // 2. Write embedded GVM bytecode
        out_file.write(reinterpret_cast<const char*>(bytecode.data()), bytecode.size());

        // 3. Write 64-bit payload size
        uint64_t payload_size = static_cast<uint64_t>(bytecode.size());
        out_file.write(reinterpret_cast<const char*>(&payload_size), sizeof(payload_size));

        // 4. Write magic signature
        out_file.write(GARUDA_STUB_MAGIC, GARUDA_MAGIC_LEN);

        out_file.close();
        return true;
    }

    static bool extract_embedded_bytecode(const std::string& self_path, std::vector<uint8_t>& out_bytecode) {
        std::ifstream self_file(self_path, std::ios::binary | std::ios::ate);
        if (!self_file.is_open()) {
            return false;
        }

        std::streamsize total_size = self_file.tellg();
        constexpr size_t footer_size = sizeof(uint64_t) + GARUDA_MAGIC_LEN; // 8 + 20 = 28 bytes

        if (total_size < static_cast<std::streamsize>(footer_size)) {
            return false;
        }

        // Read footer
        self_file.seekg(total_size - static_cast<std::streamsize>(footer_size), std::ios::beg);

        uint64_t payload_size = 0;
        self_file.read(reinterpret_cast<char*>(&payload_size), sizeof(payload_size));

        char magic_buf[GARUDA_MAGIC_LEN + 1] = {0};
        self_file.read(magic_buf, GARUDA_MAGIC_LEN);

        if (std::memcmp(magic_buf, GARUDA_STUB_MAGIC, GARUDA_MAGIC_LEN) != 0) {
            return false; // Not a packed standalone binary
        }

        if (total_size < static_cast<std::streamsize>(footer_size + payload_size)) {
            return false; // Corrupted payload
        }

        // Extract payload
        self_file.seekg(total_size - static_cast<std::streamsize>(footer_size + payload_size), std::ios::beg);
        out_bytecode.resize(static_cast<size_t>(payload_size));
        self_file.read(reinterpret_cast<char*>(out_bytecode.data()), payload_size);

        return true;
    }
};

} // namespace compiler
} // namespace garuda

#endif // GARUDA_COMPILER_STANDALONE_PACKER_HPP
