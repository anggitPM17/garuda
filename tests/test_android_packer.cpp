#include <iostream>
#include <fstream>
#include <cassert>
#include <vector>
#include <cstdio>

#include "garuda/mobile/android_packer.hpp"

using namespace garuda::mobile;

void test_pack_and_extract_apk_assets() {
    std::string asset_path = "test_app.grdbc";

    AndroidPackerConfig input_cfg;
    input_cfg.app_id = "org.richat.android";
    input_cfg.target_abi = "arm64-v8a";
    input_cfg.version_code = 67;
    input_cfg.version_name = "67.0.0-mobile";

    std::vector<uint8_t> original_bytecode = {0x01, 0x02, 0x03, 0xAA, 0xBB, 0xCC, 0xFF};

    bool pack_ok = GvmAndroidPacker::pack_apk_assets(original_bytecode, input_cfg, asset_path);
    assert(pack_ok && "GvmAndroidPacker::pack_apk_assets failed");
    (void)pack_ok;

    std::vector<uint8_t> extracted_bytecode;
    AndroidPackerConfig output_cfg;
    bool extract_ok = GvmAndroidPacker::extract_apk_assets(asset_path, extracted_bytecode, output_cfg);
    assert(extract_ok && "GvmAndroidPacker::extract_apk_assets failed");
    (void)extract_ok;

    assert(output_cfg.app_id == input_cfg.app_id);
    assert(output_cfg.target_abi == input_cfg.target_abi);
    assert(output_cfg.version_code == input_cfg.version_code);
    assert(output_cfg.version_name == input_cfg.version_name);
    assert(extracted_bytecode == original_bytecode);

    std::remove(asset_path.c_str());
    std::cout << "[PASS] test_pack_and_extract_apk_assets" << std::endl;
}

void test_corrupted_checksum_rejection() {
    std::string asset_path = "test_corrupt.grdbc";

    AndroidPackerConfig cfg;
    std::vector<uint8_t> bytecode = {0x10, 0x20, 0x30, 0x40};

    GvmAndroidPacker::pack_apk_assets(bytecode, cfg, asset_path);

    // Corrupt last byte
    std::fstream f(asset_path, std::ios::in | std::ios::out | std::ios::binary);
    f.seekp(-1, std::ios::end);
    f.put(0x00);
    f.close();

    std::vector<uint8_t> extracted_bytecode;
    AndroidPackerConfig output_cfg;
    bool extract_ok = GvmAndroidPacker::extract_apk_assets(asset_path, extracted_bytecode, output_cfg);
    assert(!extract_ok && "extract_apk_assets should return false for corrupted checksum");
    (void)extract_ok;

    std::remove(asset_path.c_str());
    std::cout << "[PASS] test_corrupted_checksum_rejection" << std::endl;
}

int main() {
    std::cout << "--- RUNNING ANDROID NDK PACKER TESTS ---" << std::endl;
    test_pack_and_extract_apk_assets();
    test_corrupted_checksum_rejection();
    std::cout << "--- ALL ANDROID NDK PACKER TESTS PASSED 100% ---" << std::endl;
    return 0;
}
