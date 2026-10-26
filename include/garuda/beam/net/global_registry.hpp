#pragma once
// Garuda Distributed Engine: Global Process Registry (`global_registry.hpp`)
// Mengimplementasikan pendaftaran & pencarian PID terdistribusi di seluruh kluster via Raft Consensus.
#include "raft.hpp"
#include "../stdlib/crypto.hpp"

#include <iostream>
#include <map>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>

namespace garuda::beam {

class ConsistentHashRing {
public:
    explicit ConsistentHashRing(std::size_t virtual_nodes_per_peer = 3)
        : vnodes_(virtual_nodes_per_peer) {}

    void add_node(const std::string& node_id) {
        std::lock_guard<std::mutex> lock(mu_);
        for (std::size_t i = 0; i < vnodes_; ++i) {
            std::size_t h = hash_key(node_id + "#vnode_" + std::to_string(i));
            ring_[h] = node_id;
        }
    }

    void remove_node(const std::string& node_id) {
        std::lock_guard<std::mutex> lock(mu_);
        for (std::size_t i = 0; i < vnodes_; ++i) {
            std::size_t h = hash_key(node_id + "#vnode_" + std::to_string(i));
            ring_.erase(h);
        }
    }

    std::string get_node(const std::string& key) const {
        std::lock_guard<std::mutex> lock(mu_);
        if (ring_.empty()) return "";

        std::size_t h = hash_key(key);
        auto it = ring_.lower_bound(h);
        if (it == ring_.end()) {
            return ring_.begin()->second;
        }
        return it->second;
    }

    std::vector<std::string> get_replica_nodes(const std::string& key, std::size_t replica_count = 3) const {
        std::lock_guard<std::mutex> lock(mu_);
        std::vector<std::string> nodes;
        if (ring_.empty()) return nodes;

        std::size_t h = hash_key(key);
        auto it = ring_.lower_bound(h);

        std::set<std::string> seen;
        while (seen.size() < replica_count && seen.size() < ring_.size()) {
            if (it == ring_.end()) it = ring_.begin();
            if (seen.insert(it->second).second) {
                nodes.push_back(it->second);
            }
            ++it;
        }
        return nodes;
    }

private:
    static std::size_t hash_key(const std::string& k) {
        std::string h_str = stdlib::Sha256::hash(k);
        std::size_t val = 0;
        for (std::size_t i = 0; i < 16 && i < h_str.length(); ++i) {
            val = (val << 4) ^ static_cast<std::size_t>(h_str[i]);
        }
        return val;
    }

    std::size_t vnodes_ = 3;
    mutable std::mutex mu_;
    std::map<std::size_t, std::string> ring_;
};

class GlobalRegistry {
public:
    explicit GlobalRegistry(RaftNode& raft) : raft_(raft) {
        raft_.set_apply_callback([this](const std::string& command) {
            apply_command(command);
        });
    }

    bool daftar(const std::string& nama, Pid pid) {
        if (raft_.role() != RaftRole::Leader) {
            std::cout << "⚠️ [GARUDA REGISTRY] Cannot register '" << nama << "' on Follower node. Must register on Leader!" << std::endl;
            return false;
        }

        std::string cmd = "REGISTER " + nama + " " + std::to_string(pid.id);
        bool ok = raft_.append_command(cmd);
        if (ok) {
            apply_command(cmd);
            std::cout << "🌐 [GARUDA REGISTRY] Registered global process '" << nama << "' -> PID <0." << pid.id << ".0>" << std::endl;
        }
        return ok;
    }

    std::optional<Pid> cari(const std::string& nama) const {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = registry_.find(nama);
        if (it != registry_.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    bool hapus(const std::string& nama) {
        if (raft_.role() != RaftRole::Leader) {
            return false;
        }

        std::string cmd = "UNREGISTER " + nama;
        bool ok = raft_.append_command(cmd);
        if (ok) {
            apply_command(cmd);
            std::cout << "🗑️ [GARUDA REGISTRY] Unregistered global process '" << nama << "'" << std::endl;
        }
        return ok;
    }

    std::map<std::string, Pid> daftar_semua() const {
        std::lock_guard<std::mutex> lock(mu_);
        return registry_;
    }

    void apply_command(const std::string& command) {
        std::lock_guard<std::mutex> lock(mu_);
        std::istringstream ss(command);
        std::string action, nama;
        ss >> action >> nama;

        if (action == "REGISTER") {
            std::uint64_t pid_id;
            if (ss >> pid_id) {
                registry_[nama] = Pid{pid_id};
            }
        } else if (action == "UNREGISTER") {
            registry_.erase(nama);
        }
    }

private:
    RaftNode& raft_;
    mutable std::mutex mu_;
    std::map<std::string, Pid> registry_;
};

} // namespace garuda::beam
