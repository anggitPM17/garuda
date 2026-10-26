#pragma once
// Garuda Language Embedded IoT Engine: Embedded Local MQTT Broker (`mqtt_broker.hpp`)
#include "hardware_drivers.hpp"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace garuda::beam::micro {

enum class MqttQos { Qos0 = 0, Qos1 = 1, Qos2 = 2 };

class MqttBroker {
public:
    MqttBroker() = default;

    bool init(uint16_t port = 1883) {
        port_ = port;
        initialized_ = true;
        return true;
    }

    bool is_initialized() const { return initialized_; }

    bool publish(const std::string& topic, const std::string& payload, MqttQos qos = MqttQos::Qos0) {
        if (!initialized_) return false;
        // High-precision 100µs CPU spin wait for simulated MQTT QoS 0 publish dispatch
        auto start_spin = std::chrono::high_resolution_clock::now();
        while (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start_spin).count() < 100) {}

        (void)qos;
        published_count_++;
        last_topic_ = topic;
        last_payload_ = payload;

        // Dispatch to subscriptions
        for (const auto& [sub_topic, cb] : subscriptions_) {
            if (sub_topic == topic || sub_topic == "#" || sub_topic == "+") {
                if (cb) cb(payload);
            }
        }
        return true;
    }

    bool subscribe(const std::string& topic, std::function<void(const std::string& msg)> cb) {
        if (!initialized_) return false;
        subscriptions_[topic] = cb;
        return true;
    }

    bool unsubscribe(const std::string& topic) {
        subscriptions_.erase(topic);
        return true;
    }

    std::size_t published_count() const { return published_count_; }
    const std::string& last_payload() const { return last_payload_; }

private:
    uint16_t port_ = 1883;
    bool initialized_ = false;
    std::size_t published_count_ = 0;
    std::string last_topic_;
    std::string last_payload_;
    std::map<std::string, std::function<void(const std::string& msg)>> subscriptions_;
};

} // namespace garuda::beam::micro
