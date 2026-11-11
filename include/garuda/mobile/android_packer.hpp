#ifndef GARUDA_MOBILE_ANDROID_PACKER_HPP
#define GARUDA_MOBILE_ANDROID_PACKER_HPP

#include <string>
#include <vector>
#include <fstream>
#include <cstring>
#include <cstdint>
#include <iostream>

namespace garuda {
namespace mobile {

constexpr const char* GARUDA_ANDROID_MAGIC = "GARUDA_APK_ASSET_V1";
constexpr size_t GARUDA_ANDROID_MAGIC_LEN = 19; // "GARUDA_APK_ASSET_V1" without null terminator

struct AndroidPackerConfig {
    std::string app_id = "com.garuda.app";
    std::string target_abi = "arm64-v8a"; // arm64-v8a, armeabi-v7a, x86_64
    uint32_t version_code = 1;
    std::string version_name = "1.0.0";
};

class GvmAndroidPacker {
public:
    static bool pack_apk_assets(const std::vector<uint8_t>& bytecode,
                                const AndroidPackerConfig& config,
                                const std::string& output_asset_path) {
        std::ofstream out_file(output_asset_path, std::ios::binary);
        if (!out_file.is_open()) {
            std::cerr << "[GvmAndroidPacker] Error: Cannot create asset file at " << output_asset_path << std::endl;
            return false;
        }

        // 1. Write Magic Header
        out_file.write(GARUDA_ANDROID_MAGIC, GARUDA_ANDROID_MAGIC_LEN);

        // 2. Write Config Metadata (Lengths + Strings)
        auto write_string = [&](const std::string& s) {
            uint32_t len = static_cast<uint32_t>(s.size());
            out_file.write(reinterpret_cast<const char*>(&len), sizeof(len));
            if (len > 0) {
                out_file.write(s.data(), len);
            }
        };

        write_string(config.app_id);
        write_string(config.target_abi);
        out_file.write(reinterpret_cast<const char*>(&config.version_code), sizeof(config.version_code));
        write_string(config.version_name);

        // 3. Calculate simple Checksum
        uint32_t checksum = 0x811C9DC5; // FNV-1a init
        for (uint8_t b : bytecode) {
            checksum ^= b;
            checksum *= 16777619u;
        }
        out_file.write(reinterpret_cast<const char*>(&checksum), sizeof(checksum));

        // 4. Write Bytecode Payload Length + Data
        uint64_t payload_len = static_cast<uint64_t>(bytecode.size());
        out_file.write(reinterpret_cast<const char*>(&payload_len), sizeof(payload_len));
        if (payload_len > 0) {
            out_file.write(reinterpret_cast<const char*>(bytecode.data()), payload_len);
        }

        out_file.close();
        return true;
    }

    static bool extract_apk_assets(const std::string& asset_path,
                                   std::vector<uint8_t>& out_bytecode,
                                   AndroidPackerConfig& out_config) {
        std::ifstream in_file(asset_path, std::ios::binary);
        if (!in_file.is_open()) {
            return false;
        }

        // 1. Check Magic Header
        char magic_buf[GARUDA_ANDROID_MAGIC_LEN + 1] = {0};
        in_file.read(magic_buf, GARUDA_ANDROID_MAGIC_LEN);
        if (std::memcmp(magic_buf, GARUDA_ANDROID_MAGIC, GARUDA_ANDROID_MAGIC_LEN) != 0) {
            return false; // Invalid magic
        }

        // 2. Read Config Metadata
        auto read_string = [&](std::string& s) -> bool {
            uint32_t len = 0;
            if (!in_file.read(reinterpret_cast<char*>(&len), sizeof(len))) return false;
            if (len > 0) {
                s.resize(len);
                if (!in_file.read(&s[0], len)) return false;
            } else {
                s.clear();
            }
            return true;
        };

        if (!read_string(out_config.app_id)) return false;
        if (!read_string(out_config.target_abi)) return false;
        if (!in_file.read(reinterpret_cast<char*>(&out_config.version_code), sizeof(out_config.version_code))) return false;
        if (!read_string(out_config.version_name)) return false;

        // 3. Read Checksum
        uint32_t expected_checksum = 0;
        if (!in_file.read(reinterpret_cast<char*>(&expected_checksum), sizeof(expected_checksum))) return false;

        // 4. Read Payload
        uint64_t payload_len = 0;
        if (!in_file.read(reinterpret_cast<char*>(&payload_len), sizeof(payload_len))) return false;

        out_bytecode.resize(static_cast<size_t>(payload_len));
        if (payload_len > 0) {
            if (!in_file.read(reinterpret_cast<char*>(out_bytecode.data()), payload_len)) return false;
        }

        // 5. Verify Checksum
        uint32_t actual_checksum = 0x811C9DC5;
        for (uint8_t b : out_bytecode) {
            actual_checksum ^= b;
            actual_checksum *= 16777619u;
        }

        if (actual_checksum != expected_checksum) {
            return false; // Corrupted payload
        }

        return true;
    }
};

} // namespace mobile
} // namespace garuda

#endif // GARUDA_MOBILE_ANDROID_PACKER_HPP
