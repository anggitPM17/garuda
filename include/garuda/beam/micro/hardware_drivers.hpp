#pragma once
// Garuda Language Embedded Hardware Drivers: I2C, SPI, UART, PWM, & Interrupts (`hardware_drivers.hpp`)
#include "gvm_micro.hpp"

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

// I2C Hardware Bus Driver
class I2cBus {
public:
    I2cBus() = default;

    bool init(int sda_pin, int scl_pin, std::uint32_t frequency_hz = 400000) {
        sda_pin_ = sda_pin;
        scl_pin_ = scl_pin;
        frequency_hz_ = frequency_hz;
        initialized_ = true;
        return true;
    }

    bool is_initialized() const { return initialized_; }

    bool write(std::uint8_t device_addr, const std::vector<std::uint8_t>& data) {
        if (!initialized_) return false;
        // High-precision 50µs CPU spin wait for simulated I2C bus write transaction
        auto start_spin = std::chrono::high_resolution_clock::now();
        while (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start_spin).count() < 50) {}

        last_device_addr_ = device_addr;
        last_write_data_ = data;
        return true;
    }

    std::vector<std::uint8_t> read(std::uint8_t device_addr, std::size_t length) {
        if (!initialized_) return {};
        // High-precision 50µs CPU spin wait for simulated I2C bus read transaction
        auto start_spin = std::chrono::high_resolution_clock::now();
        while (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start_spin).count() < 50) {}

        last_device_addr_ = device_addr;
        std::vector<std::uint8_t> result(length, 0xAB);
        return result;
    }

    std::uint8_t last_device_addr() const { return last_device_addr_; }

private:
    int sda_pin_ = -1;
    int scl_pin_ = -1;
    std::uint32_t frequency_hz_ = 400000;
    bool initialized_ = false;
    std::uint8_t last_device_addr_ = 0;
    std::vector<std::uint8_t> last_write_data_;
};

// SPI Hardware Bus Driver
class SpiBus {
public:
    SpiBus() = default;

    bool init(int mosi, int miso, int sck, int cs, std::uint32_t frequency_hz = 10000000) {
        mosi_pin_ = mosi;
        miso_pin_ = miso;
        sck_pin_ = sck;
        cs_pin_ = cs;
        frequency_hz_ = frequency_hz;
        initialized_ = true;
        return true;
    }

    bool is_initialized() const { return initialized_; }

    std::vector<std::uint8_t> transfer(const std::vector<std::uint8_t>& tx_data) {
        if (!initialized_) return {};
        // High-precision 20µs CPU spin wait per byte for simulated SPI transfer delay
        std::size_t spin_target_us = 20 * (std::max)(std::size_t(1), tx_data.size());
        auto start_spin = std::chrono::high_resolution_clock::now();
        while (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start_spin).count() < static_cast<std::int64_t>(spin_target_us)) {}

        std::vector<std::uint8_t> rx_data = tx_data; // full-duplex loopback simulation
        return rx_data;
    }

private:
    int mosi_pin_ = -1;
    int miso_pin_ = -1;
    int sck_pin_ = -1;
    int cs_pin_ = -1;
    std::uint32_t frequency_hz_ = 10000000;
    bool initialized_ = false;
};

// UART Serial Hardware Driver
class UartBus {
public:
    UartBus() = default;

    bool init(int tx_pin, int rx_pin, std::uint32_t baud_rate = 115200) {
        tx_pin_ = tx_pin;
        rx_pin_ = rx_pin;
        baud_rate_ = baud_rate;
        initialized_ = true;
        return true;
    }

    bool is_initialized() const { return initialized_; }

    bool write(const std::string& data) {
        if (!initialized_) return false;
        // High-precision 100µs CPU spin wait per byte for simulated UART serial transmission
        std::size_t spin_target_us = 100 * (std::max)(std::size_t(1), data.size());
        auto start_spin = std::chrono::high_resolution_clock::now();
        while (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start_spin).count() < static_cast<std::int64_t>(spin_target_us)) {}

        tx_buffer_ += data;
        return true;
    }

    std::string read(std::size_t length) {
        if (!initialized_) return "";
        auto start_spin = std::chrono::high_resolution_clock::now();
        while (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start_spin).count() < static_cast<std::int64_t>(100 * length)) {}

        return std::string(length, 'U');
    }

private:
    int tx_pin_ = -1;
    int rx_pin_ = -1;
    std::uint32_t baud_rate_ = 115200;
    bool initialized_ = false;
    std::string tx_buffer_;
};

// PWM Hardware Channel Driver
class PwmChannel {
public:
    PwmChannel() = default;

    bool init(int pin, std::uint32_t frequency_hz = 5000, int resolution_bits = 8) {
        pin_ = pin;
        frequency_hz_ = frequency_hz;
        resolution_bits_ = resolution_bits;
        initialized_ = true;
        return true;
    }

    bool is_initialized() const { return initialized_; }

    bool set_duty_cycle(int duty_pct) {
        if (!initialized_) return false;
        // High-precision 1µs CPU spin wait for simulated PWM duty cycle adjustment
        auto start_spin = std::chrono::high_resolution_clock::now();
        while (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start_spin).count() < 1) {}

        duty_pct_ = (std::min)(100, (std::max)(0, duty_pct));
        return true;
    }

    int duty_cycle() const { return duty_pct_; }

private:
    int pin_ = -1;
    std::uint32_t frequency_hz_ = 5000;
    int resolution_bits_ = 8;
    bool initialized_ = false;
    int duty_pct_ = 0;
};

// Hardware Interrupt Handler (IRQ)
enum class InterruptEdge { Rising, Falling, Change };

class InterruptHandler {
public:
    InterruptHandler() = default;

    bool attach(int pin, InterruptEdge edge, std::function<void()> cb) {
        // High-precision 2µs CPU spin wait for simulated IRQ register attachment
        auto start_spin = std::chrono::high_resolution_clock::now();
        while (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start_spin).count() < 2) {}

        handlers_[pin] = cb;
        edges_[pin] = edge;
        return true;
    }

    bool trigger_interrupt(int pin) {
        // High-precision 5µs CPU spin wait for simulated hardware IRQ handler invocation
        auto start_spin = std::chrono::high_resolution_clock::now();
        while (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start_spin).count() < 5) {}

        auto it = handlers_.find(pin);
        if (it != handlers_.end() && it->second) {
            it->second();
            return true;
        }
        return false;
    }

    bool detach(int pin) {
        handlers_.erase(pin);
        edges_.erase(pin);
        return true;
    }

private:
    std::map<int, std::function<void()>> handlers_;
    std::map<int, InterruptEdge> edges_;
};

} // namespace garuda::beam::micro
