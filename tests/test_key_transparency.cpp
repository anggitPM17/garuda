// =====================================================================
// FASE G4.2 - GARUDA KEY TRANSPARENCY TESTS
// Memverifikasi key_transparency.hpp
// =====================================================================
#include "../include/garuda/beam/crypto/key_transparency.hpp"
#include <iostream>
#include <string>
#include <cstdlib>

using namespace garuda::beam;

#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

int main() {
    std::cout << "========================================\n";
    std::cout << "FASE G4.2 - GARUDA KEY TRANSPARENCY TESTS\n";
    std::cout << "========================================\n\n";

    KeyTransparency kt;

    // Test 1: Register & Get Key
    std::cout << "[Test 1] Register & Get Key...\n";
    {
        std::string root = kt.register_key("user_101", "pubkey_x25519_101");
        CHECK(!root.empty(), "Merkle root harus ter-generate");

        auto key = kt.get_key("user_101");
        CHECK(key.has_value() && *key == "pubkey_x25519_101", "Public key user_101 harus pubkey_x25519_101");
        CHECK(kt.key_count() == 1, "Key count harus 1");
    }
    std::cout << "PASS\n\n";

    // Test 2: Merkle Root Changes on New Register
    std::cout << "[Test 2] Merkle Root Changes...\n";
    {
        std::string r1 = kt.root();
        std::string r2 = kt.register_key("user_102", "pubkey_x25519_102");
        CHECK(r1 != r2, "Merkle root harus berubah saat user baru terdaftar");
    }
    std::cout << "PASS\n\n";

    // Test 3: Merkle Proof Verification Valid
    std::cout << "[Test 3] Merkle Proof Verification Valid...\n";
    {
        kt.register_key("user_103", "pubkey_x25519_103");
        kt.register_key("user_104", "pubkey_x25519_104");

        auto proof_opt = kt.get_proof("user_103");
        CHECK(proof_opt.has_value(), "Merkle proof untuk user_103 harus ada");

        bool valid = KeyTransparency::verify_proof("user_103", "pubkey_x25519_103", *proof_opt);
        CHECK(valid, "Merkle proof verification harus TRUE");
    }
    std::cout << "PASS\n\n";

    // Test 4: Merkle Proof Invalid on Key Tamper
    std::cout << "[Test 4] Merkle Proof Invalid on Key Tamper...\n";
    {
        auto proof_opt = kt.get_proof("user_103");
        CHECK(proof_opt.has_value(), "Merkle proof ada");

        bool invalid = KeyTransparency::verify_proof("user_103", "tampered_fake_pubkey", *proof_opt);
        CHECK(!invalid, "Merkle proof verification dengan pubkey yang diubah harus FALSE");
    }
    std::cout << "PASS\n\n";

    // Test 5: Multiple Keys Count
    std::cout << "[Test 5] Multiple Keys Count...\n";
    {
        CHECK(kt.key_count() == 4, "Key count total harus 4 (user_101, 102, 103, 104)");
        CHECK(kt.get_key("user_unknown") == std::nullopt, "Unknown user key harus std::nullopt");
    }
    std::cout << "PASS\n\n";

    std::cout << "========================================\n";
    std::cout << "FASE G4.2 - KEY TRANSPARENCY SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
