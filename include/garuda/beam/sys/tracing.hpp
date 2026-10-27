#pragma once
// Garuda Production Infrastructure: OpenTelemetry Distributed Tracing (`tracing.hpp`)
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <map>
#include <mutex>
#include <optional>
#include <random>
#include <sstream>
#include <string>
#include <vector>

namespace garuda::beam {

struct Span {
    std::string trace_id;        // 32 hex chars
    std::string span_id;         // 16 hex chars
    std::string parent_span_id;  // 16 hex chars (optional / empty if root)
    std::string name;
    std::uint64_t start_time_ns = 0;
    std::uint64_t end_time_ns = 0;
    std::map<std::string, std::string> attributes;

    void set_attribute(const std::string& key, const std::string& val) {
        attributes[key] = val;
    }
};

class Tracer {
public:
    static Tracer& instance() {
        static Tracer inst;
        return inst;
    }

    Span start_span(const std::string& name) {
        Span span;
        span.trace_id = generate_hex(32);
        span.span_id = generate_hex(16);
        span.parent_span_id = "";
        span.name = name;
        span.start_time_ns = current_time_ns();
        return span;
    }

    Span start_child_span(const Span& parent, const std::string& name) {
        Span span;
        span.trace_id = parent.trace_id;
        span.span_id = generate_hex(16);
        span.parent_span_id = parent.span_id;
        span.name = name;
        span.start_time_ns = current_time_ns();
        return span;
    }

    void end_span(Span& span) {
        span.end_time_ns = current_time_ns();
        std::lock_guard<std::mutex> lock(mu_);
        finished_spans_.push_back(span);
    }

    std::string render_otlp_json() const {
        std::lock_guard<std::mutex> lock(mu_);
        std::ostringstream ss;

        ss << "{\"resourceSpans\":[{\"resource\":{\"attributes\":[{\"key\":\"service.name\",\"value\":{\"stringValue\":\"garuda\"}}]},\"scopeSpans\":[{\"scope\":{\"name\":\"garuda.tracer\"},\"spans\":[";

        for (std::size_t i = 0; i < finished_spans_.size(); ++i) {
            if (i > 0) ss << ",";
            const auto& s = finished_spans_[i];

            ss << "{"
               << "\"traceId\":\"" << s.trace_id << "\","
               << "\"spanId\":\"" << s.span_id << "\"";

            if (!s.parent_span_id.empty()) {
                ss << ",\"parentSpanId\":\"" << s.parent_span_id << "\"";
            }

            ss << ",\"name\":\"" << escape_json(s.name) << "\","
               << "\"startTimeUnixNano\":\"" << std::to_string(s.start_time_ns) << "\","
               << "\"endTimeUnixNano\":\"" << std::to_string(s.end_time_ns) << "\","
               << "\"attributes\":[";

            bool first_attr = true;
            for (const auto& [k, v] : s.attributes) {
                if (!first_attr) ss << ",";
                ss << "{\"key\":\"" << escape_json(k) << "\",\"value\":{\"stringValue\":\"" << escape_json(v) << "\"}}";
                first_attr = false;
            }

            ss << "]}";
        }

        ss << "]}]}]}";
        return ss.str();
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mu_);
        finished_spans_.clear();
    }

    std::size_t span_count() const {
        std::lock_guard<std::mutex> lock(mu_);
        return finished_spans_.size();
    }

private:
    Tracer() = default;

    static std::string generate_hex(std::size_t len) {
        static const char hex_chars[] = "0123456789abcdef";
        static thread_local std::mt19937 rng(static_cast<unsigned>(std::chrono::system_clock::now().time_since_epoch().count()));
        std::uniform_int_distribution<std::size_t> dist(0, 15);

        std::string res;
        res.reserve(len);
        for (std::size_t i = 0; i < len; ++i) {
            res += hex_chars[dist(rng)];
        }
        return res;
    }

    static std::uint64_t current_time_ns() {
        return static_cast<std::uint64_t>(
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch()
            ).count());
    }

    static std::string escape_json(const std::string& s) {
        std::string res;
        res.reserve(s.length() + 8);
        for (char c : s) {
            if (c == '"') res += "\\\"";
            else if (c == '\\') res += "\\\\";
            else res += c;
        }
        return res;
    }

    mutable std::mutex mu_;
    std::vector<Span> finished_spans_;
};

} // namespace garuda::beam
