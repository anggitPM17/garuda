// =====================================================================
// FASE C.5.2 - GARUDA TOTP 2FA TESTS
// Memverifikasi totp_2fa.hpp
// =====================================================================
#include "../include/garuda/security/totp_2fa.hpp"
#include <iostream>
#include <string>
#include <cstdlib>

using namespace garuda::security;

#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

int main() {
    std::cout << "========================================\n";
    std::cout << "FASE C.5.2 - TOTP 2FA TESTS\n";
    std::cout << "========================================\n\n";

    // Test 1: Generate Secret
    std::cout << "[Test 1] Generate Secret...\n";
    {
        std::string secret = Totp2fa::generate_secret(20);
        CHECK(!secret.empty(), "Secret tidak boleh kosong");
        CHECK(secret.length() == 20, "Secret length harus 20 Base32 chars");
    }
    std::cout << "PASS\n\n";

    // Test 2: Generate otpauth URI
    std::cout << "[Test 2] Generate otpauth URI...\n";
    {
        std::string uri = Totp2fa::generate_otpauth_uri("JBSWY3DPEHPK3PXP", "user@richat.my.id", "RICHAT");
        CHECK(uri.find("otpauth://totp/RICHAT:user@richat.my.id") == 0, "URI prefix harus otpauth://totp/RICHAT:user@richat.my.id");
        CHECK(uri.find("secret=JBSWY3DPEHPK3PXP") != std::string::npos, "URI harus mengandung secret");
    }
    std::cout << "PASS\n\n";

    // Test 3: Verify TOTP Valid Code
    std::cout << "[Test 3] Verify TOTP Valid Code...\n";
    {
        std::string secret = "JBSWY3DPEHPK3PXP";
        auto now = std::chrono::system_clock::now();
        std::uint64_t current_time = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
        std::uint64_t step = current_time / 30;

        std::string valid_code = Totp2fa::calculate_totp(secret, step);
        bool verified = Totp2fa::verify_totp(secret, valid_code, now);

        CHECK(verified, "verify_totp harus return true untuk kode TOTP yang sah");
    }
    std::cout << "PASS\n\n";

    // Test 4: Verify Invalid TOTP Code
    std::cout << "[Test 4] Verify Invalid TOTP Code...\n";
    {
        std::string secret = "JBSWY3DPEHPK3PXP";
        bool verified = Totp2fa::verify_totp(secret, "000000");
        CHECK(!verified, "verify_totp harus return false untuk kode TOTP yang salah");
    }
    std::cout << "PASS\n\n";

    // Test 5: Generate Backup Codes
    std::cout << "[Test 5] Generate Backup Codes...\n";
    {
        auto codes = Totp2fa::generate_backup_codes(10);
        CHECK(codes.size() == 10, "Jumlah backup codes harus 10");
        CHECK(codes[0].length() == 8, "Panjang backup code harus 8 karakter");
    }
    std::cout << "PASS\n\n";

    std::cout << "========================================\n";
    std::cout << "FASE C.5.2 - TOTP 2FA SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
