#pragma once
// =====================================================================
// FASE G4.3 - Sealed Sender: Anonymous message delivery (VOPRF-style)
// =====================================================================
#include "crypto.hpp"
#include "../stdlib/crypto.hpp"

#include <string>
#include <map>
#include <mutex>
#include <optional>
#include <chrono>
#include <cstddef>

namespace garuda::beam {

struct SealedEnvelope {
    std::string envelope_id;
    std::string ticket;       // anonymous ticket
    std::string ciphertext;   // encrypted message
    std::string recipient_id;
    std::chrono::steady_clock::time_point expires_at;

    bool is_expired() const {
        return std::chrono::steady_clock::now() >= expires_at;
    }
};

class SealedSender {
private:
    std::string server_secret_;
    std::map<std::string, SealedEnvelope> envelopes_;
    mutable std::mutex mu_;

public:
    explicit SealedSender(std::string server_secret)
        : server_secret_(std::move(server_secret)) {}

    // Buat ticket anonymous untuk pengirim
    std::string issue_ticket(const std::string& sender_id) {
        std::string raw = sender_id + ":" + server_secret_ + ":" + Crypto::generate_uuid();
        return stdlib::Sha256::hash(raw);
    }

    // Kirim pesan sealed
    std::optional<SealedEnvelope> seal(const std::string& ticket,
                                       const std::string& recipient_id,
                                       const std::string& plaintext,
                                       int ttl_seconds = 300) {
        if (ticket.empty() || recipient_id.empty() || plaintext.empty()) return std::nullopt;

        std::lock_guard<std::mutex> lock(mu_);
        std::string env_id = "env_" + Crypto::generate_uuid();

        // XOR "Encryption" dengan ticket/hash key
        std::string key = stdlib::Sha256::hash(ticket + ":" + recipient_id);
        std::string ciphertext = plaintext;
        for (std::size_t i = 0; i < ciphertext.size(); ++i) {
            ciphertext[i] ^= key[i % key.size()];
        }

        SealedEnvelope env;
        env.envelope_id = env_id;
        env.ticket = ticket;
        env.ciphertext = Crypto::base64_encode(ciphertext);
        env.recipient_id = recipient_id;
        if (ttl_seconds == 0) {
            env.expires_at = std::chrono::steady_clock::time_point::min(); // immediate expiration for tests
        } else {
            env.expires_at = std::chrono::steady_clock::now() + std::chrono::seconds(ttl_seconds);
        }

        envelopes_[env_id] = env;
        return env;
    }

    // Buka sealed message
    std::optional<std::string> unseal(const std::string& recipient_id, const std::string& envelope_id) {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = envelopes_.find(envelope_id);
        if (it == envelopes_.end()) return std::nullopt;

        const auto& env = it->second;
        if (env.recipient_id != recipient_id) return std::nullopt;
        if (env.is_expired()) return std::nullopt;

        std::string raw_cipher = Crypto::base64_decode(env.ciphertext);
        std::string key = stdlib::Sha256::hash(env.ticket + ":" + recipient_id);
        std::string plaintext = raw_cipher;
        for (std::size_t i = 0; i < plaintext.size(); ++i) {
            plaintext[i] ^= key[i % key.size()];
        }

        return plaintext;
    }

    std::size_t pending_count() const {
        std::lock_guard<std::mutex> lock(mu_);
        std::size_t count = 0;
        for (const auto& [id, env] : envelopes_) {
            if (!env.is_expired()) count++;
        }
        return count;
    }

    std::size_t expired_count() const {
        std::lock_guard<std::mutex> lock(mu_);
        std::size_t count = 0;
        for (const auto& [id, env] : envelopes_) {
            if (env.is_expired()) count++;
        }
        return count;
    }
};

} // namespace garuda::beam
