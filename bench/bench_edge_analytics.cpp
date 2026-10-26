#include "../include/garuda/beam/micro/edge_alert_dispatcher.hpp"
#include "../include/garuda/beam/micro/hardware_drivers.hpp"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <iostream>
#include <vector>

using namespace garuda::beam::micro;

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA REAL-TIME EDGE ANALYTICS BENCHMARK (SPRINT G.2 — STREAMING PERFORMANCE)\n";
    std::cout << "=======================================================\n\n";

    // Benchmark 1: Tumbling Window Aggregation Latency (Target <= 5.0 µs)
    TumblingWindow window(10);
    std::size_t sample_window = 1000;
    std::cout << "[BENCH 1] Executing " << sample_window << " Window Aggregations...\n";

    std::vector<double> window_latencies_us;
    window_latencies_us.reserve(sample_window);

    for (std::size_t i = 0; i < sample_window; ++i) {
        auto t0 = std::chrono::high_resolution_clock::now();
        window.add_sample(static_cast<double>(i));
        auto t1 = std::chrono::high_resolution_clock::now();
        window_latencies_us.push_back(std::chrono::duration<double, std::micro>(t1 - t0).count());
    }

    std::sort(window_latencies_us.begin(), window_latencies_us.end());
    double window_p50 = window_latencies_us[static_cast<std::size_t>(window_latencies_us.size() * 0.50)];
    std::cout << "  ⏱️ Measured Window Aggregation Latency p50: " << window_p50 << " µs (Target <= 5.0 µs!)\n\n";

    // Benchmark 2: Anomaly Detection Z-Score Latency (Target <= 1.0 µs)
    AnomalyDetector detector(3.0);
    std::size_t sample_anom = 1000;
    std::cout << "[BENCH 2] Executing " << sample_anom << " Z-Score Anomaly Detections...\n";

    std::vector<double> anom_latencies_us;
    anom_latencies_us.reserve(sample_anom);

    for (std::size_t i = 0; i < sample_anom; ++i) {
        auto t0 = std::chrono::high_resolution_clock::now();
        bool is_anom = detector.detect_zscore(45.0, 10.0, 2.0);
        (void)is_anom;
        auto t1 = std::chrono::high_resolution_clock::now();
        anom_latencies_us.push_back(std::chrono::duration<double, std::micro>(t1 - t0).count());
    }

    std::sort(anom_latencies_us.begin(), anom_latencies_us.end());
    double anom_p50 = anom_latencies_us[static_cast<std::size_t>(anom_latencies_us.size() * 0.50)];
    std::cout << "  ⏱️ Measured Anomaly Detection Z-Score Latency p50: " << anom_p50 << " µs (Target <= 1.0 µs!)\n\n";

    // Benchmark 3: Edge Alert Dispatcher Latency (Target <= 10.0 µs)
    EdgeAlertDispatcher dispatcher(1000);
    std::size_t sample_disp = 1000;
    std::cout << "[BENCH 3] Executing " << sample_disp << " Edge Alert Dispatches...\n";

    std::vector<double> disp_latencies_us;
    disp_latencies_us.reserve(sample_disp);

    for (std::size_t i = 0; i < sample_disp; ++i) {
        auto t0 = std::chrono::high_resolution_clock::now();
        dispatcher.dispatch({"temp_high", "dht22", static_cast<double>(i), 1000}, [](const EdgeAlert&) {});
        auto t1 = std::chrono::high_resolution_clock::now();
        disp_latencies_us.push_back(std::chrono::duration<double, std::micro>(t1 - t0).count());
    }

    std::sort(disp_latencies_us.begin(), disp_latencies_us.end());
    double disp_p50 = disp_latencies_us[static_cast<std::size_t>(disp_latencies_us.size() * 0.50)];
    std::cout << "  ⏱️ Measured Alert Dispatch Latency p50: " << disp_p50 << " µs (Target <= 10.0 µs!)\n\n";

    std::cout << "🎉 ALL REAL-TIME EDGE ANALYTICS BENCHMARKS EXECUTED SUCCESSFULLY!\n";
    return 0;
}
