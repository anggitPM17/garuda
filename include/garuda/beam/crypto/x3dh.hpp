#pragma once
// Garuda GVM: X3DH Prekey Bundle & E2EE Crypto BIFs untuk RICHAT
#include "crypto.hpp"
#include "../core/term.hpp"

#include <iostream>
#include <map>
#include <mutex>
#include <optional>
#include <string>

namespace garuda::beam {

struct PrekeyBundle {
    std::string user_id;
    std::string identity_key_b64;
    std::string signed_prekey_b64;
    std::string one_time_prekey_b64;
    std::string signature_b64;
};

class X3DhEngine {
public:
    void upload_prekey_bundle(PrekeyBundle bundle) {
        std::lock_guard<std::mutex> lock(mu_);
        std::string uid = bundle.user_id;
        bundles_[uid] = std::move(bundle);
        std::cout << "🔐 [GARUDA X3DH] Prekey bundle uploaded for user: " << uid << std::endl;
    }

    std::optional<PrekeyBundle> fetch_prekey_bundle(const std::string& user_id) {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = bundles_.find(user_id);
        if (it != bundles_.end()) return it->second;
        return std::nullopt;
    }

private:
    std::mutex mu_;
    std::map<std::string, PrekeyBundle> bundles_;
};

} // namespace garuda::beam
