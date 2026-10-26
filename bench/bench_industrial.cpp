#include "../include/garuda/beam/micro/fleet_manager.hpp"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <iostream>
#include <vector>

using namespace garuda::beam::micro;

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA INDUSTRIAL ENGINE BENCHMARK (SPRINT G.4 — CERTIFICATION PERFORMANCE)\n";
    std::cout << "=======================================================\n\n";

    // Benchmark 1: OTA Update Cycle Latency (Target <= 5.0s)
    OtaUpdater ota;
    std::cout << "[BENCH 1] Executing OTA Firmware Download, Verification & Flash Apply...\n";
    auto t0_ota = std::chrono::high_resolution_clock::now();
    ota.check_for_update("https://firmware.garuda.io/v1.1.0.bin");
    ota.download("https://firmware.garuda.io/v1.1.0.bin");
    ota.verify("valid_ed25519_sig");
    ota.apply();
    auto t1_ota = std::chrono::high_resolution_clock::now();
    double ota_ms = std::chrono::duration<double, std::milli>(t1_ota - t0_ota).count();
    std::cout << "  ⏱️ OTA Update Cycle Latency: " << (ota_ms / 1000.0) << " sec (Target <= 5.0 sec!)\n\n";

    // Benchmark 2: Secure Boot Firmware Signature Verification (Target <= 100 ms)
    SecureBoot sec_boot;
    std::cout << "[BENCH 2] Executing Secure Boot Signature Verification...\n";
    auto t0_sb = std::chrono::high_resolution_clock::now();
    sec_boot.verify_firmware("valid_hash_123", "valid_sig_123");
    sec_boot.measure_boot();
    auto t1_sb = std::chrono::high_resolution_clock::now();
    double sb_ms = std::chrono::duration<double, std::milli>(t1_sb - t0_sb).count();
    std::cout << "  ⏱️ Secure Boot Verification Latency: " << sb_ms << " ms (Target <= 100 ms!)\n\n";

    // Benchmark 3: Modbus RTU Register Read/Write Latency (Target <= 10 ms)
    ModbusBridge modbus;
    modbus.init(1, 9600);
    std::size_t sample_mb = 100;
    std::cout << "[BENCH 3] Executing " << sample_mb << " Modbus RTU Register Transactions...\n";

    std::vector<double> mb_latencies_ms;
    mb_latencies_ms.reserve(sample_mb);

    for (std::size_t i = 0; i < sample_mb; ++i) {
        auto t0 = std::chrono::high_resolution_clock::now();
        modbus.write_register(0x0100, static_cast<uint16_t>(i));
        auto t1 = std::chrono::high_resolution_clock::now();
        mb_latencies_ms.push_back(std::chrono::duration<double, std::milli>(t1 - t0).count());
    }

    std::sort(mb_latencies_ms.begin(), mb_latencies_ms.end());
    double mb_p50 = mb_latencies_ms[static_cast<std::size_t>(mb_latencies_ms.size() * 0.50)];
    std::cout << "  ⏱️ Measured Modbus RTU Transaction Latency p50: " << mb_p50 << " ms (Target <= 10.0 ms!)\n\n";

    // Benchmark 4: OPC-UA Client Node Read/Write Latency (Target <= 5.0 ms)
    OpcUaBridge opcua;
    opcua.init("opc.tcp://industrial.garuda.local:4840");
    opcua.connect();
    std::size_t sample_opc = 100;
    std::cout << "[BENCH 4] Executing " << sample_opc << " OPC-UA Node Transactions...\n";

    std::vector<double> opc_latencies_ms;
    opc_latencies_ms.reserve(sample_opc);

    for (std::size_t i = 0; i < sample_opc; ++i) {
        auto t0 = std::chrono::high_resolution_clock::now();
        opcua.write_node("ns=2;s=MotorSpeed", "1500_rpm");
        auto t1 = std::chrono::high_resolution_clock::now();
        opc_latencies_ms.push_back(std::chrono::duration<double, std::milli>(t1 - t0).count());
    }

    std::sort(opc_latencies_ms.begin(), opc_latencies_ms.end());
    double opc_p50 = opc_latencies_ms[static_cast<std::size_t>(opc_latencies_ms.size() * 0.50)];
    std::cout << "  ⏱️ Measured OPC-UA Node Transaction Latency p50: " << opc_p50 << " ms (Target <= 5.0 ms!)\n\n";

    // Benchmark 5: Device Fleet Management Query Throughput
    FleetManager fleet;
    for (int i = 0; i < 500; ++i) {
        fleet.register_device("device_node_" + std::to_string(i), "PLC_Industrial_v1");
    }

    std::cout << "[BENCH 5] Executing Device Fleet Query across 500 Industrial Edge Nodes...\n";
    auto t0_fleet = std::chrono::high_resolution_clock::now();
    auto query_res = fleet.query_devices("device_node");
    auto t1_fleet = std::chrono::high_resolution_clock::now();
    double fleet_ms = std::chrono::duration<double, std::milli>(t1_fleet - t0_fleet).count();

    std::cout << "  ⏱️ Fleet Query for " << query_res.size() << " devices completed in " << fleet_ms << " ms (Target >= 100 dev/sec!)\n\n";

    std::cout << "🎉 ALL INDUSTRIAL PARTNERSHIP & CERTIFICATION BENCHMARKS EXECUTED SUCCESSFULLY!\n";
    return 0;
}
