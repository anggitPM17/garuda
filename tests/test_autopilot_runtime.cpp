// =====================================================================
// FASE D.1.4 - GARUDA AUTOPILOT RUNTIME TESTS
// Memverifikasi autopilot_runtime.hpp
// =====================================================================
#include "../include/garuda/autopilot/autopilot_runtime.hpp"
#include <iostream>
#include <string>
#include <cstdlib>

using namespace garuda::autopilot;

#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

int main() {
    std::cout << "========================================\n";
    std::cout << "FASE D.1.4 - GARUDA AUTOPILOT RUNTIME TESTS\n";
    std::cout << "========================================\n\n";

    AutopilotRuntime runtime;

    // Test 1: Init Runtime
    std::cout << "[Test 1] Init Runtime...\n";
    {
        CHECK(runtime.anomaly_count() == 0, "Awalnya anomaly_count = 0");
        CHECK(runtime.pending_action_count() == 0, "Awalnya pending_action_count = 0");
        CHECK(runtime.executed_action_count() == 0, "Awalnya executed_action_count = 0");
    }
    std::cout << "PASS\n\n";

    // Test 2: Tick with Anomaly
    std::cout << "[Test 2] Tick with Anomaly...\n";
    {
        runtime.detector().ingest_metric("cpu_percent", 85.0);
        runtime.detector().ingest_metric("cpu_percent", 88.0);
        runtime.detector().ingest_metric("cpu_percent", 92.0); // Anomaly triggered

        CHECK(runtime.anomaly_count() == 1, "Anomaly_count = 1");

        runtime.tick();
        auto logs = runtime.event_log();
        CHECK(!logs.empty(), "Event log harus terisi setelah tick()");
        CHECK(logs[0].event_type == "anomaly_detected", "Event 1 type = anomaly_detected");
    }
    std::cout << "PASS\n\n";

    // Test 3: Tick with RCA & Proposed Action
    std::cout << "[Test 3] Tick with RCA & Proposed Action...\n";
    {
        runtime.rca().ingest_log("[WARNING] High CPU spike on worker 1");
        runtime.tick();

        auto logs = runtime.event_log();
        bool found_rca = false;
        for (const auto& ev : logs) {
            if (ev.event_type == "rca_completed") found_rca = true;
        }
        CHECK(found_rca, "Event log harus berisi rca_completed");
        CHECK(runtime.pending_action_count() >= 1, "pending_action_count harus >= 1");
    }
    std::cout << "PASS\n\n";

    // Test 4: Autopilot Mode Auto-Executes Safe Action
    std::cout << "[Test 4] Autopilot Mode Auto-Executes Safe Action...\n";
    {
        runtime.set_mode(ActionMode::AUTOPILOT);

        // Ingest DISK_FULL anomaly (recommended action = alert)
        runtime.detector().ingest_metric("disk_percent", 95.0);
        runtime.tick();

        auto logs = runtime.event_log();
        bool found_auto = false;
        for (const auto& ev : logs) {
            if (ev.event_type == "action_executed") found_auto = true;
        }
        CHECK(found_auto, "Event log harus berisi action_executed untuk alert di AUTOPILOT mode");
    }
    std::cout << "PASS\n\n";

    // Test 5: Render Status Dashboard
    std::cout << "[Test 5] Render Status Dashboard...\n";
    {
        std::string status = runtime.render_status();
        CHECK(!status.empty(), "render_status() tidak boleh kosong");
        CHECK(status.find("AUTOPILOT AI STATUS DASHBOARD") != std::string::npos, "Dashboard title harus ada");
        std::cout << status << "\n";
    }
    std::cout << "PASS\n\n";

    std::cout << "========================================\n";
    std::cout << "FASE D.1.4 - AUTOPILOT RUNTIME SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
