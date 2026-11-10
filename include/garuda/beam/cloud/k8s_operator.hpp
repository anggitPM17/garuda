#pragma once
// Garuda Standard Library: Kubernetes Native Operator & Multi-Region CRDT Engine (`k8s_operator.hpp`)
// Mengimplementasikan CRD Spec Parser, HPA AutoScaler, & CRDT Multi-Datacenter State Sync.

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace garuda::beam::cloud {

struct GarudaClusterSpec {
    std::string cluster_name = "garuda-prod-cluster";
    int replicas = 3;
    int min_replicas = 2;
    int max_replicas = 10;
    double target_cpu_utilization = 70.0;
    std::size_t target_actor_queue_depth = 500;
};

struct NodeMetrics {
    std::string node_id;
    double cpu_usage_percent = 0.0;
    double memory_usage_mb = 0.0;
    std::size_t actor_queue_depth = 0;
};

class HpaAutoScaler {
public:
    explicit HpaAutoScaler(GarudaClusterSpec spec) : spec_(std::move(spec)) {}

    int evaluate_scale(const std::vector<NodeMetrics>& metrics) {
        if (metrics.empty()) return spec_.replicas;

        double total_cpu = 0.0;
        std::size_t total_queue = 0;

        for (const auto& m : metrics) {
            total_cpu += m.cpu_usage_percent;
            total_queue += m.actor_queue_depth;
        }

        double avg_cpu = total_cpu / metrics.size();
        std::size_t avg_queue = total_queue / metrics.size();

        int desired_replicas = spec_.replicas;

        if (avg_cpu > spec_.target_cpu_utilization || avg_queue > spec_.target_actor_queue_depth) {
            // Scale out
            desired_replicas = std::min(spec_.max_replicas, spec_.replicas + 2);
        } else if (avg_cpu < (spec_.target_cpu_utilization * 0.4) && avg_queue < (spec_.target_actor_queue_depth * 0.2)) {
            // Scale in
            desired_replicas = std::max(spec_.min_replicas, spec_.replicas - 1);
        }

        spec_.replicas = desired_replicas;
        return desired_replicas;
    }

    const GarudaClusterSpec& spec() const { return spec_; }

private:
    GarudaClusterSpec spec_;
};

// CRDT 1: G-Counter (Grow-Only Counter)
class GCounter {
public:
    explicit GCounter(std::string node_id) : node_id_(std::move(node_id)) {}

    void increment(std::int64_t amount = 1) {
        counts_[node_id_] += amount;
    }

    std::int64_t value() const {
        std::int64_t sum = 0;
        for (const auto& [id, c] : counts_) sum += c;
        return sum;
    }

    void merge(const GCounter& other) {
        for (const auto& [id, c] : other.counts_) {
            counts_[id] = std::max(counts_[id], c);
        }
    }

private:
    std::string node_id_;
    std::unordered_map<std::string, std::int64_t> counts_;
};

// CRDT 2: PN-Counter (Positive-Negative Counter)
class PNCounter {
public:
    explicit PNCounter(std::string node_id)
        : p_counter_(node_id), n_counter_(node_id) {}

    void increment(std::int64_t amount = 1) {
        p_counter_.increment(amount);
    }

    void decrement(std::int64_t amount = 1) {
        n_counter_.increment(amount);
    }

    std::int64_t value() const {
        return p_counter_.value() - n_counter_.value();
    }

    void merge(const PNCounter& other) {
        p_counter_.merge(other.p_counter_);
        n_counter_.merge(other.n_counter_);
    }

private:
    GCounter p_counter_;
    GCounter n_counter_;
};

// CRDT 3: LWW-Register (Last-Write-Wins Register)
template <typename T>
class LWWRegister {
public:
    LWWRegister() : timestamp_(0) {}
    LWWRegister(T value, std::uint64_t timestamp)
        : value_(std::move(value)), timestamp_(timestamp) {}

    void update(T value, std::uint64_t timestamp) {
        if (timestamp >= timestamp_) {
            value_ = std::move(value);
            timestamp_ = timestamp;
        }
    }

    void merge(const LWWRegister<T>& other) {
        if (other.timestamp_ > timestamp_) {
            value_ = other.value_;
            timestamp_ = other.timestamp_;
        }
    }

    const T& value() const { return value_; }
    std::uint64_t timestamp() const { return timestamp_; }

private:
    T value_{};
    std::uint64_t timestamp_ = 0;
};

} // namespace garuda::beam::cloud
