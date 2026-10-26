#pragma once
// Garuda Language Embedded IoT Engine: LoRaWAN Abstraction (`lorawan.hpp`)
#include "ble_mesh.hpp"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

namespace garuda::beam::micro {

class LoraWan {
public:
    LoraWan(std::string dev_eui, std::string app_eui, std::string app_key)
        : dev_eui_(std::move(dev_eui)), app_eui_(std::move(app_eui)), app_key_(std::move(app_key)) {}

    bool join() {
        // High-precision 2.0-second (2000ms) delay for simulated LoRaWAN OTAA join handshake
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        joined_ = true;
        return true;
    }

    bool is_joined() const { return joined_; }

    bool send(const std::string& payload, bool confirmed = false) {
        if (!joined_) return false;
        // High-precision 5ms (5000µs) CPU spin wait for simulated LoRaWAN transmission
        auto start_spin = std::chrono::high_resolution_clock::now();
        while (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start_spin).count() < 5000) {}

        (void)confirmed;
        last_sent_payload_ = payload;
        sent_count_++;
        return true;
    }

    void on_receive(std::function<void(const std::string& downlink)> cb) {
        rx_cb_ = std::move(cb);
    }

    std::size_t sent_count() const { return sent_count_; }

private:
    std::string dev_eui_;
    std::string app_eui_;
    std::string app_key_;
    bool joined_ = false;
    std::string last_sent_payload_;
    std::size_t sent_count_ = 0;
    std::function<void(const std::string&)> rx_cb_;
};

} // namespace garuda::beam::micro
