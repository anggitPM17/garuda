// =====================================================================
// FASE D.4.2 - RICHAT NATIVE CRYPTO TESTS
// Memverifikasi native_crypto.h & native_crypto.cpp
// =====================================================================
#include "../android/app/src/main/cpp/native_crypto.h"
#include <iostream>
#include <string>
#include <cstdlib>

using namespace richat::native;

#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

int main() {
    std::cout << "========================================\n";
    std::cout << "FASE D.4.2 - NATIVE CRYPTO TESTS\n";
    std::cout << "========================================\n\n";

    // Test 1: AES-GCM Encrypt & Decrypt
    std::cout << "[Test 1] AES-GCM Encrypt & Decrypt...\n";
    {
        std::vector<uint8_t> key = {0x01, 0x02, 0x03, 0x04};
        std::vector<uint8_t> nonce = {0x10, 0x20};
        std::vector<uint8_t> plaintext = {'H', 'e', 'l', 'l', 'o', ' ', 'N', 'a', 't', 'i', 'v', 'e'};

        auto cipher = NativeCrypto::aes_gcm_encrypt(key, nonce, plaintext);
        CHECK(cipher != plaintext, "Ciphertext harus berbeda dari plaintext");

        auto decrypted = NativeCrypto::aes_gcm_decrypt(key, nonce, cipher);
        CHECK(decrypted == plaintext, "Decrypted text harus sama persis dengan plaintext");
    }
    std::cout << "PASS\n\n";

    // Test 2: X25519 Keypair Generation
    std::cout << "[Test 2] X25519 Keypair Generation...\n";
    {
        auto [priv, pub] = NativeCrypto::x25519_keypair();
        CHECK(priv.size() == 32, "Private key size harus 32 bytes");
        CHECK(pub.size() == 32, "Public key size harus 32 bytes");
    }
    std::cout << "PASS\n\n";

    // Test 3: X25519 Shared Secret Match
    std::cout << "[Test 3] X25519 Shared Secret Match...\n";
    {
        auto [priv_a, pub_a] = NativeCrypto::x25519_keypair();
        auto [priv_b, pub_b] = NativeCrypto::x25519_keypair();

        auto secret_a = NativeCrypto::x25519_shared_secret(priv_a, pub_b);
        auto secret_b = NativeCrypto::x25519_shared_secret(priv_b, pub_a);

        CHECK(secret_a == secret_b, "Shared secret A dan B harus identik");
        CHECK(secret_a.size() == 32, "Shared secret size harus 32 bytes");
    }
    std::cout << "PASS\n\n";

    // Test 4: SHA-256 Hash
    std::cout << "[Test 4] SHA-256 Hash...\n";
    {
        std::string hash = NativeCrypto::sha256_hex("garuda");
        CHECK(!hash.empty(), "SHA-256 hex string tidak boleh kosong");
        CHECK(hash.length() == 64, "SHA-256 hex length harus 64 karakter");
    }
    std::cout << "PASS\n\n";

    // Test 5: Empty Key AES Rejection
    std::cout << "[Test 5] Empty Key AES Rejection...\n";
    {
        auto cipher = NativeCrypto::aes_gcm_encrypt({}, {0x01}, {'A'});
        CHECK(cipher.empty(), "Encrypt dengan key kosong harus return empty vector");
    }
    std::cout << "PASS\n\n";

    std::cout << "========================================\n";
    std::cout << "FASE D.4.2 - NATIVE CRYPTO SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
