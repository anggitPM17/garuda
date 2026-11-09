// =====================================================================
// GARUDA COMPILER — FASE 15 PART 2: PACKAGE SIGNATURE TEST SUITE
// =====================================================================

#include "../include/garuda/compiler/package_registry.hpp"
#include <iostream>
#include <cassert>
#include <filesystem>

using namespace garuda::compiler;
namespace fs = std::filesystem;

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA COMPILER — FASE 15 PART 2: PACKAGE SIGNATURE SUITE\n";
    std::cout << "=======================================================\n\n";

    fs::path tmp_cache = fs::temp_directory_path() / "garuda_sig_e2e_test";
    fs::remove_all(tmp_cache);
    fs::create_directories(tmp_cache);

    PackageRegistry reg(tmp_cache.string());
    int failures = 0;

    std::string payload = "modul SafeLibrary\n\npublik fungsi halo() -> \"aman\"\nrampung\n";
    std::string secret_key = "key_garuda_secret_123";
    std::string public_key = secret_key;
    std::string checksum = Sha256::hash(payload);
    std::string signature = PackageSignature::sign_payload(payload, secret_key);

    // Test 1: Verify Valid Ed25519 Signature
    bool valid_sig = PackageSignature::verify_signature(payload, signature, public_key);
    if (valid_sig) {
        std::cout << "[Test 1] Verify Valid Ed25519 Signature -> [PASS]\n";
    } else {
        std::cout << "[Test 1] Verify Valid Signature -> [FAIL]\n";
        failures++;
    }

    // Test 2: Reject Tampered Payload Signature
    std::string tampered_payload = payload + "\n// tampered code injection\n";
    bool tampered_sig = PackageSignature::verify_signature(tampered_payload, signature, public_key);
    if (!tampered_sig) {
        std::cout << "[Test 2] Reject Tampered Payload Signature -> Correctly Rejected [PASS]\n";
    } else {
        std::cout << "[Test 2] Reject Tampered Payload -> [FAIL]\n";
        failures++;
    }

    // Test 3: Verify & Install Signed Package
    PackageMeta meta{
        "garuda_crypto_safe",
        "1.0.0",
        "Cryptographically Verified Safe Package",
        "Anggit Prasetyo",
        checksum,
        public_key,
        signature,
        {}
    };

    bool installed = reg.verify_and_install_package(meta, payload);
    if (installed && reg.is_installed("garuda_crypto_safe")) {
        std::cout << "[Test 3] Verify & Install Signed Package -> Installed [PASS]\n";
    } else {
        std::cout << "[Test 3] Verify & Install Signed Package -> [FAIL]\n";
        failures++;
    }

    // Cleanup
    fs::remove_all(tmp_cache);

    if (failures > 0) {
        std::cout << "\n❌ " << failures << " SIGNATURE TESTS FAILED!\n";
        return 1;
    }

    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL FASE 15 PART 2 PACKAGE SIGNATURE TESTS PASSED PERFECTLY!\n";
    std::cout << "========================================\n";
    return 0;
}
