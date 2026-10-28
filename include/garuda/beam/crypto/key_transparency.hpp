#pragma once
// =====================================================================
// FASE G4.2 - Key Transparency: Merkle Tree untuk verifikasi kunci
// =====================================================================
#include "../stdlib/crypto.hpp"

#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <optional>
#include <memory>
#include <algorithm>

namespace garuda::beam {

struct MerkleProof {
    std::vector<std::string> siblings;  // hash path
    std::size_t index = 0;
    std::string root;
};

class KeyTransparency {
private:
    std::map<std::string, std::string> keys_;  // user_id -> public_key
    std::vector<std::string> user_order_;      // ordered user_ids
    std::string root_;
    mutable std::mutex mu_;

    void rebuild_tree() {
        if (user_order_.empty()) {
            root_ = "";
            return;
        }

        std::vector<std::string> current_level;
        for (const auto& uid : user_order_) {
            std::string leaf_data = uid + ":" + keys_[uid];
            current_level.push_back(stdlib::Sha256::hash(leaf_data));
        }

        while (current_level.size() > 1) {
            std::vector<std::string> next_level;
            for (std::size_t i = 0; i < current_level.size(); i += 2) {
                if (i + 1 < current_level.size()) {
                    next_level.push_back(stdlib::Sha256::hash(current_level[i] + current_level[i + 1]));
                } else {
                    next_level.push_back(current_level[i]);
                }
            }
            current_level = std::move(next_level);
        }

        root_ = current_level[0];
    }

public:
    std::string register_key(const std::string& user_id, const std::string& public_key) {
        std::lock_guard<std::mutex> lock(mu_);
        if (keys_.find(user_id) == keys_.end()) {
            user_order_.push_back(user_id);
        }
        keys_[user_id] = public_key;
        rebuild_tree();
        return root_;
    }

    std::optional<std::string> get_key(const std::string& user_id) const {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = keys_.find(user_id);
        if (it != keys_.end()) return it->second;
        return std::nullopt;
    }

    std::string root() const {
        std::lock_guard<std::mutex> lock(mu_);
        return root_;
    }

    std::optional<MerkleProof> get_proof(const std::string& user_id) const {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = keys_.find(user_id);
        if (it == keys_.end()) return std::nullopt;

        std::size_t idx = 0;
        for (std::size_t i = 0; i < user_order_.size(); ++i) {
            if (user_order_[i] == user_id) {
                idx = i;
                break;
            }
        }

        MerkleProof proof;
        proof.index = idx;
        proof.root = root_;

        std::vector<std::string> current_level;
        for (const auto& uid : user_order_) {
            std::string leaf_data = uid + ":" + keys_.at(uid);
            current_level.push_back(stdlib::Sha256::hash(leaf_data));
        }

        std::size_t curr_idx = idx;
        while (current_level.size() > 1) {
            std::size_t sibling_idx = (curr_idx % 2 == 0) ? curr_idx + 1 : curr_idx - 1;
            if (sibling_idx < current_level.size()) {
                proof.siblings.push_back(current_level[sibling_idx]);
            } else {
                proof.siblings.push_back(current_level[curr_idx]);
            }

            std::vector<std::string> next_level;
            for (std::size_t i = 0; i < current_level.size(); i += 2) {
                if (i + 1 < current_level.size()) {
                    next_level.push_back(stdlib::Sha256::hash(current_level[i] + current_level[i + 1]));
                } else {
                    next_level.push_back(current_level[i]);
                }
            }
            current_level = std::move(next_level);
            curr_idx /= 2;
        }

        return proof;
    }

    static bool verify_proof(const std::string& user_id,
                             const std::string& public_key,
                             const MerkleProof& proof) {
        std::string current_hash = stdlib::Sha256::hash(user_id + ":" + public_key);
        std::size_t idx = proof.index;

        for (const auto& sibling : proof.siblings) {
            if (idx % 2 == 0) {
                current_hash = stdlib::Sha256::hash(current_hash + sibling);
            } else {
                current_hash = stdlib::Sha256::hash(sibling + current_hash);
            }
            idx /= 2;
        }

        return current_hash == proof.root;
    }

    std::size_t key_count() const {
        std::lock_guard<std::mutex> lock(mu_);
        return keys_.size();
    }
};

} // namespace garuda::beam
