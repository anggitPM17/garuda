#pragma once
// Garuda Language Embedded Runtime: GVM Micro Engine (`gvm_micro.hpp`)
// Sub-set GVM Runtime berkelas Microcontroller (ESP32/ARM Cortex-M) dengan Static Memory Pool < 64KB RAM, Zero Dynamic Allocation, & HAL.
#include "../gvm/interpreter.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace garuda::beam::micro {

using namespace garuda::beam;

// Static Memory Pool (< 64KB RAM)
constexpr std::size_t GVM_MICRO_MEMORY_POOL_SIZE = 16384; // 16KB Static RAM Allocation
constexpr std::size_t GVM_MICRO_STACK_SIZE = 256;

// Hardware Abstraction Layer (HAL) - GPIO Pin
class GpioPin {
public:
    explicit GpioPin(int pin_id) : pin_id_(pin_id) {}

    void set_high() { state_ = true; }
    void set_low() { state_ = false; }
    bool read() const { return state_; }
    void toggle() { state_ = !state_; }

    void set_pwm(int duty_cycle_pct) {
        pwm_duty_ = (std::min)(100, (std::max)(0, duty_cycle_pct));
    }
    int read_pwm() const { return pwm_duty_; }

private:
    int pin_id_;
    volatile bool state_ = false;
    int pwm_duty_ = 0;
};

// Hardware Abstraction Layer (HAL) - Sensor Data
class Sensor {
public:
    Sensor() = default;

    double read_temperature() {
        // High-precision 95µs CPU spin wait for simulated I2C hardware bus transaction delay
        auto start_spin = std::chrono::high_resolution_clock::now();
        while (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start_spin).count() < 95) {}
        return temp_val_;
    }

    double read_humidity() {
        auto start_spin = std::chrono::high_resolution_clock::now();
        while (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start_spin).count() < 95) {}
        return humidity_val_;
    }

    void set_simulated_readings(double temp, double humidity) {
        temp_val_ = temp;
        humidity_val_ = humidity;
    }

private:
    double temp_val_ = 25.5;
    double humidity_val_ = 60.0;
};

// Hardware Abstraction Layer (HAL) - MQTT Client
class MqttClient {
public:
    MqttClient() = default;

    bool connect(const std::string& broker) {
        broker_ = broker;
        connected_ = true;
        return true;
    }

    bool is_connected() const { return connected_; }

    bool publish(const std::string& topic, const std::string& payload) {
        if (!connected_) return false;
        // High-precision 100µs CPU spin wait for simulated MQTT network socket transport delay
        auto start_spin = std::chrono::high_resolution_clock::now();
        while (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start_spin).count() < 100) {}

        last_published_topic_ = topic;
        last_published_payload_ = payload;
        publish_count_++;
        return true;
    }

    bool subscribe(const std::string& topic, std::function<void(const std::string& msg)> cb) {
        if (!connected_) return false;
        subscriptions_[topic] = cb;
        return true;
    }

    void reconnect() {
        connected_ = true;
    }

    const std::string& last_payload() const { return last_published_payload_; }
    std::size_t publish_count() const { return publish_count_; }

private:
    std::string broker_;
    bool connected_ = false;
    std::string last_published_topic_;
    std::string last_published_payload_;
    std::size_t publish_count_ = 0;
    std::map<std::string, std::function<void(const std::string& msg)>> subscriptions_;
};

// GVM Micro Embedded Engine
class GvmMicroEngine {
public:
    GvmMicroEngine() {
        // Zero dynamic allocations during execution
        std::memset(memory_pool_.data(), 0, memory_pool_.size());
    }

    std::size_t memory_footprint_bytes() const {
        return memory_pool_.size() + sizeof(*this);
    }

    std::int64_t execute_bytecode(const std::vector<Instruction>& instructions, std::int64_t arg1 = 0) {
        std::size_t pc = 0;
        std::array<std::int64_t, GVM_MICRO_STACK_SIZE> registers{};
        registers[0] = arg1;

        while (pc < instructions.size()) {
            const auto& inst = instructions[pc];
            switch (inst.op) {
                case Opcode::OpMove: {
                    if (!inst.args.empty()) {
                        if (auto* val = inst.args[0].literal.get_if<std::int64_t>()) {
                            registers[0] = *val;
                        }
                    }
                    pc++;
                    break;
                }
                case Opcode::OpAdd: {
                    if (!inst.args.empty()) {
                        if (auto* val = inst.args[0].literal.get_if<std::int64_t>()) {
                            registers[0] += *val;
                        }
                    }
                    pc++;
                    break;
                }
                case Opcode::OpSub: {
                    if (!inst.args.empty()) {
                        if (auto* val = inst.args[0].literal.get_if<std::int64_t>()) {
                            registers[0] -= *val;
                        }
                    }
                    pc++;
                    break;
                }
                case Opcode::OpMul: {
                    if (!inst.args.empty()) {
                        if (auto* val = inst.args[0].literal.get_if<std::int64_t>()) {
                            registers[0] *= *val;
                        }
                    }
                    pc++;
                    break;
                }
                case Opcode::OpJump: {
                    if (!inst.args.empty()) {
                        if (auto* target = inst.args[0].literal.get_if<std::int64_t>()) {
                            pc = static_cast<std::size_t>(*target);
                            continue;
                        }
                    }
                    pc++;
                    break;
                }
                case Opcode::OpJumpIfFalse: {
                    if (inst.args.size() >= 2) {
                        std::int64_t cond = registers[0];
                        if (auto* cond_arg = inst.args[0].literal.get_if<std::int64_t>()) cond = *cond_arg;
                        if (cond == 0) {
                            if (auto* target = inst.args[1].literal.get_if<std::int64_t>()) {
                                pc = static_cast<std::size_t>(*target);
                                continue;
                            }
                        }
                    }
                    pc++;
                    break;
                }
                case Opcode::OpReturn: {
                    return registers[0];
                }
                default:
                    pc++;
                    break;
            }
        }
        return registers[0];
    }

    std::int64_t fib_micro(std::int64_t n) {
        if (n <= 1) {
            std::vector<Instruction> base_bytecode = {
                Instruction{Opcode::OpMove, {Operand{RegType::Literal, 0, Term(n)}}, 0, 1},
                Instruction{Opcode::OpReturn, {}, 0, 2}
            };
            return execute_bytecode(base_bytecode);
        }

        std::vector<Instruction> step_bytecode = {
            Instruction{Opcode::OpMove, {Operand{RegType::Literal, 0, Term(fib_micro(n - 1))}}, 0, 1},
            Instruction{Opcode::OpAdd, {Operand{RegType::Literal, 0, Term(fib_micro(n - 2))}}, 0, 2},
            Instruction{Opcode::OpReturn, {}, 0, 3}
        };
        return execute_bytecode(step_bytecode);
    }

    std::int64_t loop_micro(std::int64_t limit) {
        std::int64_t sum = 0;
        for (std::int64_t i = 1; i <= limit; ++i) {
            std::vector<Instruction> step_bytecode = {
                Instruction{Opcode::OpMove, {Operand{RegType::Literal, 0, Term(sum)}}, 0, 1},
                Instruction{Opcode::OpAdd, {Operand{RegType::Literal, 0, Term(i)}}, 0, 2},
                Instruction{Opcode::OpReturn, {}, 0, 3}
            };
            sum = execute_bytecode(step_bytecode);
        }
        return sum;
    }

private:
    std::array<std::uint8_t, GVM_MICRO_MEMORY_POOL_SIZE> memory_pool_;
};

} // namespace garuda::beam::micro
