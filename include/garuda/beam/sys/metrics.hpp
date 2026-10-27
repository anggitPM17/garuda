#pragma once
// Garuda Production Infrastructure: Prometheus Metrics Exporter (`metrics.hpp`)
#include <atomic>
#include <map>
#include <mutex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace garuda::beam {

enum class MetricType { Counter, Gauge, Histogram };

struct MetricKey {
    std::string name;
    std::map<std::string, std::string> labels;

    bool operator<(const MetricKey& o) const {
        if (name != o.name) return name < o.name;
        return labels < o.labels;
    }
};

class MetricsRegistry {
public:
    static MetricsRegistry& instance() {
        static MetricsRegistry inst;
        return inst;
    }

    void increment_counter(const std::string& name, double delta = 1.0, const std::map<std::string, std::string>& labels = {}) {
        std::lock_guard<std::mutex> lock(mu_);
        MetricKey key{name, labels};
        metric_types_[name] = MetricType::Counter;
        counters_[key] += delta;
    }

    void set_gauge(const std::string& name, double value, const std::map<std::string, std::string>& labels = {}) {
        std::lock_guard<std::mutex> lock(mu_);
        MetricKey key{name, labels};
        metric_types_[name] = MetricType::Gauge;
        gauges_[key] = value;
    }

    void observe_histogram(const std::string& name, double value, const std::map<std::string, std::string>& labels = {}) {
        std::lock_guard<std::mutex> lock(mu_);
        MetricKey key{name, labels};
        metric_types_[name] = MetricType::Histogram;
        histograms_[key].push_back(value);
    }

    std::string render_prometheus() const {
        std::lock_guard<std::mutex> lock(mu_);
        std::ostringstream ss;

        // 1. Render Counters
        for (const auto& [key, val] : counters_) {
            ss << "# HELP " << key.name << " " << key.name << " metric\n";
            ss << "# TYPE " << key.name << " counter\n";
            ss << key.name << format_labels(key.labels) << " " << val << "\n";
        }

        // 2. Render Gauges
        for (const auto& [key, val] : gauges_) {
            ss << "# HELP " << key.name << " " << key.name << " metric\n";
            ss << "# TYPE " << key.name << " gauge\n";
            ss << key.name << format_labels(key.labels) << " " << val << "\n";
        }

        // 3. Render Histograms (Sum & Count)
        for (const auto& [key, values] : histograms_) {
            double sum = 0;
            for (double v : values) sum += v;
            ss << "# HELP " << key.name << " " << key.name << " metric\n";
            ss << "# TYPE " << key.name << " histogram\n";
            ss << key.name << "_sum" << format_labels(key.labels) << " " << sum << "\n";
            ss << key.name << "_count" << format_labels(key.labels) << " " << values.size() << "\n";
        }

        return ss.str();
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mu_);
        counters_.clear();
        gauges_.clear();
        histograms_.clear();
        metric_types_.clear();
    }

private:
    MetricsRegistry() = default;

    static std::string format_labels(const std::map<std::string, std::string>& labels) {
        if (labels.empty()) return "";
        std::ostringstream ss;
        ss << "{";
        bool first = true;
        for (const auto& [k, v] : labels) {
            if (!first) ss << ",";
            ss << k << "=\"" << v << "\"";
            first = false;
        }
        ss << "}";
        return ss.str();
    }

    mutable std::mutex mu_;
    std::unordered_map<std::string, MetricType> metric_types_;
    std::map<MetricKey, double> counters_;
    std::map<MetricKey, double> gauges_;
    std::map<MetricKey, std::vector<double>> histograms_;
};

} // namespace garuda::beam
