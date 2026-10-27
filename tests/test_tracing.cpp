#include "../include/garuda/beam/sys/tracing.hpp"
#include "../include/garuda/beam/stdlib/json.hpp"
#include <chrono>
#include <future>
#include <iostream>

using namespace garuda::beam;
using namespace garuda::beam::stdlib;

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
    std::cout << "GARUDA OPENTELEMETRY DISTRIBUTED TRACING SUITE\n";
    std::cout << "=======================================================\n\n";

    // === Test 1: OpenTelemetry OTLP JSON Spans & Parent/Child Propagation ===
    run_test_with_timeout("Test 1: OTLP Spans & Parent-Child Propagation", []() {
        auto& tracer = Tracer::instance();
        tracer.clear();

        // 1. Create Root Span
        Span root = tracer.start_span("handle_request");
        root.set_attribute("http.method", "GET");
        root.set_attribute("http.target", "/api/v1/user");

        // 2. Create Child Span
        Span child = tracer.start_child_span(root, "process_message");
        child.set_attribute("actor.pid", "101");

        // 3. End Spans
        tracer.end_span(child);
        tracer.end_span(root);

        GARUDA_CHECK(tracer.span_count() == 2, "Tracer must store 2 completed spans");

        // 4. Render & Parse OTLP JSON
        std::string json_str = tracer.render_otlp_json();
        GARUDA_CHECK(!json_str.empty(), "Rendered OTLP JSON must not be empty");

        auto decoded_opt = Json::decode(json_str);
        GARUDA_CHECK(decoded_opt.has_value(), "Json::decode must successfully parse OTLP JSON");

        // Verify JSON string attributes directly
        GARUDA_CHECK(json_str.find(root.trace_id) != std::string::npos, "OTLP JSON must contain root trace_id");
        GARUDA_CHECK(json_str.find(root.span_id) != std::string::npos, "OTLP JSON must contain root span_id");
        GARUDA_CHECK(json_str.find(child.span_id) != std::string::npos, "OTLP JSON must contain child span_id");
        GARUDA_CHECK(json_str.find("\"parentSpanId\":\"" + root.span_id + "\"") != std::string::npos, "Child span must reference parentSpanId matching root span_id");
        GARUDA_CHECK(child.trace_id == root.trace_id, "Child span trace_id must equal root trace_id");

        std::cout << "  ✓ Test 1: OTLP Spans & Parent-Child Propagation PASS (Trace ID: " << root.trace_id << ")\n";
    }, 5);

    std::cout << "\n🎉 ALL OPENTELEMETRY TRACING TESTS PASSED PERFECTLY!\n";
    return 0;
}
