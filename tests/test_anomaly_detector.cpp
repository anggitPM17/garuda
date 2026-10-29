// =====================================================================
// FASE D.1.1 - GARUDA ANOMALY DETECTOR TESTS
// Memverifikasi anomaly_detector.hpp
// =====================================================================
#include "../include/garuda/autopilot/anomaly_detector.hpp"
#include <iostream>
#include <string>
#include <cstdlib>

using namespace garuda::autopilot;

#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

int main() {
    std::cout << "========================================\n";
    std::cout << "FASE D.1.1 - GARUDA ANOMALY DETECTOR TESTS\n";
    std::cout << "========================================\n\n";

    AnomalyDetector detector;

    // Test 1: Register Default Rules
    std::cout << "[Test 1] Register Default Rules...\n";
    {
        CHECK(detector.rule_count() >= 6, "Default rules count harus >= 6");
        CHECK(detector.anomaly_count() == 0, "Awalnya anomaly count harus 0");
    }
    std::cout << "PASS\n\n";

    // Test 2: No Anomaly Normal Metric
    std::cout << "[Test 2] No Anomaly Normal Metric...\n";
    {
        detector.ingest_metric("cpu_percent", 45.0);
        detector.ingest_metric("cpu_percent", 50.0);
        CHECK(detector.anomaly_count() == 0, "CPU normal tidak menrigger anomali");
    }
    std::cout << "PASS\n\n";

    // Test 3: High CPU Anomaly Trigger
    std::cout << "[Test 3] High CPU Anomaly Trigger...\n";
    {
        detector.ingest_metric("cpu_percent", 85.0);
        detector.ingest_metric("cpu_percent", 88.0);
        detector.ingest_metric("cpu_percent", 92.0); // 3x breach

        CHECK(detector.anomaly_count() == 1, "3x CPU breach harus menrigger 1 anomali");
        auto anomalies = detector.active_anomalies();
        CHECK(anomalies[0].type == AnomalyType::HIGH_CPU, "Tipe anomali harus HIGH_CPU");
        CHECK(anomalies[0].severity == AnomalySeverity::HIGH, "Severity harus HIGH");
    }
    std::cout << "PASS\n\n";

    // Test 4: High Error Rate Anomaly Trigger
    std::cout << "[Test 4] High Error Rate Anomaly Trigger...\n";
    {
        for (int i = 0; i < 5; ++i) {
            detector.ingest_metric("error_rate", 12.0);
        }
        CHECK(detector.anomaly_count() == 2, "High error rate 5x breach harus menrigger anomali ke-2");
    }
    std::cout << "PASS\n\n";

    // Test 5: Clear Anomaly
    std::cout << "[Test 5] Clear Anomaly...\n";
    {
        auto list = detector.active_anomalies();
        std::string first_id = list[0].id;
        detector.clear_anomaly(first_id);
        CHECK(detector.anomaly_count() == 1, "Anomali berkurang jadi 1 setelah clear_anomaly");
    }
    std::cout << "PASS\n\n";

    std::cout << "========================================\n";
    std::cout << "FASE D.1.1 - ANOMALY DETECTOR SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
