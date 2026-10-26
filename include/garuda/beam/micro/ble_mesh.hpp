#pragma once
// Garuda Language Embedded IoT Engine: BLE Mesh Networking (`ble_mesh.hpp`)
#include "mqtt_broker.hpp"

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

class BleMesh {
public:
    explicit BleMesh(uint16_t node_id = 1) : node_id_(node_id) {}

    bool init() {
        initialized_ = true;
        return true;
    }

    bool is_initialized() const { return initialized_; }

    bool send(uint16_t dest_id, const std::string& payload) {
        if (!initialized_) return false;
        // High-precision 5ms (5000µs) CPU spin wait per hop for simulated BLE mesh routing
        auto start_spin = std::chrono::high_resolution_clock::now();
        while (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start_spin).count() < 5000) {}

        (void)dest_id;
        last_sent_payload_ = payload;
        msg_count_++;
        return true;
    }

    bool broadcast(const std::string& payload) {
        return send(0xFFFF, payload);
    }

    void on_receive(std::function<void(uint16_t src, const std::string& msg)> cb) {
        rx_callback_ = std::move(cb);
    }

    std::size_t msg_count() const { return msg_count_; }

private:
    uint16_t node_id_;
    bool initialized_ = false;
    std::string last_sent_payload_;
    std::size_t msg_count_ = 0;
    std::function<void(uint16_t, const std::string&)> rx_callback_;
};

} // namespace garuda::beam::micro
