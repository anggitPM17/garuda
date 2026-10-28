// =====================================================================
// FASE G4.1 - GARUDA CRYPTO TESTS
// Memverifikasi crypto.hpp
// =====================================================================
#include "../include/garuda/beam/crypto/crypto.hpp"
#include "../include/garuda/beam/stdlib/crypto.hpp"
#include <iostream>
#include <string>
#include <cstdlib>

using namespace garuda::beam;

#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

int main() {
    std::cout << "========================================\n";
    std::cout << "FASE G4.1 - GARUDA CRYPTO TESTS\n";
    std::cout << "========================================\n\n";

    // Test 1: Base64 & Base64URL Encoding / Decoding
    std::cout << "[Test 1] Base64 & Base64URL Encoding / Decoding...\n";
    {
        std::string text = "Garuda BEAM Crypto Engine 2026";
        std::string b64 = Crypto::base64_encode(text);
        std::string decoded = Crypto::base64_decode(b64);
        CHECK(decoded == text, "Base64 decode harus mengembalikan teks asli");

        std::string b64url = Crypto::base64url_encode(text);
        CHECK(b64url.find('+') == std::string::npos, "Base64URL tidak boleh mengandung +");
        CHECK(b64url.find('/') == std::string::npos, "Base64URL tidak boleh mengandung /");
        CHECK(b64url.find('=') == std::string::npos, "Base64URL tidak boleh mengandung =");
    }
    std::cout << "PASS\n\n";

    // Test 2: FNV-1a Hash
    std::cout << "[Test 2] FNV-1a Hash...\n";
    {
        std::string h1 = Crypto::fnv1a_hash("garuda_secret");
        std::string h2 = Crypto::fnv1a_hash("garuda_secret");
        std::string h3 = Crypto::fnv1a_hash("garuda_different");

        CHECK(h1 == h2, "FNV-1a hash harus deterministik");
        CHECK(h1 != h3, "FNV-1a hash beda input harus beda output");
        CHECK(h1.length() == 16, "FNV-1a hex length harus 16 hex chars");
    }
    std::cout << "PASS\n\n";

    // Test 3: SHA-256 Hash & Salt
    std::cout << "[Test 3] SHA-256 Hash & Salt...\n";
    {
        std::string sha = stdlib::Sha256::hash("hello_garuda");
        CHECK(sha.length() == 64, "SHA-256 hex string harus 64 karakter");

        std::string sha2 = stdlib::Sha256::hash("hello_garuda");
        CHECK(sha == sha2, "SHA-256 harus deterministik");

        std::string salt1 = stdlib::Sha256::generate_salt(16);
        std::string salt2 = stdlib::Sha256::generate_salt(16);
        CHECK(salt1.length() == 16, "Salt length harus 16");
        CHECK(salt1 != salt2, "Random salt harus unik");
    }
    std::cout << "PASS\n\n";

    // Test 4: UUID & OTP Generation
    std::cout << "[Test 4] UUID & OTP Generation...\n";
    {
        std::string uuid1 = Crypto::generate_uuid("usr_");
        std::string uuid2 = Crypto::generate_uuid("usr_");
        CHECK(uuid1.rfind("usr_", 0) == 0, "UUID harus diawali prefix usr_");
        CHECK(uuid1 != uuid2, "UUID harus unik");

        std::string otp = Crypto::generate_otp();
        CHECK(otp.length() == 6, "OTP length harus 6 digit");
        CHECK(otp.find_first_not_of("0123456789") == std::string::npos, "OTP hanya berisi angka");
    }
    std::cout << "PASS\n\n";

    // Test 5: JWT Token Generation & Verification
    std::cout << "[Test 5] JWT Token Generation & Verification...\n";
    {
        std::string user_id = "usr_883921";
        std::string secret = "garuda_jwt_super_secret_key";

        std::string jwt = Crypto::generate_jwt(user_id, secret);
        CHECK(!jwt.empty(), "JWT token harus tergenerasi");

        std::string extracted_user;
        bool valid = Crypto::verify_jwt(jwt, secret, extracted_user);
        CHECK(valid, "JWT verification dengan secret benar harus valid");
        CHECK(extracted_user == user_id, "User ID dari JWT harus usr_883921");

        std::string wrong_user;
        bool invalid_secret = Crypto::verify_jwt(jwt, "wrong_secret", wrong_user);
        CHECK(!invalid_secret, "JWT verification dengan secret salah harus fail");
    }
    std::cout << "PASS\n\n";

    std::cout << "========================================\n";
    std::cout << "FASE G4.1 - GARUDA CRYPTO SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
