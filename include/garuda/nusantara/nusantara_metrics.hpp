#pragma once
#include "nusantara_protocol.hpp"
#include "../beam/sys/metrics.hpp"
#include "../beam/sys/tracing.hpp"
#include <atomic>
#include <memory>
#include <mutex>
#include <string>

namespace garuda::nusantara {

class NusantaraMetrics {
private:
    std::atomic<bool> enabled_{false};

public:
    NusantaraMetrics() = default;

    bool enable() {
        enabled_.store(true);
        return true;
    }

    void disable() {
        enabled_.store(false);
    }

    bool is_enabled() const {
        return enabled_.load();
    }

    void inc_messages_received(EntityTarget sender, NusantaraAction action) {
        if (!is_enabled()) return;
        garuda::beam::MetricsRegistry::instance().increment_counter(
            "nusantara_messages_received_total", 1.0,
            {{"sender", to_string(sender)}, {"action", to_string(action)}}
        );
    }

    void inc_messages_processed(EntityTarget receiver, bool success) {
        if (!is_enabled()) return;
        garuda::beam::MetricsRegistry::instance().increment_counter(
            "nusantara_messages_processed_total", 1.0,
            {{"receiver", to_string(receiver)}, {"status", success ? "success" : "error"}}
        );
    }

    void inc_transport_error(const std::string& transport_name) {
        if (!is_enabled()) return;
        garuda::beam::MetricsRegistry::instance().increment_counter(
            "nusantara_transport_errors_total", 1.0,
            {{"transport", transport_name}}
        );
    }

    void set_queue_size(std::size_t size) {
        if (!is_enabled()) return;
        garuda::beam::MetricsRegistry::instance().set_gauge(
            "nusantara_queue_size", static_cast<double>(size)
        );
    }

    void set_db_pending(std::size_t count) {
        if (!is_enabled()) return;
        garuda::beam::MetricsRegistry::instance().set_gauge(
            "nusantara_db_pending", static_cast<double>(count)
        );
    }

    void observe_processing_latency_ms(double latency_ms) {
        if (!is_enabled()) return;
        garuda::beam::MetricsRegistry::instance().observe_histogram(
            "nusantara_processing_latency_ms", latency_ms
        );
    }

    std::string render_prometheus() const {
        if (!is_enabled()) return "";
        return garuda::beam::MetricsRegistry::instance().render_prometheus();
    }
};

class NusantaraTracer {
private:
    std::atomic<bool> enabled_{false};
    std::string service_name_ = "garuda-nusantara";

public:
    struct SpanHandle {
        garuda::beam::Span span;
        bool valid = false;
    };

    NusantaraTracer() = default;

    bool enable(const std::string& service_name = "garuda-nusantara") {
        service_name_ = service_name;
        enabled_.store(true);
        return true;
    }

    void disable() {
        enabled_.store(false);
    }

    bool is_enabled() const {
        return enabled_.load();
    }

    SpanHandle start_span(const NusantaraMessage& msg) {
        if (!is_enabled()) return SpanHandle{};
        SpanHandle h;
        h.span = garuda::beam::Tracer::instance().start_span("nusantara.process_message");
        h.span.set_attribute("message.id", msg.message_id);
        h.span.set_attribute("message.sender", to_string(msg.sender));
        h.span.set_attribute("message.receiver", to_string(msg.receiver));
        h.span.set_attribute("message.action", to_string(msg.action));
        h.valid = true;
        return h;
    }

    void end_span(SpanHandle& h, bool success) {
        if (!is_enabled() || !h.valid) return;
        h.span.set_attribute("processing.status", success ? "success" : "error");
        garuda::beam::Tracer::instance().end_span(h.span);
        h.valid = false;
    }

    std::string export_otlp_json() const {
        if (!is_enabled()) return "{}";
        return garuda::beam::Tracer::instance().render_otlp_json();
    }
};

} // namespace garuda::nusantara
