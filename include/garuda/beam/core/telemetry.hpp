#pragma once
// Garuda BEAM-Core: Telemetry, Metrics, and Process Inspector
// Inspeksi memori proses real-time (process_info/2) dan exporter format Prometheus / JSON.
#include "runtime.hpp"

#include <chrono>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace garuda::beam {

struct ProcessSnapshot {
    std::uint64_t pid_id = 0;
    std::string name;
    std::size_t mailbox_length = 0;
    std::uint64_t reductions = 0;
    std::string state;
};

class TelemetryEngine {
public:
    static std::string export_prometheus(const RuntimeStats& stats, std::size_t active_processes) {
        std::ostringstream ss;
        ss << "# HELP garuda_processes_active Total active Garuda BEAM processes\n";
        ss << "# TYPE garuda_processes_active gauge\n";
        ss << "garuda_processes_active " << active_processes << "\n\n";

        ss << "# HELP garuda_processes_spawned_total Total spawned processes\n";
        ss << "# TYPE garuda_processes_spawned_total counter\n";
        ss << "garuda_processes_spawned_total " << stats.spawned << "\n\n";

        ss << "# HELP garuda_messages_total Total delivered BEAM messages\n";
        ss << "# TYPE garuda_messages_total counter\n";
        ss << "garuda_messages_total " << stats.messages << "\n\n";

        ss << "# HELP garuda_reductions_total Total reductions executed\n";
        ss << "# TYPE garuda_reductions_total counter\n";
        ss << "garuda_reductions_total " << stats.reductions << "\n\n";

        ss << "# HELP garuda_crashes_total Total process crash events\n";
        ss << "# TYPE garuda_crashes_total counter\n";
        ss << "garuda_crashes_total " << stats.crashes << "\n";

        return ss.str();
    }

    static std::string export_json(const RuntimeStats& stats, std::size_t active_processes) {
        std::ostringstream ss;
        ss << "{\n";
        ss << "  \"engine\": \"Garuda GVM v67.0.0\",\n";
        ss << "  \"active_processes\": " << active_processes << ",\n";
        ss << "  \"spawned_processes\": " << stats.spawned << ",\n";
        ss << "  \"messages_total\": " << stats.messages << ",\n";
        ss << "  \"reductions_total\": " << stats.reductions << ",\n";
        ss << "  \"preemptions\": " << stats.preemptions << ",\n";
        ss << "  \"crashes\": " << stats.crashes << "\n";
        ss << "}";
        return ss.str();
    }
};

} // namespace garuda::beam
