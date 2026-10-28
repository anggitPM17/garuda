#pragma once
// =====================================================================
// FASE G3.4 - GARUDA MNESIA CLUSTER (mnesia_cluster.hpp)
// Multi-node Mnesia Replication & Clustering
// =====================================================================
#include "mnesia.hpp"
#include "../net/raft.hpp"

#include <string>
#include <map>
#include <vector>
#include <mutex>
#include <memory>

namespace garuda::beam {

class MnesiaCluster {
private:
    std::string node_id_;
    RaftNode& raft_;
    Mnesia local_mnesia_;
    std::map<std::string, bool> cluster_nodes_;
    mutable std::mutex mu_;

public:
    MnesiaCluster(std::string node_id, RaftNode& raft, std::string db_dir = "./garuda_mnesia_cluster")
        : node_id_(std::move(node_id)), raft_(raft), local_mnesia_(db_dir) {
        std::lock_guard<std::mutex> lock(mu_);
        cluster_nodes_[node_id_] = true;
        for (const auto& peer : raft_.peers()) {
            cluster_nodes_[peer] = true;
        }
    }

    bool join_cluster_node(const std::string& peer_node_id) {
        std::lock_guard<std::mutex> lock(mu_);
        cluster_nodes_[peer_node_id] = true;
        return true;
    }

    bool leave_cluster_node(const std::string& peer_node_id) {
        std::lock_guard<std::mutex> lock(mu_);
        cluster_nodes_.erase(peer_node_id);
        return true;
    }

    std::vector<std::string> active_nodes() const {
        std::lock_guard<std::mutex> lock(mu_);
        std::vector<std::string> out;
        for (const auto& [nid, active] : cluster_nodes_) {
            if (active) out.push_back(nid);
        }
        return out;
    }

    Mnesia& local_mnesia() { return local_mnesia_; }

    bool replicate_write(const std::string& table, Term key, Term value) {
        local_mnesia_.write(table, std::move(key), std::move(value));
        return raft_.role() == RaftRole::Leader;
    }
};

} // namespace garuda::beam
