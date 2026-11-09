#pragma once
// Garuda Production Infrastructure: OpenTelemetry Tracing & Prometheus Metrics Export (`opentelemetry_tracing.hpp`)
// Mengimplementasikan OpenTelemetry Span ID, Trace ID propagation, OpenTelemetry JSON Exporter, & Prometheus Metrics Exporter.

#include "../core/term.hpp"
#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

namespace garuda::beam {

struct Span {
    std::string trace_id;
    std::string span_id;
    std::string parent_span_id;
    std::string name;
    uint64_t start_time_us = 0;
    uint64_t end_time_us = 0;
    std::map<std::string, std::string> attributes;

    uint64_t duration_us() const { return end_time_us >= start_time_us ? end_time_us - start_time_us : 0; }

    std::string to_json() const {
        std::ostringstream ss;
        ss << "{\"trace_id\":\"" << trace_id << "\",\"span_id\":\"" << span_id << "\",\"parent_span_id\":\"" << parent_span_id
           << "\",\"name\":\"" << name << "\",\"duration_us\":" << duration_us() << "}";
        return ss.str();
    }
};

class OpenTelemetryTracer {
public:
    static OpenTelemetryTracer& instance() {
        static OpenTelemetryTracer inst;
        return inst;
    }

    Span start_span(const std::string& name, const std::string& parent_trace_id = "", const std::string& parent_span_id = "") {
        std::lock_guard<std::mutex> lock(mu_);
        Span s;
        s.trace_id = parent_trace_id.empty() ? generate_id("trace_") : parent_trace_id;
        s.span_id = generate_id("span_");
        s.parent_span_id = parent_span_id;
        s.name = name;
        s.start_time_us = now_us();
        return s;
    }

    void end_span(Span& span) {
        std::lock_guard<std::mutex> lock(mu_);
        span.end_time_us = now_us();
        completed_spans_.push_back(span);
    }

    std::string export_spans_json() const {
        std::lock_guard<std::mutex> lock(mu_);
        std::ostringstream ss;
        ss << "[\n";
        for (size_t i = 0; i < completed_spans_.size(); ++i) {
            ss << "  " << completed_spans_[i].to_json();
            if (i + 1 < completed_spans_.size()) ss << ",";
            ss << "\n";
        }
        ss << "]";
        return ss.str();
    }

    size_t span_count() const {
        std::lock_guard<std::mutex> lock(mu_);
        return completed_spans_.size();
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mu_);
        completed_spans_.clear();
    }

private:
    OpenTelemetryTracer() = default;

    uint64_t now_us() const {
        return std::chrono::duration_cast<std::chrono::microseconds>(
                   std::chrono::system_clock::now().time_since_epoch())
            .count();
    }

    std::string generate_id(const std::string& prefix) {
        static uint64_t counter = 1000;
        return prefix + std::to_string(++counter);
    }

    mutable std::mutex mu_;
    std::vector<Span> completed_spans_;
};

class PrometheusMetricsExporter {
public:
    PrometheusMetricsExporter() = default;

    void counter_inc(const std::string& metric, double val = 1.0) {
        std::lock_guard<std::mutex> lock(mu_);
        counters_[metric] += val;
    }

    void gauge_set(const std::string& metric, double val) {
        std::lock_guard<std::mutex> lock(mu_);
        gauges_[metric] = val;
    }

    std::string export_prometheus() const {
        std::lock_guard<std::mutex> lock(mu_);
        std::ostringstream ss;
        ss << "# HELP garuda_runtime_metrics Prometheus Exporter for Garuda BEAM\n";
        for (const auto& [m, v] : counters_) {
            ss << m << "_total " << v << "\n";
        }
        for (const auto& [m, v] : gauges_) {
            ss << m << " " << v << "\n";
        }
        return ss.str();
    }

private:
    mutable std::mutex mu_;
    std::map<std::string, double> counters_;
    std::map<std::string, double> gauges_;
};

} // namespace garuda::beam
