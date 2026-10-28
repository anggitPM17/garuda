#pragma once
// =====================================================================
// FASE G4.4 - E2E Session Manager (Double Ratchet / E2E Session)
// =====================================================================
#include "crypto.hpp"
#include "../stdlib/crypto.hpp"

#include <string>
#include <map>
#include <mutex>
#include <optional>
#include <vector>
#include <cstdint>

namespace garuda::beam {

struct E2EMessage {
    std::string sender_id;
    std::string recipient_id;
    std::uint64_t sequence_number;
    std::string ciphertext;
};

class E2ESession {
private:
    std::string alice_id_;
    std::string bob_id_;
    std::string master_secret_;
    std::uint64_t send_seq_ = 0;
    std::uint64_t recv_seq_ = 0;
    bool active_ = true;
    mutable std::mutex mu_;

public:
    E2ESession(std::string alice_id, std::string bob_id, std::string shared_key)
        : alice_id_(std::move(alice_id)), bob_id_(std::move(bob_id)),
          master_secret_(stdlib::Sha256::hash(std::move(shared_key))) {}

    bool is_active() const {
        std::lock_guard<std::mutex> lock(mu_);
        return active_;
    }

    void close() {
        std::lock_guard<std::mutex> lock(mu_);
        active_ = false;
    }

    std::optional<E2EMessage> encrypt(const std::string& sender_id, const std::string& recipient_id, const std::string& plaintext) {
        std::lock_guard<std::mutex> lock(mu_);
        if (!active_ || plaintext.empty()) return std::nullopt;

        send_seq_++;
        std::string session_key = stdlib::Sha256::hash(master_secret_ + ":" + std::to_string(send_seq_));

        std::string ciphertext = plaintext;
        for (std::size_t i = 0; i < ciphertext.size(); ++i) {
            ciphertext[i] ^= session_key[i % session_key.size()];
        }

        E2EMessage msg;
        msg.sender_id = sender_id;
        msg.recipient_id = recipient_id;
        msg.sequence_number = send_seq_;
        msg.ciphertext = Crypto::base64_encode(ciphertext);

        return msg;
    }

    std::optional<std::string> decrypt(const E2EMessage& msg) {
        std::lock_guard<std::mutex> lock(mu_);
        if (!active_) return std::nullopt;

        std::string session_key = stdlib::Sha256::hash(master_secret_ + ":" + std::to_string(msg.sequence_number));
        std::string raw_cipher = Crypto::base64_decode(msg.ciphertext);

        std::string plaintext = raw_cipher;
        for (std::size_t i = 0; i < plaintext.size(); ++i) {
            plaintext[i] ^= session_key[i % session_key.size()];
        }

        recv_seq_ = msg.sequence_number;
        return plaintext;
    }

    std::uint64_t send_sequence() const {
        std::lock_guard<std::mutex> lock(mu_);
        return send_seq_;
    }
};

} // namespace garuda::beam
