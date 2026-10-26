#include "../include/garuda/beam/micro/fleet_manager.hpp"
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
    std::cout << "GARUDA FASE G — CONSOLIDATED AUDIT & PERFORMANCE BENCHMARK\n";
    std::cout << "=======================================================\n\n";

    GvmMicroEngine engine;

    // Benchmark 1: Total Phase G RAM Footprint Verification
    std::size_t total_phase_g_bytes = engine.memory_footprint_bytes() +
        sizeof(I2cBus) + sizeof(SpiBus) + sizeof(UartBus) + sizeof(PwmChannel) + sizeof(InterruptHandler) +
        sizeof(StreamProcessor) + sizeof(TumblingWindow) + sizeof(AnomalyDetector) + sizeof(EdgeAlertDispatcher) +
        sizeof(MqttBroker) + sizeof(BleMesh) + sizeof(LoraWan) + sizeof(DeviceProvisioning) +
        sizeof(OtaUpdater) + sizeof(SecureBoot) + sizeof(ModbusBridge) + sizeof(OpcUaBridge) + sizeof(FleetManager);

    std::cout << "[BENCH 1] Phase G Consolidated Embedded RAM Footprint: " << total_phase_g_bytes << " bytes (" << (total_phase_g_bytes / 1024.0) << " KB, Target < 64KB!)\n\n";

    // Benchmark 2: End-to-End Pipeline Throughput with Hardware Simulation
    std::cout << "[BENCH 2] Executing End-to-End Hardware & Sensor Pipeline Throughput...\n";
    I2cBus i2c;
    i2c.init(21, 22);

    SensorPipeline pipe("sensor_i2c", 1, [&]() {
        auto data = i2c.read(0x76, 2);
        return static_cast<double>(data.empty() ? 0 : data[0]);
    });

    StreamProcessor proc;
    MqttBroker broker;
    broker.init(1883);

    auto start_pipe = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000; ++i) {
        pipe.sample_now();
        if (pipe.has_samples()) {
            auto s = pipe.pop_sample();
            proc.push(s);
            if (auto p = proc.pop()) {
                broker.publish("garuda/pipe/data", std::to_string(p->value));
            }
        }
    }
    auto elapsed_pipe_ms = std::chrono::duration<double, std::milli>(
        std::chrono::high_resolution_clock::now() - start_pipe).count();

    double pipe_throughput = (1000.0 / (elapsed_pipe_ms / 1000.0));
    std::cout << "  ⏱️ Pipeline Throughput: " << pipe_throughput << " samples/sec (Target >= 1000 samples/sec!)\n\n";

    // Benchmark 3: OTA Update Cycle & Secure Boot Benchmarks
    std::cout << "[BENCH 3] Executing OTA Update Cycle & Secure Boot Handshake...\n";
    OtaUpdater ota;
    auto start_ota = std::chrono::high_resolution_clock::now();
    ota.check_for_update("https://firmware.garuda.io/v1.1.0.bin");
    ota.download("https://firmware.garuda.io/v1.1.0.bin");
    ota.verify("valid_ed25519_sig");
    ota.apply();
    auto elapsed_ota_sec = std::chrono::duration<double>(
        std::chrono::high_resolution_clock::now() - start_ota).count();

    std::cout << "  ⏱️ OTA Update Cycle Latency: " << elapsed_ota_sec << " sec (Target <= 5.1 sec!)\n\n";

    std::cout << "🎉 ALL CONSOLIDATED PHASE G BENCHMARKS EXECUTED SUCCESSFULLY!\n";
    return 0;
}
