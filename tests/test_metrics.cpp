#include "../include/garuda/beam/sys/metrics.hpp"
#include <chrono>
#include <future>
#include <iostream>

using namespace garuda::beam;

#define GARUDA_CHECK(cond, msg) \
    if (!(cond)) { \
        std::cerr << "  ❌ ASSERTION FAILED: " << msg << " (" << #cond << ")\n"; \
        std::exit(1); \
    }

void run_test_with_timeout(const std::string& test_name, std::function<void()> test_fn, int timeout_seconds = 5) {
    auto future = std::async(std::launch::async, test_fn);
    if (future.wait_for(std::chrono::seconds(timeout_seconds)) == std::future_status::timeout) {
        std::cerr << "  ❌ " << test_name << " FAILED (TIMEOUT > " << timeout_seconds << "s)\n";
        std::exit(1);
    }
}

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA PROMETHEUS METRICS REGISTRY SUITE\n";
    std::cout << "=======================================================\n\n";

    // === Test 1: Prometheus Metrics Counter, Gauge & Histogram Rendering ===
    run_test_with_timeout("Test 1: Prometheus Metrics Rendering", []() {
        auto& reg = MetricsRegistry::instance();
        reg.clear();

        // 1. Counter
        reg.increment_counter("garuda_http_requests_total", 1.0, {{"method", "GET"}});
        reg.increment_counter("garuda_http_requests_total", 1.0, {{"method", "GET"}});

        // 2. Gauge
        reg.set_gauge("garuda_active_actors", 42.0);

        // 3. Histogram
        reg.observe_histogram("garuda_request_duration_seconds", 0.005);
        reg.observe_histogram("garuda_request_duration_seconds", 0.015);

        // Render Output
        std::string prom_out = reg.render_prometheus();
        GARUDA_CHECK(!prom_out.empty(), "Prometheus output must not be empty");

        // Verify Counter
        GARUDA_CHECK(prom_out.find("# TYPE garuda_http_requests_total counter") != std::string::npos, "Must contain counter type header");
        GARUDA_CHECK(prom_out.find("garuda_http_requests_total{method=\"GET\"} 2") != std::string::npos, "Counter value must be 2");

        // Verify Gauge
        GARUDA_CHECK(prom_out.find("# TYPE garuda_active_actors gauge") != std::string::npos, "Must contain gauge type header");
        GARUDA_CHECK(prom_out.find("garuda_active_actors 42") != std::string::npos, "Gauge value must be 42");

        // Verify Histogram
        GARUDA_CHECK(prom_out.find("# TYPE garuda_request_duration_seconds histogram") != std::string::npos, "Must contain histogram type header");
        GARUDA_CHECK(prom_out.find("garuda_request_duration_seconds_count 2") != std::string::npos, "Histogram count must be 2");

        std::cout << "  ✓ Test 1: Prometheus Metrics Rendering PASS\n";
    }, 5);

    std::cout << "\n🎉 ALL PROMETHEUS METRICS REGISTRY TESTS PASSED PERFECTLY!\n";
    return 0;
}
