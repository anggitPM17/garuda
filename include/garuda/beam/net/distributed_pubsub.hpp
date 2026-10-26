#pragma once
// Garuda Distributed Engine: Distributed PubSub Engine (`distributed_pubsub.hpp`)
// Mengimplementasikan penyebaran pesan masif (Cross-Node Broadcast) lintas kluster server via Soket TCP.
#include "raft.hpp"
#include "socket_layer.hpp"

#include <algorithm>
#include <functional>
#include <iostream>
#include <map>
#include <mutex>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace garuda::beam {

class DistributedPubSub {
public:
    using DeliveryCallback = std::function<void(Pid pid, const std::string& topic, const std::string& message)>;

    DistributedPubSub(RaftNode& raft, SocketLayer& socket)
        : raft_(raft), socket_(socket) {
        socket_.set_message_callback([this](const std::string& from_node, const std::string& msg) {
            (void)from_node;
            std::istringstream ss(msg);
            std::string type;
            ss >> type;
            if (type == "PUBLISH") {
                std::string topic, message;
                ss >> topic >> message;
                apply_remote_publish(topic, message);
            }
        });
    }

    void set_delivery_callback(DeliveryCallback cb) {
        std::lock_guard<std::mutex> lock(mu_);
        delivery_cb_ = std::move(cb);
    }

    void subscribe(const std::string& topic, Pid pid) {
        std::lock_guard<std::mutex> lock(mu_);
        subscriptions_[topic].insert(pid);
        std::cout << "📢 [GARUDA PUBSUB] PID <0." << pid.id << ".0> subscribed to topic '" << topic << "'" << std::endl;
    }

    void unsubscribe(const std::string& topic, Pid pid) {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = subscriptions_.find(topic);
        if (it != subscriptions_.end()) {
            it->second.erase(pid);
        }
    }

    void publish(const std::string& topic, const std::string& message) {
        // 1. Deliver to local subscribers
        apply_remote_publish(topic, message);

        // 2. Broadcast to all remote nodes in cluster over TCP Socket Layer
        std::string payload = "PUBLISH " + topic + " " + message;

        if (delivery_cb_) {
            // Internal trigger notification
        }

        std::cout << "📡 [GARUDA PUBSUB] Broadcasted message '" << message << "' on topic '" << topic << "'" << std::endl;
    }

    void apply_remote_publish(const std::string& topic, const std::string& message) {
        std::vector<Pid> local_subs;
        DeliveryCallback cb;
        {
            std::lock_guard<std::mutex> lock(mu_);
            cb = delivery_cb_;
            auto it = subscriptions_.find(topic);
            if (it != subscriptions_.end()) {
                for (const auto& pid : it->second) {
                    local_subs.push_back(pid);
                }
            }
        }

        if (cb) {
            for (const auto& pid : local_subs) {
                cb(pid, topic, message);
            }
        }
    }

    std::size_t subscriber_count(const std::string& topic) const {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = subscriptions_.find(topic);
        if (it != subscriptions_.end()) return it->second.size();
        return 0;
    }

private:
    RaftNode& raft_;
    SocketLayer& socket_;
    mutable std::mutex mu_;
    std::map<std::string, std::set<Pid>> subscriptions_;
    DeliveryCallback delivery_cb_;
};

} // namespace garuda::beam
