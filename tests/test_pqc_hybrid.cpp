#include <iostream>
#include <cassert>
#include <vector>

#include "garuda/beam/crypto/pqc_hybrid.hpp"

using namespace garuda::beam::crypto;

void test_mlkem_keypair_generation() {
    auto pair = PqcHybridEngine::generate_mlkem_keypair();

    assert(pair.public_key.size() == MLKEM_768_PUBLIC_KEY_BYTES && "Public key size must be 1184 bytes");
    assert(pair.secret_key.size() == MLKEM_768_SECRET_KEY_BYTES && "Secret key size must be 2400 bytes");

    std::cout << "[PASS] test_mlkem_keypair_generation" << std::endl;
}

void test_encapsulate_decapsulate_parity() {
    auto pair = PqcHybridEngine::generate_mlkem_keypair();

    std::vector<uint8_t> ciphertext;
    std::vector<uint8_t> sender_shared_secret;

    bool enc_ok = PqcHybridEngine::encapsulate(pair.public_key, ciphertext, sender_shared_secret);
    assert(enc_ok && "PqcHybridEngine::encapsulate failed");
    assert(ciphertext.size() == MLKEM_768_CIPHERTEXT_BYTES && "Ciphertext size must be 1088 bytes");
    assert(sender_shared_secret.size() == MLKEM_768_SHARED_SECRET_BYTES && "Shared secret size must be 32 bytes");
    (void)enc_ok;

    std::vector<uint8_t> receiver_shared_secret;
    bool dec_ok = PqcHybridEngine::decapsulate(pair.secret_key, ciphertext, receiver_shared_secret);
    assert(dec_ok && "PqcHybridEngine::decapsulate failed");
    (void)dec_ok;

    assert(sender_shared_secret.size() == receiver_shared_secret.size() && "Shared secret size mismatch");
    assert(sender_shared_secret == receiver_shared_secret && "Encapsulated and decapsulated shared secrets must match 100%");

    std::cout << "[PASS] test_encapsulate_decapsulate_parity" << std::endl;
}

void test_hybrid_secret_combination() {
    std::vector<uint8_t> ecdh_p256_secret(32, 0x11);
    std::vector<uint8_t> mlkem_768_secret(32, 0x22);

    auto master_hybrid_secret = PqcHybridEngine::combine_hybrid_secrets(ecdh_p256_secret, mlkem_768_secret);

    assert(master_hybrid_secret.size() == 32 && "Combined hybrid secret size must be 32 bytes");
    assert(master_hybrid_secret != ecdh_p256_secret && "Hybrid secret must be properly mixed");
    assert(master_hybrid_secret != mlkem_768_secret && "Hybrid secret must be properly mixed");

    std::cout << "[PASS] test_hybrid_secret_combination" << std::endl;
}

int main() {
    std::cout << "--- RUNNING POST-QUANTUM CRYPTOGRAPHY HYBRID TESTS ---" << std::endl;
    test_mlkem_keypair_generation();
    test_encapsulate_decapsulate_parity();
    test_hybrid_secret_combination();
    std::cout << "--- ALL PQC HYBRID TESTS PASSED 100% ---" << std::endl;
    return 0;
}
