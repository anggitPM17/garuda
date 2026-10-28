#pragma once
// =====================================================================
// R3 - RICHAT TRACER (OpenTelemetry-like)
// =====================================================================
#include <string>
#include <chrono>
#include <iostream>
#include <mutex>
#include <vector>
#include <cstddef>

namespace garuda::richat {

class RichatTracer {
public:
    struct Span {
        std::string trace_id;
        std::string span_id;
        std::string action;
        std::chrono::steady_clock::time_point start_time;
    };

private:
    std::vector<Span> completed_spans_;
    std::mutex mutex_;

public:
    Span start_span(const std::string& action) {
        Span span;
        span.action = action;
        span.start_time = std::chrono::steady_clock::now();
        span.trace_id = "trace_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
        span.span_id = "span_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
        return span;
    }

    void end_span(const Span& span) {
        std::lock_guard<std::mutex> lock(mutex_);
        completed_spans_.push_back(span);
    }

    // Export format JSON (mirip OTLP)
    std::string export_json() {
        std::lock_guard<std::mutex> lock(mutex_);
        std::string json = "[\n";
        for (size_t i = 0; i < completed_spans_.size(); ++i) {
            json += "  {\n";
            json += "    \"trace_id\": \"" + completed_spans_[i].trace_id + "\",\n";
            json += "    \"span_id\": \"" + completed_spans_[i].span_id + "\",\n";
            json += "    \"action\": \"" + completed_spans_[i].action + "\"\n";
            json += "  }";
            if (i < completed_spans_.size() - 1) json += ",";
            json += "\n";
        }
        json += "]\n";
        return json;
    }
};

} // namespace garuda::richat
