// =====================================================================
// GARUDA COMPILER — FASE 17: PROMETHEUS METRICS EXPORT SUITE
// =====================================================================

#include "../include/garuda/beam/sys/opentelemetry_tracing.hpp"
#include <iostream>
#include <cassert>

using namespace garuda::beam;

int main() {
    int failures = 0;

    std::cout << "=======================================================\n";
    std::cout << "GARUDA COMPILER — FASE 17: PROMETHEUS METRICS EXPORT SUITE\n";
    std::cout << "=======================================================\n\n";

    PrometheusMetricsExporter exporter;

    // Test 1: Record Counters & Gauges
    exporter.counter_inc("gvm_requests_total", 100);
    exporter.gauge_set("gvm_active_actors", 42);

    std::string prom_output = exporter.export_prometheus();

    if (prom_output.find("gvm_requests_total_total 100") != std::string::npos) {
        std::cout << "[Test 1] Record Counter Metric -> gvm_requests_total_total 100 [PASS]\n";
    } else {
        std::cout << "[Test 1] Record Counter Metric -> [FAIL]\n";
        failures++;
    }

    if (prom_output.find("gvm_active_actors 42") != std::string::npos) {
        std::cout << "[Test 2] Record Gauge Metric -> gvm_active_actors 42 [PASS]\n";
    } else {
        std::cout << "[Test 2] Record Gauge Metric -> [FAIL]\n";
        failures++;
    }

    if (failures > 0) {
        std::cout << "\n❌ " << failures << " PROMETHEUS METRICS EXPORT TESTS FAILED!\n";
        return 1;
    }

    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL FASE 17 PROMETHEUS METRICS EXPORT TESTS PASSED PERFECTLY!\n";
    std::cout << "========================================\n";
    return 0;
}
