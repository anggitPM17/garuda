// =====================================================================
// FASE G4.3 - GARUDA SEALED SENDER TESTS
// Memverifikasi sealed_sender.hpp
// =====================================================================
#include "../include/garuda/beam/crypto/sealed_sender.hpp"
#include <iostream>
#include <string>
#include <cstdlib>

using namespace garuda::beam;

#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

int main() {
    std::cout << "========================================\n";
    std::cout << "FASE G4.3 - GARUDA SEALED SENDER TESTS\n";
    std::cout << "========================================\n\n";

    SealedSender ss("voprf_secret_key_garuda_2026");

    // Test 1: Issue Ticket
    std::cout << "[Test 1] Issue Ticket...\n";
    {
        std::string ticket = ss.issue_ticket("sender_101");
        CHECK(!ticket.empty(), "Ticket harus non-kosong");
        CHECK(ticket.length() == 64, "Ticket hash SHA-256 length harus 64");
    }
    std::cout << "PASS\n\n";

    // Test 2: Seal & Unseal
    std::cout << "[Test 2] Seal & Unseal...\n";
    {
        std::string ticket = ss.issue_ticket("sender_101");
        auto env_opt = ss.seal(ticket, "recipient_202", "Pesan rahasia terenkripsi!");
        CHECK(env_opt.has_value(), "Seal message harus mengembalikan envelope");

        std::string env_id = env_opt->envelope_id;
        auto unsealed = ss.unseal("recipient_202", env_id);
        CHECK(unsealed.has_value(), "Unseal harus berhasil untuk recipient yang benar");
        CHECK(*unsealed == "Pesan rahasia terenkripsi!", "Plaintext unsealed harus sama dengan pesan asli");
    }
    std::cout << "PASS\n\n";

    // Test 3: Expired Envelope
    std::cout << "[Test 3] Expired Envelope...\n";
    {
        std::string ticket = ss.issue_ticket("sender_101");
        auto env_opt = ss.seal(ticket, "recipient_202", "Pesan kadaluarsa", 0); // ttl_seconds = 0
        CHECK(env_opt.has_value(), "Seal envelope dibuat");

        auto unsealed = ss.unseal("recipient_202", env_opt->envelope_id);
        CHECK(!unsealed.has_value(), "Unseal expired envelope harus return std::nullopt");
        CHECK(ss.expired_count() >= 1, "Expired count harus >= 1");
    }
    std::cout << "PASS\n\n";

    // Test 4: Recipient Isolation
    std::cout << "[Test 4] Recipient Isolation...\n";
    {
        std::string ticket = ss.issue_ticket("sender_101");
        auto env_opt = ss.seal(ticket, "recipient_A", "Hanya untuk User A", 300);
        CHECK(env_opt.has_value(), "Envelope created");

        auto wrong_unseal = ss.unseal("recipient_B", env_opt->envelope_id);
        CHECK(!wrong_unseal.has_value(), "User B tidak boleh bisa unseal envelope User A");

        auto right_unseal = ss.unseal("recipient_A", env_opt->envelope_id);
        CHECK(right_unseal.has_value() && *right_unseal == "Hanya untuk User A", "User A harus berhasil unseal");
    }
    std::cout << "PASS\n\n";

    // Test 5: Multiple Envelopes
    std::cout << "[Test 5] Multiple Envelopes...\n";
    {
        std::string ticket = ss.issue_ticket("sender_101");
        auto e1 = ss.seal(ticket, "r1", "Msg 1", 300);
        auto e2 = ss.seal(ticket, "r2", "Msg 2", 300);

        CHECK(ss.unseal("r1", e1->envelope_id) == "Msg 1", "Msg 1 ok");
        CHECK(ss.unseal("r2", e2->envelope_id) == "Msg 2", "Msg 2 ok");
        CHECK(ss.pending_count() >= 3, "Pending envelopes count >= 3");
    }
    std::cout << "PASS\n\n";

    std::cout << "========================================\n";
    std::cout << "FASE G4.3 - SEALED SENDER SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
