// =====================================================================
// FASE G4.4 - GARUDA E2E SESSION TESTS
// Memverifikasi e2e_session.hpp
// =====================================================================
#include "../include/garuda/beam/crypto/e2e_session.hpp"
#include <iostream>
#include <string>
#include <cstdlib>

using namespace garuda::beam;

#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

int main() {
    std::cout << "========================================\n";
    std::cout << "FASE G4.4 - GARUDA E2E SESSION TESTS\n";
    std::cout << "========================================\n\n";

    // Test 1: Session Init
    std::cout << "[Test 1] Session Init...\n";
    {
        E2ESession session("alice", "bob", "shared_x25519_key_32bytes");
        CHECK(session.is_active(), "E2E Session baru harus aktif");
        CHECK(session.send_sequence() == 0, "Initial send sequence = 0");
    }
    std::cout << "PASS\n\n";

    // Test 2: Encrypt & Decrypt Single Message
    std::cout << "[Test 2] Encrypt & Decrypt Single Message...\n";
    {
        E2ESession session("alice", "bob", "shared_x25519_key_32bytes");
        auto msg_opt = session.encrypt("alice", "bob", "Pesan Rahasia E2E #1");
        CHECK(msg_opt.has_value(), "Encrypt message harus mengembalikan E2EMessage");
        CHECK(msg_opt->sequence_number == 1, "Sequence number = 1");

        auto decrypted = session.decrypt(*msg_opt);
        CHECK(decrypted.has_value(), "Decrypt message harus berhasil");
        CHECK(*decrypted == "Pesan Rahasia E2E #1", "Decrypted plaintext harus cocok");
    }
    std::cout << "PASS\n\n";

    // Test 3: Multiple Messages Sequence
    std::cout << "[Test 3] Multiple Messages Sequence...\n";
    {
        E2ESession session("alice", "bob", "shared_x25519_key_32bytes");
        auto m1 = session.encrypt("alice", "bob", "Pesan 1");
        auto m2 = session.encrypt("alice", "bob", "Pesan 2");
        auto m3 = session.encrypt("alice", "bob", "Pesan 3");

        CHECK(m3->sequence_number == 3, "Sequence number harus 3");

        CHECK(session.decrypt(*m1) == "Pesan 1", "Msg 1 decrypt ok");
        CHECK(session.decrypt(*m2) == "Pesan 2", "Msg 2 decrypt ok");
        CHECK(session.decrypt(*m3) == "Pesan 3", "Msg 3 decrypt ok");
    }
    std::cout << "PASS\n\n";

    // Test 4: Closed Session Rejection
    std::cout << "[Test 4] Closed Session Rejection...\n";
    {
        E2ESession session("alice", "bob", "shared_x25519_key_32bytes");
        session.close();
        CHECK(!session.is_active(), "Session status harus false setelah close");

        auto msg_opt = session.encrypt("alice", "bob", "Halo?");
        CHECK(!msg_opt.has_value(), "Encrypt pada closed session harus std::nullopt");
    }
    std::cout << "PASS\n\n";

    // Test 5: Empty Message Rejection
    std::cout << "[Test 5] Empty Message Rejection...\n";
    {
        E2ESession session("alice", "bob", "shared_x25519_key_32bytes");
        auto empty_opt = session.encrypt("alice", "bob", "");
        CHECK(!empty_opt.has_value(), "Encrypt empty message harus std::nullopt");
    }
    std::cout << "PASS\n\n";

    std::cout << "========================================\n";
    std::cout << "FASE G4.4 - E2E SESSION SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
