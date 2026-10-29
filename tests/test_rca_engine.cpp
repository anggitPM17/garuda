// =====================================================================
// FASE D.1.2 - GARUDA RCA ENGINE TESTS
// Memverifikasi rca_engine.hpp
// =====================================================================
#include "../include/garuda/autopilot/rca_engine.hpp"
#include <iostream>
#include <string>
#include <cstdlib>

using namespace garuda::autopilot;

#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

int main() {
    std::cout << "========================================\n";
    std::cout << "FASE D.1.2 - GARUDA RCA ENGINE TESTS\n";
    std::cout << "========================================\n\n";

    RcaEngine rca;

    // Test 1: Ingest Logs
    std::cout << "[Test 1] Ingest Logs...\n";
    {
        for (int i = 0; i < 50; ++i) {
            rca.ingest_log("[INFO] System normal activity log " + std::to_string(i));
        }
        CHECK(rca.log_count() == 50, "Ingest 50 log harus tercatat 50");
    }
    std::cout << "PASS\n\n";

    // Test 2: Analyze High CPU
    std::cout << "[Test 2] Analyze High CPU...\n";
    {
        rca.ingest_log("[WARNING] High CPU spike observed on worker thread 3");
        Anomaly cpu_anom;
        cpu_anom.id = "anom_cpu_1";
        cpu_anom.type = AnomalyType::HIGH_CPU;

        auto rc = rca.analyze(cpu_anom);
        CHECK(rc.confidence >= 0.80, "Confidence untuk CPU spike terdeteksi harus >= 0.80");
        CHECK(rc.recommended_action == "alert", "Recommended action harus alert");
        CHECK(!rc.evidence.empty(), "Evidence log harus tidak kosong");
    }
    std::cout << "PASS\n\n";

    // Test 3: Analyze Repeated Crash
    std::cout << "[Test 3] Analyze Repeated Crash...\n";
    {
        rca.ingest_log("[CRITICAL] Process segfault encountered in worker Module A");
        Anomaly crash_anom;
        crash_anom.id = "anom_crash_1";
        crash_anom.type = AnomalyType::REPEATED_CRASH;

        auto rc = rca.analyze(crash_anom);
        CHECK(rc.confidence >= 0.90, "Confidence untuk crash segfault harus >= 0.90");
        CHECK(rc.recommended_action == "rollback", "Recommended action untuk crash harus rollback");
    }
    std::cout << "PASS\n\n";

    // Test 4: Analyze Unknown Anomaly
    std::cout << "[Test 4] Analyze Unknown Anomaly...\n";
    {
        Anomaly unknown_anom;
        unknown_anom.id = "anom_net_1";
        unknown_anom.type = AnomalyType::NETWORK_ERROR;

        auto rc = rca.analyze(unknown_anom);
        CHECK(rc.confidence < 0.50, "Confidence tanpa log cocok harus < 0.50");
    }
    std::cout << "PASS\n\n";

    // Test 5: Clear Logs
    std::cout << "[Test 5] Clear Logs...\n";
    {
        rca.clear_logs();
        CHECK(rca.log_count() == 0, "clear_logs() harus mengosongkan recent_logs_");
    }
    std::cout << "PASS\n\n";

    std::cout << "========================================\n";
    std::cout << "FASE D.1.2 - RCA ENGINE SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
