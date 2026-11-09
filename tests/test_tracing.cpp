// =====================================================================
// GARUDA COMPILER — FASE 17: OPENTELEMETRY TRACING SUITE
// =====================================================================

#include "../include/garuda/beam/sys/opentelemetry_tracing.hpp"
#include <iostream>
#include <cassert>

using namespace garuda::beam;

int main() {
    int failures = 0;

    std::cout << "=======================================================\n";
    std::cout << "GARUDA COMPILER — FASE 17: OPENTELEMETRY TRACING SUITE\n";
    std::cout << "=======================================================\n\n";

    auto& tracer = OpenTelemetryTracer::instance();
    tracer.clear();

    // Test 1: Start & End Span
    Span s1 = tracer.start_span("gvm_actor_spawn");
    s1.attributes["actor_type"] = "GenServer";
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    tracer.end_span(s1);

    if (tracer.span_count() == 1) {
        std::cout << "[Test 1] Start & End Span -> Recorded 1 Span [PASS]\n";
    } else {
        std::cout << "[Test 1] Start & End Span -> [FAIL]\n";
        failures++;
    }

    // Test 2: Trace ID Propagation for Distributed Spans
    Span s2 = tracer.start_span("remote_rpc_call", s1.trace_id, s1.span_id);
    tracer.end_span(s2);

    if (s2.trace_id == s1.trace_id && s2.parent_span_id == s1.span_id) {
        std::cout << "[Test 2] Trace ID & Parent Span ID Propagation -> Propagated [PASS]\n";
    } else {
        std::cout << "[Test 2] Trace ID Propagation -> [FAIL]\n";
        failures++;
    }

    // Test 3: Export OpenTelemetry Spans JSON
    std::string otel_json = tracer.export_spans_json();
    if (otel_json.find("trace_id") != std::string::npos && otel_json.find("gvm_actor_spawn") != std::string::npos) {
        std::cout << "[Test 3] Export OpenTelemetry JSON Spans -> [PASS]\n";
    } else {
        std::cout << "[Test 3] Export OpenTelemetry JSON -> [FAIL]\n";
        failures++;
    }

    if (failures > 0) {
        std::cout << "\n❌ " << failures << " OPENTELEMETRY TRACING TESTS FAILED!\n";
        return 1;
    }

    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL FASE 17 OPENTELEMETRY TRACING TESTS PASSED PERFECTLY!\n";
    std::cout << "========================================\n";
    return 0;
}
