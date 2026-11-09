// =====================================================================
// GARUDA COMPILER — FASE 15 PART 2: PACKAGE REGISTRY TEST SUITE
// =====================================================================

#include "../include/garuda/compiler/package_registry.hpp"
#include <iostream>
#include <cassert>
#include <filesystem>

using namespace garuda::compiler;
namespace fs = std::filesystem;

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA COMPILER — FASE 15 PART 2: PACKAGE REGISTRY SUITE\n";
    std::cout << "=======================================================\n\n";

    fs::path tmp_cache = fs::temp_directory_path() / "garuda_registry_e2e_test";
    fs::remove_all(tmp_cache);
    fs::create_directories(tmp_cache);

    PackageRegistry reg(tmp_cache.string());
    int failures = 0;

    // Test 1: Publish Package to Registry
    PackageMeta meta1{
        "garuda_super_math",
        "1.2.0",
        "Super Fast Math Library for Garuda",
        "Anggit Prasetyo",
        "sha256_mock_checksum_123",
        "pubkey_anggit_999",
        "ed25519_sig_mock",
        {{"garuda_core", "^1.0.0"}}
    };
    reg.publish_package(meta1);

    auto retrieved = reg.get_package("garuda_super_math", "1.2.0");
    if (retrieved.has_value() && retrieved->author == "Anggit Prasetyo") {
        std::cout << "[Test 1] Publish Package & Retrieve Metadata -> [PASS]\n";
    } else {
        std::cout << "[Test 1] Publish Package -> [FAIL]\n";
        failures++;
    }

    // Test 2: Search Registry Catalog
    auto search_res = reg.search("super_math");
    if (!search_res.empty() && search_res[0].name == "garuda_super_math") {
        std::cout << "[Test 2] Search Package Catalog -> Matched 'garuda_super_math' [PASS]\n";
    } else {
        std::cout << "[Test 2] Search Package Catalog -> [FAIL]\n";
        failures++;
    }

    // Test 3: Semver Resolution
    reg.publish_package({"garuda_super_math", "1.5.0", "Newer", "Anggit", "hash", "", "", {}});
    reg.publish_package({"garuda_super_math", "2.0.0", "Breaking", "Anggit", "hash", "", "", {}});

    auto best = reg.resolve_best_version("garuda_super_math", "^1.0.0");
    if (best.has_value() && *best == "1.5.0") {
        std::cout << "[Test 3] Semver Caret ^1.0.0 Resolution -> Resolved to '1.5.0' [PASS]\n";
    } else {
        std::cout << "[Test 3] Semver Caret Resolution -> [FAIL]\n";
        failures++;
    }

    // Cleanup
    fs::remove_all(tmp_cache);

    if (failures > 0) {
        std::cout << "\n❌ " << failures << " REGISTRY TESTS FAILED!\n";
        return 1;
    }

    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL FASE 15 PART 2 PACKAGE REGISTRY TESTS PASSED PERFECTLY!\n";
    std::cout << "========================================\n";
    return 0;
}
