#include "../include/garuda/autopilot/health_monitor.hpp"
#include <iostream>
#include <cstdlib>

using namespace garuda::autopilot;

#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

int main() {
    std::cout << "========================================\n";
    std::cout << "FASE 5.1 - HEALTH MONITOR TESTS\n";
    std::cout << "========================================\n\n";

    // Test 1: Sistem sehat
    std::cout << "[Test 1] Sistem sehat...\n";
    {
        HealthMonitor mon([]() {
            HealthSnapshot s;
            s.db_ok = true;
            s.ram_usage_mb = 512.0;
            s.latency_ms = 20.0;
            return s;
        });
        CHECK(mon.is_healthy(), "Sistem harus sehat");
        auto snap = mon.sample();
        CHECK(snap.is_healthy(), "Snapshot sehat");
    }
    std::cout << "PASS\n\n";

    // Test 2: RAM melebihi threshold -> tidak sehat
    std::cout << "[Test 2] RAM over threshold...\n";
    {
        HealthMonitor mon([]() {
            HealthSnapshot s;
            s.db_ok = true;
            s.ram_usage_mb = 4096.0; // > 2048
            s.latency_ms = 20.0;
            return s;
        });
        CHECK(!mon.is_healthy(), "RAM > max harus tidak sehat");
        CHECK(!mon.sample().is_healthy(), "Snapshot tidak sehat");
    }
    std::cout << "PASS\n\n";

    // Test 3: Latensi melebihi threshold -> tidak sehat
    std::cout << "[Test 3] Latency over threshold...\n";
    {
        HealthMonitor mon([]() {
            HealthSnapshot s;
            s.db_ok = true;
            s.ram_usage_mb = 512.0;
            s.latency_ms = 999.0; // > 500
            return s;
        });
        CHECK(!mon.is_healthy(), "Latency > max harus tidak sehat");
    }
    std::cout << "PASS\n\n";

    // Test 4: db_ok = false -> tidak sehat (meski RAM/latency OK)
    std::cout << "[Test 4] DB down...\n";
    {
        HealthMonitor mon([]() {
            HealthSnapshot s;
            s.db_ok = false;
            s.ram_usage_mb = 128.0;
            s.latency_ms = 5.0;
            return s;
        });
        CHECK(!mon.is_healthy(), "DB down harus tidak sehat");
    }
    std::cout << "PASS\n\n";

    // Test 5 (bonus): source throw -> fail-safe jadi tidak sehat
    std::cout << "[Test 5] Source throw -> fail-safe...\n";
    {
        HealthMonitor mon([]() -> HealthSnapshot {
            throw std::runtime_error("sensor error");
        });
        CHECK(!mon.is_healthy(), "Source throw harus dianggap tidak sehat");
    }
    std::cout << "PASS\n\n";

    std::cout << "========================================\n";
    std::cout << "HEALTH MONITOR SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
