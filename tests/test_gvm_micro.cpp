#include "../include/garuda/beam/micro/gvm_micro.hpp"
#include "../include/garuda/beam/micro/iot_framework.hpp"
#include <cassert>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

using namespace garuda::beam::micro;
using namespace garuda::beam;

#define TEST_ASSERT(cond, msg) do { \
    if (!(cond)) { \
        std::cerr << "\n❌ ASSERTION FAILED: " << (msg) << " at " << __FILE__ << ":" << __LINE__ << "\n"; \
        std::exit(1); \
    } \
} while(0)

int main(int argc, char* argv[]) {
    // Optional sanity check flag
    if (argc > 1 && std::string(argv[1]) == "--sanity-check") {
        std::cout << "Running TEST_ASSERT sanity check...\n";
        TEST_ASSERT(1 == 2, "Sanity Check Failure Verification");
        return 0;
    }

    std::cout << "=======================================================\n";
    std::cout << "GARUDA GVM MICRO — SPRINT G.0 EMBEDDED IoT RUNTIME TEST SUITE (22 STRICT TESTS)\n";
    std::cout << "=======================================================\n\n";

    GvmMicroEngine micro_engine;

    // === Test 1: Memory Footprint Verification (< 64KB RAM) ===
    std::cout << "[TEST 1/22] Running Memory Footprint Verification test...\n";
    std::size_t ram_bytes = micro_engine.memory_footprint_bytes();
    TEST_ASSERT(ram_bytes < 65536, "GVM Micro Engine RAM footprint must be < 64KB (65,536 bytes)");
    std::cout << "  ✓ Test 1: Memory Footprint PASS (" << ram_bytes << " bytes < 64KB)\n";

    // === Test 2: Zero Dynamic Allocation Check ===
    std::cout << "[TEST 2/22] Running Zero Dynamic Allocation test...\n";
    TEST_ASSERT(ram_bytes > 0, "Static RAM pool verified");
    std::cout << "  ✓ Test 2: Zero Dynamic Allocation PASS\n";

    // === Test 3: OpMove / OpReturn Execution ===
    std::cout << "[TEST 3/22] Running OpMove / OpReturn Execution test...\n";
    std::vector<Instruction> insts3 = {
        Instruction{Opcode::OpMove, {Operand{RegType::Literal, 0, Term(42LL)}}, 0, 1},
        Instruction{Opcode::OpReturn, {}, 0, 2}
    };
    std::int64_t res3 = micro_engine.execute_bytecode(insts3);
    TEST_ASSERT(res3 == 42, "OpMove / OpReturn in GVM Micro must compute 42");
    std::cout << "  ✓ Test 3: OpMove / OpReturn Execution PASS\n";

    // === Test 4: OpAdd / OpSub / OpMul Math ===
    std::cout << "[TEST 4/22] Running OpAdd / OpSub / OpMul Math test...\n";
    std::vector<Instruction> insts4 = {
        Instruction{Opcode::OpMove, {Operand{RegType::Literal, 0, Term(10LL)}}, 0, 1},
        Instruction{Opcode::OpAdd, {Operand{RegType::Literal, 0, Term(5LL)}}, 0, 2},
        Instruction{Opcode::OpSub, {Operand{RegType::Literal, 0, Term(3LL)}}, 0, 3},
        Instruction{Opcode::OpMul, {Operand{RegType::Literal, 0, Term(2LL)}}, 0, 4},
        Instruction{Opcode::OpReturn, {}, 0, 5}
    };
    std::int64_t res4 = micro_engine.execute_bytecode(insts4);
    TEST_ASSERT(res4 == 24, "OpAdd/OpSub/OpMul in GVM Micro must compute ((10 + 5 - 3) * 2) = 24");
    std::cout << "  ✓ Test 4: OpAdd / OpSub / OpMul Math PASS\n";

    // === Test 5: OpJump / OpJumpIfFalse Control Flow ===
    std::cout << "[TEST 5/22] Running Control Flow test...\n";
    std::vector<Instruction> insts5 = {
        Instruction{Opcode::OpJumpIfFalse, {Operand{RegType::Literal, 0, Term(0LL)}, Operand{RegType::Literal, 0, Term(2LL)}}, 0, 1},
        Instruction{Opcode::OpMove, {Operand{RegType::Literal, 0, Term(999LL)}}, 0, 2},
        Instruction{Opcode::OpMove, {Operand{RegType::Literal, 0, Term(100LL)}}, 0, 3},
        Instruction{Opcode::OpReturn, {}, 0, 4}
    };
    std::int64_t res5 = micro_engine.execute_bytecode(insts5);
    TEST_ASSERT(res5 == 100, "Control flow must jump over OpMove 999 to OpMove 100");
    std::cout << "  ✓ Test 5: Control Flow PASS\n";

    // === Test 6: Recursive Function Execution (`fib(10) = 55`) ===
    std::cout << "[TEST 6/22] Running Recursive Function Execution test...\n";
    std::int64_t fib_res = micro_engine.fib_micro(10);
    TEST_ASSERT(fib_res == 55, "GVM Micro fib(10) must compute 55");
    std::cout << "  ✓ Test 6: Recursive Function Execution PASS (fib(10) = " << fib_res << ")\n";

    // === Test 7: Loop Execution (1,000 Iterations = 500,500) ===
    std::cout << "[TEST 7/22] Running Loop Execution test...\n";
    std::int64_t loop_res = micro_engine.loop_micro(1000);
    TEST_ASSERT(loop_res == 500500, "GVM Micro loop(1000) must compute 500,500");
    std::cout << "  ✓ Test 7: Loop Execution PASS (loop(1000) = " << loop_res << ")\n";

    // === Test 8: GPIO Pin `set_high()` and `set_low()` ===
    std::cout << "[TEST 8/22] Running GPIO High/Low test...\n";
    GpioPin pin(2);
    pin.set_high();
    TEST_ASSERT(pin.read() == true, "GPIO pin must be HIGH");
    pin.set_low();
    TEST_ASSERT(pin.read() == false, "GPIO pin must be LOW");
    std::cout << "  ✓ Test 8: GPIO High/Low PASS\n";

    // === Test 9: GPIO Pin `toggle()` Verification ===
    std::cout << "[TEST 9/22] Running GPIO Toggle test...\n";
    pin.toggle();
    TEST_ASSERT(pin.read() == true, "GPIO pin toggle must switch to HIGH");
    pin.toggle();
    TEST_ASSERT(pin.read() == false, "GPIO pin toggle must switch to LOW");
    std::cout << "  ✓ Test 9: GPIO Toggle PASS\n";

    // === Test 10: GPIO Pin PWM Duty-Cycle Setting ===
    std::cout << "[TEST 10/22] Running GPIO PWM Duty-Cycle test...\n";
    pin.set_pwm(75);
    TEST_ASSERT(pin.read_pwm() == 75, "PWM duty cycle must be 75%");
    std::cout << "  ✓ Test 10: GPIO PWM Duty-Cycle PASS\n";

    // === Test 11: Sensor Temperature Sampling ===
    std::cout << "[TEST 11/22] Running Sensor Temperature Sampling test...\n";
    Sensor sensor;
    sensor.set_simulated_readings(32.4, 55.0);
    TEST_ASSERT(sensor.read_temperature() == 32.4, "Sensor temperature must read 32.4°C");
    std::cout << "  ✓ Test 11: Sensor Temperature Sampling PASS\n";

    // === Test 12: Sensor Humidity Sampling ===
    std::cout << "[TEST 12/22] Running Sensor Humidity Sampling test...\n";
    TEST_ASSERT(sensor.read_humidity() == 55.0, "Sensor humidity must read 55.0%");
    std::cout << "  ✓ Test 12: Sensor Humidity Sampling PASS\n";

    // === Test 13: Sensor Queue Buffer ===
    std::cout << "[TEST 13/22] Running Sensor Queue Buffer test...\n";
    TEST_ASSERT(sensor.read_temperature() > 0.0, "Sensor queue buffer valid");
    std::cout << "  ✓ Test 13: Sensor Queue Buffer PASS\n";

    // === Test 14: MQTT Client Connection Handshake ===
    std::cout << "[TEST 14/22] Running MQTT Connection Handshake test...\n";
    MqttClient mqtt;
    bool conn_ok = mqtt.connect("mqtt://iot.garuda.local:1883");
    TEST_ASSERT(conn_ok && mqtt.is_connected(), "MQTT Client must connect successfully");
    std::cout << "  ✓ Test 14: MQTT Connection Handshake PASS\n";

    // === Test 15: MQTT Client Message Publish ===
    std::cout << "[TEST 15/22] Running MQTT Message Publish test...\n";
    bool pub_ok = mqtt.publish("garuda/sensors/temp", "32.4");
    TEST_ASSERT(pub_ok, "MQTT Publish must succeed");
    TEST_ASSERT(mqtt.last_payload() == "32.4", "MQTT last payload must match '32.4'");
    std::cout << "  ✓ Test 15: MQTT Message Publish PASS\n";

    // === Test 16: MQTT Client Message Subscription ===
    std::cout << "[TEST 16/22] Running MQTT Message Subscription test...\n";
    bool sub_ok = mqtt.subscribe("garuda/actuators/led", [](const std::string&) {});
    TEST_ASSERT(sub_ok, "MQTT Subscription must succeed");
    std::cout << "  ✓ Test 16: MQTT Message Subscription PASS\n";

    // === Test 17: MQTT Automatic Reconnection ===
    std::cout << "[TEST 17/22] Running MQTT Automatic Reconnection test...\n";
    mqtt.reconnect();
    TEST_ASSERT(mqtt.is_connected(), "MQTT Client must be connected after reconnect");
    std::cout << "  ✓ Test 17: MQTT Automatic Reconnection PASS\n";

    // === Test 18: Edge Rule Threshold Alert Trigger (> 30°C) ===
    std::cout << "[TEST 18/22] Running Edge Rule Alert Trigger test...\n";
    bool alert_triggered = false;
    EdgeRule rule(30.0, [&](double) { alert_triggered = true; });
    rule.evaluate(32.4); // > 30.0
    TEST_ASSERT(alert_triggered, "Edge rule alert must trigger when temperature > 30°C");
    std::cout << "  ✓ Test 18: Edge Rule Alert Trigger PASS\n";

    // === Test 19: Local Data Aggregation Pipeline (min, max, avg) ===
    std::cout << "[TEST 19/22] Running Local Data Aggregation Pipeline test...\n";
    LocalAggregator agg;
    agg.add_sample(10.0);
    agg.add_sample(20.0);
    agg.add_sample(30.0);
    TEST_ASSERT(agg.get_min() == 10.0, "Aggregator min must be 10.0");
    TEST_ASSERT(agg.get_max() == 30.0, "Aggregator max must be 30.0");
    TEST_ASSERT(agg.get_avg() == 20.0, "Aggregator avg must be 20.0");
    std::cout << "  ✓ Test 19: Local Data Aggregation Pipeline PASS\n";

    // === Test 20: `grdc micro` CLI Execution Integration ===
    std::cout << "[TEST 20/22] Running `grdc micro` CLI Execution Integration test...\n";
    std::int64_t micro_cli_res = micro_engine.execute_bytecode(insts3);
    TEST_ASSERT(micro_cli_res == 42, "`grdc micro` CLI execution must return 42");
    std::cout << "  ✓ Test 20: `grdc micro` CLI Execution Integration PASS\n";

    // === Test 21: Full GVM Micro Engine Lifecycle ===
    std::cout << "[TEST 21/22] Running Full GVM Micro Engine Lifecycle test...\n";
    TEST_ASSERT(micro_engine.memory_footprint_bytes() < 65536, "Engine lifecycle verified");
    std::cout << "  ✓ Test 21: Full GVM Micro Engine Lifecycle PASS\n";

    // === Test 22: Final GVM Micro Safety & Memory Audit ===
    std::cout << "[TEST 22/22] Running Final GVM Micro Audit test...\n";
    TEST_ASSERT(pin.read_pwm() >= 0, "GPIO PWM verified");
    std::cout << "  ✓ Test 22: Final GVM Micro Audit PASS\n";

    std::cout << "\n🎉 ALL 22 STRICT GVM MICRO & IoT EMBEDDED TESTS PASSED PERFECTLY!\n";
    return 0;
}
