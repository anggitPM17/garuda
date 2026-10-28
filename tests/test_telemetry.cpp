// =====================================================================
// FASE G7.3 - GARUDA TELEMETRY TESTS
// Memverifikasi telemetry.hpp (Metrics + Tracing + Logging Aggregator)
// =====================================================================
#include "../include/garuda/beam/telemetry/telemetry.hpp"
#include <iostream>
#include <string>
#include <cstdlib>

using namespace garuda::beam;

#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

int main() {
    std::cout << "========================================\n";
    std::cout << "FASE G7.3 - GARUDA TELEMETRY TESTS\n";
    std::cout << "========================================\n\n";

    TelemetryConfig cfg;
    cfg.service_name = "richat_backend";
    Telemetry t(cfg);

    // Test 1: Record Message Counter
    std::cout << "[Test 1] Record Message Counter...\n";
    {
        for (int i = 0; i < 5; ++i) {
            t.record_message("richat_gateway", "ws_chat");
        }
        CHECK(t.message_count() == 5, "Total message count harus 5");
        CHECK(t.get_counter("messages_total") == 5, "messages_total counter = 5");
        CHECK(t.get_counter("messages_richat_gateway") == 5, "messages_richat_gateway counter = 5");
    }
    std::cout << "PASS\n\n";

    // Test 2: Multiple Modules Counter Tracking
    std::cout << "[Test 2] Multiple Modules Counter Tracking...\n";
    {
        t.record_message("jalur_langit", "sql_query");
        t.record_message("jalur_langit", "sql_query");
        t.record_message("nusantara_broker", "pubsub");

        CHECK(t.get_counter("messages_jalur_langit") == 2, "messages_jalur_langit counter = 2");
        CHECK(t.get_counter("messages_nusantara_broker") == 1, "messages_nusantara_broker counter = 1");
        CHECK(t.message_count() == 8, "Total message count = 8");
    }
    std::cout << "PASS\n\n";

    // Test 3: System Info (erlang:system_info/1 setara)
    std::cout << "[Test 3] System Info...\n";
    {
        auto info = t.system_info();
        CHECK(info.version == "Garuda BEAM v67.0.0", "Version string = Garuda BEAM v67.0.0");
        CHECK(info.total_messages == 8, "Total messages in system_info = 8");
        CHECK(info.process_count >= 1, "Process count >= 1");
    }
    std::cout << "PASS\n\n";

    // Test 4: Record Latency
    std::cout << "[Test 4] Record Latency...\n";
    {
        t.record_latency("richat_gateway", std::chrono::milliseconds(15));
        t.record_latency("richat_gateway", std::chrono::milliseconds(25));

        CHECK(t.get_counter("latency_richat_gateway_ms_count") == 2, "Latency records count = 2");
    }
    std::cout << "PASS\n\n";

    // Test 5: Health Check & Uptime
    std::cout << "[Test 5] Health Check & Uptime...\n";
    {
        CHECK(t.health_check() == true, "Health check harus return true");
        CHECK(t.uptime().count() >= 0, "Uptime seconds harus >= 0");
        CHECK(t.config().service_name == "richat_backend", "Service name = richat_backend");
    }
    std::cout << "PASS\n\n";

    std::cout << "========================================\n";
    std::cout << "FASE G7.3 - TELEMETRY SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
