#include "../include/garuda/beam/micro/device_provisioning.hpp"
#include "../include/garuda/beam/micro/edge_alert_dispatcher.hpp"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <iostream>
#include <vector>

using namespace garuda::beam::micro;

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA IoT FRAMEWORK BENCHMARK (SPRINT G.3 — MESH NETWORKING PERFORMANCE)\n";
    std::cout << "=======================================================\n\n";

    // Benchmark 1: MQTT Broker Publish Latency (Target <= 100 µs)
    MqttBroker broker;
    broker.init(1883);
    std::size_t sample_broker = 1000;
    std::cout << "[BENCH 1] Executing " << sample_broker << " MQTT Broker Local Publishes...\n";

    std::vector<double> broker_latencies_us;
    broker_latencies_us.reserve(sample_broker);

    for (std::size_t i = 0; i < sample_broker; ++i) {
        auto t0 = std::chrono::high_resolution_clock::now();
        broker.publish("garuda/sensors/temp", "26.8");
        auto t1 = std::chrono::high_resolution_clock::now();
        broker_latencies_us.push_back(std::chrono::duration<double, std::micro>(t1 - t0).count());
    }

    std::sort(broker_latencies_us.begin(), broker_latencies_us.end());
    double broker_p50 = broker_latencies_us[static_cast<std::size_t>(broker_latencies_us.size() * 0.50)];
    std::cout << "  ⏱️ Measured MQTT Broker Local Publish Latency p50: " << broker_p50 << " µs (Target <= 100 µs!)\n\n";

    // Benchmark 2: BLE Mesh Send Latency (Target <= 5.0 ms per hop)
    BleMesh mesh(101);
    mesh.init();
    std::size_t sample_mesh = 100;
    std::cout << "[BENCH 2] Executing " << sample_mesh << " BLE Mesh Multi-Hop Transmission Operations...\n";

    std::vector<double> mesh_latencies_ms;
    mesh_latencies_ms.reserve(sample_mesh);

    for (std::size_t i = 0; i < sample_mesh; ++i) {
        auto t0 = std::chrono::high_resolution_clock::now();
        mesh.send(202, "mesh_data_payload");
        auto t1 = std::chrono::high_resolution_clock::now();
        mesh_latencies_ms.push_back(std::chrono::duration<double, std::milli>(t1 - t0).count());
    }

    std::sort(mesh_latencies_ms.begin(), mesh_latencies_ms.end());
    double mesh_p50 = mesh_latencies_ms[static_cast<std::size_t>(mesh_latencies_ms.size() * 0.50)];
    std::cout << "  ⏱️ Measured BLE Mesh Send Latency p50 per hop: " << mesh_p50 << " ms (Target <= 5.0 ms!)\n\n";

    // Benchmark 3: LoRaWAN Join & Uplink Latency (Target Join <= 2.0s, Send <= 1.0s)
    LoraWan lorawan("0004A30B001A2B3C", "70B3D57ED0001234", "0102030405060708090A0B0C0D0E0F10");
    std::cout << "[BENCH 3] Executing LoRaWAN OTAA Join & Uplink Transmission...\n";

    auto t0_join = std::chrono::high_resolution_clock::now();
    lorawan.join();
    auto t1_join = std::chrono::high_resolution_clock::now();
    double join_ms = std::chrono::duration<double, std::milli>(t1_join - t0_join).count();

    auto t0_send = std::chrono::high_resolution_clock::now();
    lorawan.send("uplink_data");
    auto t1_send = std::chrono::high_resolution_clock::now();
    double send_ms = std::chrono::duration<double, std::milli>(t1_send - t0_send).count();

    std::cout << "  ⏱️ LoRaWAN OTAA Join Latency: " << (join_ms / 1000.0) << " sec (Target <= 2.0 sec!)\n";
    std::cout << "  ⏱️ LoRaWAN Uplink Transmission Latency: " << (send_ms / 1000.0) << " sec (Target <= 1.0 sec!)\n\n";

    // Benchmark 4: End-to-End Pipeline Throughput with Real I2C Read Simulation (50µs)
    std::cout << "[BENCH 4] Executing End-to-End Pipeline Throughput Benchmark with Simulated I2C Read (50µs)...\n";
    I2cBus i2c_e2e;
    i2c_e2e.init(21, 22);

    SensorPipeline pipe("i2c_temp_pipe", 1, [&]() {
        auto data = i2c_e2e.read(0x76, 2); // 50µs simulated I2C read
        return static_cast<double>(data.empty() ? 0 : data[0]);
    });

    StreamProcessor stream_proc;
    MqttBroker e2e_broker;
    e2e_broker.init(1883);

    auto start_e2e = std::chrono::high_resolution_clock::now();
    std::size_t processed_samples = 0;

    for (int i = 0; i < 1000; ++i) {
        pipe.sample_now();
        if (pipe.has_samples()) {
            auto s = pipe.pop_sample();
            stream_proc.push(s);
            if (auto p = stream_proc.pop()) {
                e2e_broker.publish("garuda/sensors/i2c", std::to_string(p->value));
                processed_samples++;
            }
        }
    }

    auto elapsed_e2e_ms = std::chrono::duration<double, std::milli>(
        std::chrono::high_resolution_clock::now() - start_e2e).count();

    double e2e_throughput = (processed_samples / (elapsed_e2e_ms / 1000.0));

    std::cout << "  ⏱️ End-to-End Pipeline: " << processed_samples << " samples processed in " << elapsed_e2e_ms << " ms\n";
    std::cout << "  ⏱️ Measured End-to-End Throughput: " << e2e_throughput << " samples/sec (Target >= 1000 samples/sec!)\n\n";

    std::cout << "🎉 ALL IoT FRAMEWORK & MESH NETWORKING BENCHMARKS EXECUTED SUCCESSFULLY!\n";
    return 0;
}
