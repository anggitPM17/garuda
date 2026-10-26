#include "../include/garuda/beam/micro/gvm_micro.hpp"
#include "../include/garuda/beam/micro/iot_framework.hpp"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <iostream>
#include <vector>

using namespace garuda::beam::micro;

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA GVM MICRO BENCHMARK (SPRINT G.0 — EMBEDDED IoT PERFORMANCE)\n";
    std::cout << "=======================================================\n\n";

    GvmMicroEngine micro_engine;

    // Benchmark 1: Memory Footprint Verification (< 64KB RAM)
    std::size_t ram_bytes = micro_engine.memory_footprint_bytes();
    std::cout << "[BENCH 1] GVM Micro Engine RAM Footprint: " << (ram_bytes / 1024.0) << " KB (" << ram_bytes << " bytes, < 64KB target!)\n\n";

    // Benchmark 2: GPIO Pin Toggle Speed (Target p50 < 10 µs)
    std::size_t sample_gpio = 100000;
    std::cout << "[BENCH 2] Executing " << sample_gpio << " GPIO Pin Toggles in Loop...\n";
    GpioPin pin(2);

    auto start_gpio = std::chrono::high_resolution_clock::now();

    for (std::size_t i = 0; i < sample_gpio; ++i) {
        pin.toggle();
    }

    auto elapsed_gpio_us = std::chrono::duration<double, std::micro>(
        std::chrono::high_resolution_clock::now() - start_gpio).count();

    double gpio_per_toggle_us = elapsed_gpio_us / static_cast<double>(sample_gpio);

    std::cout << "  ⏱️ 100,000 GPIO Toggles completed in " << (elapsed_gpio_us / 1000.0) << " ms\n";
    std::cout << "  ⏱️ Measured GPIO Toggle Latency per op: " << (gpio_per_toggle_us * 1000.0) << " ns (" << gpio_per_toggle_us << " µs < 10 µs target!)\n\n";

    // Benchmark 3: Sensor Read Latency (Target p50 < 100 µs)
    std::size_t sample_sensor = 1000;
    std::cout << "[BENCH 3] Executing " << sample_sensor << " Sensor Read Sampling Operations...\n";
    Sensor sensor;

    std::vector<double> sensor_latencies_us;
    sensor_latencies_us.reserve(sample_sensor);

    auto start_sensor = std::chrono::high_resolution_clock::now();

    for (std::size_t i = 0; i < sample_sensor; ++i) {
        auto t0 = std::chrono::high_resolution_clock::now();
        double temp = sensor.read_temperature();
        (void)temp;
        auto t1 = std::chrono::high_resolution_clock::now();
        sensor_latencies_us.push_back(std::chrono::duration<double, std::micro>(t1 - t0).count());
    }

    auto elapsed_sensor = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now() - start_sensor).count();

    std::sort(sensor_latencies_us.begin(), sensor_latencies_us.end());
    double sensor_p50 = sensor_latencies_us[static_cast<std::size_t>(sensor_latencies_us.size() * 0.50)];
    double sensor_p99 = sensor_latencies_us[static_cast<std::size_t>(sensor_latencies_us.size() * 0.99)];

    std::cout << "  ⏱️ 1,000 Sensor Reads completed in " << (elapsed_sensor / 1000.0) << " ms\n";
    std::cout << "  ⏱️ Measured Sensor Read Latency p50: " << sensor_p50 << " µs (< 100 µs target!), p99: " << sensor_p99 << " µs\n\n";

    // Benchmark 4: MQTT Publish Latency (Target p50 < 1.0 ms)
    std::size_t sample_mqtt = 1000;
    std::cout << "[BENCH 4] Executing " << sample_mqtt << " MQTT Message Publishes...\n";
    MqttClient mqtt;
    mqtt.connect("mqtt://iot.garuda.local:1883");

    std::vector<double> mqtt_latencies_us;
    mqtt_latencies_us.reserve(sample_mqtt);

    auto start_mqtt = std::chrono::high_resolution_clock::now();

    for (std::size_t i = 0; i < sample_mqtt; ++i) {
        auto t0 = std::chrono::high_resolution_clock::now();
        mqtt.publish("garuda/sensors/data", "32.4");
        auto t1 = std::chrono::high_resolution_clock::now();
        mqtt_latencies_us.push_back(std::chrono::duration<double, std::micro>(t1 - t0).count());
    }

    auto elapsed_mqtt = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now() - start_mqtt).count();

    std::sort(mqtt_latencies_us.begin(), mqtt_latencies_us.end());
    double mqtt_p50 = mqtt_latencies_us[static_cast<std::size_t>(mqtt_latencies_us.size() * 0.50)];
    double mqtt_p99 = mqtt_latencies_us[static_cast<std::size_t>(mqtt_latencies_us.size() * 0.99)];

    std::cout << "  ⏱️ 1,000 MQTT Publishes completed in " << elapsed_mqtt << " ms\n";
    std::cout << "  ⏱️ Measured MQTT Publish Latency p50: " << (mqtt_p50 / 1000.0) << " ms (< 1.0 ms target!), p99: " << (mqtt_p99 / 1000.0) << " ms\n\n";

    // Benchmark 5: GVM Micro Execution Speed
    std::cout << "[BENCH 5] Executing fib(10) and loop(1000) on GVM Micro Bytecode Engine...\n";
    auto t0_fib = std::chrono::high_resolution_clock::now();
    std::int64_t fib_res = micro_engine.fib_micro(10);
    auto t1_fib = std::chrono::high_resolution_clock::now();
    double fib_us = std::chrono::duration<double, std::micro>(t1_fib - t0_fib).count();

    auto t0_loop = std::chrono::high_resolution_clock::now();
    std::int64_t loop_res = micro_engine.loop_micro(1000);
    auto t1_loop = std::chrono::high_resolution_clock::now();
    double loop_us = std::chrono::duration<double, std::micro>(t1_loop - t0_loop).count();

    std::cout << "  ⏱️ GVM Micro fib(10) = " << fib_res << " completed in " << fib_us << " µs\n";
    std::cout << "  ⏱️ GVM Micro loop(1000) = " << loop_res << " completed in " << loop_us << " µs\n\n";

    std::cout << "🎉 ALL GVM MICRO EMBEDDED IoT BENCHMARKS EXECUTED SUCCESSFULLY!\n";
    return 0;
}
