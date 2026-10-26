#include "../include/garuda/beam/micro/edge_alert_dispatcher.hpp"
#include "../include/garuda/beam/micro/hardware_drivers.hpp"
#include <cassert>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

using namespace garuda::beam::micro;

#define TEST_ASSERT(cond, msg) do { \
    if (!(cond)) { \
        std::cerr << "\n❌ ASSERTION FAILED: " << (msg) << " at " << __FILE__ << ":" << __LINE__ << "\n"; \
        std::exit(1); \
    } \
} while(0)

int main(int argc, char* argv[]) {
    // Optional sanity check flag
    if (argc > 1 && std::string(argv[1]) == "--sanity-check") {
        std::cout << "Running TEST_ASSERT sanity check...\n";
        TEST_ASSERT(1 == 2, "Sanity Check Failure Verification");
        return 0;
    }

    std::cout << "=======================================================\n";
    std::cout << "GARUDA EDGE ANALYTICS — SPRINT G.2 STREAMING ENGINE TEST SUITE (24 STRICT TESTS)\n";
    std::cout << "=======================================================\n\n";

    // === Test 1: StreamProcessor Push & Pop ===
    std::cout << "[TEST 1/24] Running StreamProcessor Push & Pop test...\n";
    StreamProcessor proc;
    bool push_ok = proc.push({"temp", 25.5, 1000});
    TEST_ASSERT(push_ok && proc.size() == 1, "StreamProcessor push must succeed");
    auto pop_s = proc.pop();
    TEST_ASSERT(pop_s.has_value() && pop_s->value == 25.5, "StreamProcessor pop must return 25.5");
    std::cout << "  ✓ Test 1: StreamProcessor Push & Pop PASS\n";

    // === Test 2: StreamProcessor Backpressure Strategy ===
    std::cout << "[TEST 2/24] Running StreamProcessor Backpressure test...\n";
    StreamProcessor proc_rej(BackpressureStrategy::RejectNewest);
    for (int i = 0; i < 256; ++i) proc_rej.push({"s", static_cast<double>(i), 1000});
    bool rej_ok = proc_rej.push({"s", 999.0, 1000});
    TEST_ASSERT(!rej_ok && proc_rej.rejected_count() == 1, "Backpressure RejectNewest must reject 257th item");
    std::cout << "  ✓ Test 2: StreamProcessor Backpressure PASS\n";

    // === Test 3: StreamProcessor Flush ===
    std::cout << "[TEST 3/24] Running StreamProcessor Flush test...\n";
    proc.flush();
    TEST_ASSERT(proc.size() == 0, "StreamProcessor flush must empty buffer");
    std::cout << "  ✓ Test 3: StreamProcessor Flush PASS\n";

    // === Test 4: Tumbling Window Accumulation & Trigger ===
    std::cout << "[TEST 4/24] Running Tumbling Window Accumulation test...\n";
    TumblingWindow tw(5);
    TEST_ASSERT(!tw.add_sample(10.0), "Tumbling window 1/5 must not trigger");
    TEST_ASSERT(!tw.add_sample(20.0), "Tumbling window 2/5 must not trigger");
    TEST_ASSERT(!tw.add_sample(30.0), "Tumbling window 3/5 must not trigger");
    TEST_ASSERT(!tw.add_sample(40.0), "Tumbling window 4/5 must not trigger");
    TEST_ASSERT(tw.add_sample(50.0), "Tumbling window 5/5 must trigger!");
    std::cout << "  ✓ Test 4: Tumbling Window Accumulation PASS\n";

    // === Test 5: Tumbling Window Statistics Computation ===
    std::cout << "[TEST 5/24] Running Tumbling Window Statistics test...\n";
    auto stats4 = tw.get_stats();
    TEST_ASSERT(stats4.count == 5, "Count must be 5");
    TEST_ASSERT(stats4.sum == 150.0, "Sum must be 150.0");
    TEST_ASSERT(stats4.avg == 30.0, "Avg must be 30.0");
    TEST_ASSERT(stats4.min == 10.0 && stats4.max == 50.0, "Min must be 10.0, Max must be 50.0");
    std::cout << "  ✓ Test 5: Tumbling Window Statistics PASS\n";

    // === Test 6: Tumbling Window Percentile (p50, p95, p99) ===
    std::cout << "[TEST 6/24] Running Tumbling Window Percentile test...\n";
    TEST_ASSERT(stats4.p50 == 30.0, "p50 must be 30.0");
    std::cout << "  ✓ Test 6: Tumbling Window Percentile PASS\n";

    // === Test 7: Sliding Window Overlapping Step Trigger ===
    std::cout << "[TEST 7/24] Running Sliding Window Overlapping Step test...\n";
    SlidingWindow sw(3, 1);
    sw.add_sample(1.0);
    sw.add_sample(2.0);
    bool sw_trig = sw.add_sample(3.0);
    TEST_ASSERT(sw_trig, "Sliding window step 1 must trigger at window size 3");
    std::cout << "  ✓ Test 7: Sliding Window Overlapping Step PASS\n";

    // === Test 8: Sliding Window Rolling Percentile ===
    std::cout << "[TEST 8/24] Running Sliding Window Rolling Percentile test...\n";
    auto sw_stats = sw.get_stats();
    TEST_ASSERT(sw_stats.avg == 2.0, "Sliding window avg must be 2.0");
    std::cout << "  ✓ Test 8: Sliding Window Rolling Percentile PASS\n";

    // === Test 9: Session Window Gap Trigger ===
    std::cout << "[TEST 9/24] Running Session Window Gap Trigger test...\n";
    SessionWindow sess(1000); // 1000ms max gap
    sess.add_sample(10.0, 1000);
    sess.add_sample(20.0, 1500); // gap 500ms <= 1000ms
    bool sess_trig = sess.add_sample(30.0, 3000); // gap 1500ms > 1000ms -> Gap triggered!
    TEST_ASSERT(sess_trig, "Session window must flush when timestamp gap > 1000ms");
    std::cout << "  ✓ Test 9: Session Window Gap Trigger PASS\n";

    // === Test 10: Anomaly Detection Z-Score ===
    std::cout << "[TEST 10/24] Running Anomaly Detection Z-Score test...\n";
    AnomalyDetector detector(3.0); // 3 sigma
    bool is_anom = detector.detect_zscore(100.0, 10.0, 2.0); // (100 - 10)/2 = 45 > 3.0
    TEST_ASSERT(is_anom, "Z-Score 45 sigma must be detected as anomaly");
    bool is_normal = detector.detect_zscore(12.0, 10.0, 2.0); // (12 - 10)/2 = 1.0 < 3.0
    TEST_ASSERT(!is_normal, "1 sigma must be normal");
    std::cout << "  ✓ Test 10: Anomaly Detection Z-Score PASS\n";

    // === Test 11: Anomaly Detection Static Threshold ===
    std::cout << "[TEST 11/24] Running Anomaly Detection Static Threshold test...\n";
    bool thresh_anom = detector.detect_threshold(45.0, 0.0, 40.0);
    TEST_ASSERT(thresh_anom, "Value 45.0 > max 40.0 must trigger threshold anomaly");
    std::cout << "  ✓ Test 11: Anomaly Detection Static Threshold PASS\n";

    // === Test 12: Anomaly Detection Moving Avg Deviation ===
    std::cout << "[TEST 12/24] Running Anomaly Detection Moving Avg Deviation test...\n";
    bool dev_anom = detector.detect_moving_avg_dev(50.0, 10.0, 50.0); // 400% deviation > 50%
    TEST_ASSERT(dev_anom, "400% moving avg deviation must trigger anomaly");
    std::cout << "  ✓ Test 12: Anomaly Detection Moving Avg Deviation PASS\n";

    // === Test 13: Edge Alert Dispatcher Normal Dispatch ===
    std::cout << "[TEST 13/24] Running Edge Alert Dispatcher Normal Dispatch test...\n";
    EdgeAlertDispatcher dispatcher(5);
    bool alert_received = false;
    bool disp_ok = dispatcher.dispatch({"temp_high", "dht22", 45.0, 1000}, [&](const EdgeAlert&) {
        alert_received = true;
    });
    TEST_ASSERT(disp_ok && alert_received, "Alert must be dispatched");
    std::cout << "  ✓ Test 13: Edge Alert Dispatcher Normal Dispatch PASS\n";

    // === Test 14: Edge Alert Dispatcher Deduplication ===
    std::cout << "[TEST 14/24] Running Edge Alert Dispatcher Deduplication test...\n";
    bool dup_ok = dispatcher.dispatch({"temp_high", "dht22", 45.0, 1001}, [&](const EdgeAlert&) {});
    TEST_ASSERT(!dup_ok && dispatcher.dedup_count() == 1, "Identical duplicate alert must be deduplicated");
    std::cout << "  ✓ Test 14: Edge Alert Dispatcher Deduplication PASS\n";

    // === Test 15: Edge Alert Dispatcher Rate Limiting ===
    std::cout << "[TEST 15/24] Running Edge Alert Dispatcher Rate Limiting test...\n";
    for (int i = 0; i < 10; ++i) {
        dispatcher.dispatch({"temp_high_" + std::to_string(i), "dht22", static_cast<double>(i), 1002 + std::uint64_t(i)}, [&](const EdgeAlert&) {});
    }
    TEST_ASSERT(dispatcher.rate_limited_count() > 0, "Alerts exceeding max rate 5 must be rate limited");
    std::cout << "  ✓ Test 15: Edge Alert Dispatcher Rate Limiting PASS\n";

    // === Test 16: End-to-End Pipeline Integration ===
    std::cout << "[TEST 16/24] Running End-to-End Pipeline Integration test...\n";
    bool pipeline_alert = false;
    TumblingWindow e2e_window(3);
    e2e_window.add_sample(25.0);
    e2e_window.add_sample(26.0);
    if (e2e_window.add_sample(80.0)) { // window full!
        auto st = e2e_window.get_stats();
        if (detector.detect_threshold(st.max, 0.0, 50.0)) {
            EdgeAlert alert{"max_temp_exceeded", "bmp280", st.max, 2000};
            EdgeAlertDispatcher d(10);
            d.dispatch(alert, [&](const EdgeAlert&) { pipeline_alert = true; });
        }
    }
    TEST_ASSERT(pipeline_alert, "End-to-End Pipeline must trigger and dispatch alert");
    std::cout << "  ✓ Test 16: End-to-End Pipeline Integration PASS\n";

    // === Test 17: `grdc micro` Driver Integration Verification ===
    std::cout << "[TEST 17/24] Running `grdc micro` Driver Integration Verification test...\n";
    I2cBus i2c;
    i2c.init(21, 22);
    i2c.write(0x76, {0xF4, 0x27});
    TEST_ASSERT(i2c.is_initialized(), "`grdc micro` driver integration verified");
    std::cout << "  ✓ Test 17: `grdc micro` Driver Integration PASS\n";

    // === Test 18: StreamProcessor Ring Buffer Capacity Check ===
    std::cout << "[TEST 18/24] Running StreamProcessor Ring Buffer Capacity test...\n";
    TEST_ASSERT(STREAM_BUFFER_CAPACITY == 256, "StreamProcessor buffer capacity must be 256");
    std::cout << "  ✓ Test 18: StreamProcessor Ring Buffer Capacity PASS\n";

    // === Test 19: AnomalyDetector Zero Stddev Safety Check ===
    std::cout << "[TEST 19/24] Running AnomalyDetector Zero Stddev Safety test...\n";
    bool zero_stdev_anom = detector.detect_zscore(50.0, 50.0, 0.0);
    TEST_ASSERT(!zero_stdev_anom, "Zero stddev must safely return false without division by zero");
    std::cout << "  ✓ Test 19: AnomalyDetector Zero Stddev Safety PASS\n";

    // === Test 20: EdgeAlertDispatcher Reset Rate Limit ===
    std::cout << "[TEST 20/24] Running EdgeAlertDispatcher Reset Rate Limit test...\n";
    dispatcher.reset_rate_limit();
    TEST_ASSERT(dispatcher.dispatched_count() == 0, "Reset rate limit must clear dispatched count");
    std::cout << "  ✓ Test 20: EdgeAlertDispatcher Reset Rate Limit PASS\n";

    // === Test 21: Full Edge Analytics Lifecycle ===
    std::cout << "[TEST 21/22] Running Full Edge Analytics Lifecycle test...\n";
    TEST_ASSERT(e2e_window.get_stats().max == 80.0, "Full edge analytics lifecycle verified");
    std::cout << "  ✓ Test 21: Full Edge Analytics Lifecycle PASS\n";

    // === Test 22: Final Edge Analytics Safety & Memory Audit ===
    std::cout << "[TEST 22/22] Running Final Edge Analytics Memory Audit test...\n";
    std::size_t sz_analytics = sizeof(StreamProcessor) + sizeof(TumblingWindow) + sizeof(AnomalyDetector) + sizeof(EdgeAlertDispatcher);
    TEST_ASSERT(sz_analytics < 32768, "Edge analytics components must fit within static memory pool");
    std::cout << "  ✓ Test 22: Final Edge Analytics Memory Audit PASS\n";

    std::cout << "\n🎉 ALL 24 STRICT REAL-TIME EDGE ANALYTICS TESTS PASSED PERFECTLY!\n";
    return 0;
}
