#pragma once
// Garuda OTP: Distributed PubSub Engine (`pubsub.hpp`)
// Mengimplementasikan Publish/Subscribe ala Phoenix PubSub Elixir untuk penyiaran pesan massal ke jutaan proses.
#include "../core/runtime.hpp"

#include <algorithm>
#include <iostream>
#include <map>
#include <mutex>
#include <set>
#include <string>

namespace garuda::beam {

class PubSub {
public:
    explicit PubSub(Runtime& rt) : rt_(rt) {}

    void subscribe(const std::string& topic, Pid subscriber) {
        std::lock_guard<std::mutex> lock(mu_);
        topics_[topic].insert(subscriber);
        std::cout << "📢 [GARUDA PUBSUB] PID " << to_string(Term(subscriber)) << " subscribed to topic: '" << topic << "'" << std::endl;
    }

    void unsubscribe(const std::string& topic, Pid subscriber) {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = topics_.find(topic);
        if (it != topics_.end()) {
            it->second.erase(subscriber);
            if (it->second.empty()) topics_.erase(it);
        }
    }

    std::size_t publish(const std::string& topic, Term message) {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = topics_.find(topic);
        if (it == topics_.end()) return 0;

        std::size_t count = 0;
        for (Pid pid : it->second) {
            rt_.send(Pid{}, pid, tuple({atom("pubsub_msg"), Term(topic), message}));
            count++;
        }
        std::cout << "📡 [GARUDA PUBSUB] Broadcasted message to " << count << " subscribers on topic: '" << topic << "'" << std::endl;
        return count;
    }

    std::size_t subscriber_count(const std::string& topic) const {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = topics_.find(topic);
        if (it != topics_.end()) return it->second.size();
        return 0;
    }

private:
    Runtime& rt_;
    mutable std::mutex mu_;
    std::map<std::string, std::set<Pid>> topics_;
};

} // namespace garuda::beam
