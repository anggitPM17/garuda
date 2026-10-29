// =====================================================================
// FASE C.5.3 - GARUDA CAPTCHA POW TESTS
// Memverifikasi captcha_pow.hpp
// =====================================================================
#include "../include/garuda/security/captcha_pow.hpp"
#include <iostream>
#include <string>
#include <cstdlib>

using namespace garuda::security;

#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

int main() {
    std::cout << "========================================\n";
    std::cout << "FASE C.5.3 - CAPTCHA POW TESTS\n";
    std::cout << "========================================\n\n";

    CaptchaPow captcha;

    // Test 1: Generate Challenge
    std::cout << "[Test 1] Generate Challenge...\n";
    {
        auto ch = captcha.generate_challenge(3);
        CHECK(!ch.id.empty(), "Challenge ID tidak boleh kosong");
        CHECK(!ch.prefix.empty(), "Prefix tidak boleh kosong");
        CHECK(ch.difficulty == 3, "Difficulty harus 3");
        CHECK(captcha.active_count() == 1, "active_count harus 1");
    }
    std::cout << "PASS\n\n";

    // Test 2: Solve & Verify PoW
    std::cout << "[Test 2] Solve & Verify PoW...\n";
    {
        auto ch = captcha.generate_challenge(2); // Lower difficulty for fast test
        std::string target_prefix(2, '0');
        std::string nonce;
        std::uint64_t counter = 0;

        // Brute force PoW nonce
        while (true) {
            std::string candidate = std::to_string(counter++);
            std::string hash = garuda::beam::stdlib::Sha256::hash(ch.prefix + candidate);
            if (hash.compare(0, 2, target_prefix) == 0) {
                nonce = candidate;
                break;
            }
        }

        bool verified = captcha.verify(ch.id, nonce);
        CHECK(verified, "verify() harus return true untuk PoW nonce yang valid");
    }
    std::cout << "PASS\n\n";

    // Test 3: Verify Wrong Nonce
    std::cout << "[Test 3] Verify Wrong Nonce...\n";
    {
        auto ch = captcha.generate_challenge(3);
        bool verified = captcha.verify(ch.id, "invalid_nonce_12345");
        CHECK(!verified, "verify() dengan nonce salah harus return false");
    }
    std::cout << "PASS\n\n";

    // Test 4: One-Time Use Verification
    std::cout << "[Test 4] One-Time Use Verification...\n";
    {
        auto ch = captcha.generate_challenge(2);
        std::string nonce;
        std::uint64_t counter = 0;
        while (true) {
            std::string candidate = std::to_string(counter++);
            std::string hash = garuda::beam::stdlib::Sha256::hash(ch.prefix + candidate);
            if (hash.compare(0, 2, "00") == 0) {
                nonce = candidate;
                break;
            }
        }

        CHECK(captcha.verify(ch.id, nonce), "First verify harus true");
        CHECK(!captcha.verify(ch.id, nonce), "Second verify dengan challenge_id sama harus false (one-time use)");
    }
    std::cout << "PASS\n\n";

    // Test 5: Non-Existent Challenge ID Rejection
    std::cout << "[Test 5] Non-Existent Challenge ID Rejection...\n";
    {
        bool verified = captcha.verify("ch_non_existent", "000");
        CHECK(!verified, "verify() dengan ID tidak dikenal harus return false");
    }
    std::cout << "PASS\n\n";

    std::cout << "========================================\n";
    std::cout << "FASE C.5.3 - CAPTCHA POW SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
