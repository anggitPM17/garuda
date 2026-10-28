#pragma once
// =====================================================================
// FASE G6.3 - Observer: TUI monitoring (setara observer Erlang)
// =====================================================================
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <chrono>
#include <sstream>
#include <cstddef>
#include <cstdint>

namespace garuda::beam {

struct ProcessInfo {
    std::string pid;
    std::string name;
    std::string state;  // running, waiting, terminated
    std::uint64_t reductions = 0;
    std::uint64_t memory_bytes = 0;
};

struct SystemSnapshot {
    std::chrono::steady_clock::time_point timestamp;
    std::size_t process_count = 0;
    std::size_t total_memory_bytes = 0;
    std::vector<ProcessInfo> processes;
};

class Observer {
private:
    std::map<std::string, ProcessInfo> processes_;
    std::map<std::string, std::uint64_t> counters_;  // metric_name -> value
    mutable std::mutex mu_;

public:
    void register_process(const ProcessInfo& info) {
        std::lock_guard<std::mutex> lock(mu_);
        processes_[info.pid] = info;
    }

    void unregister_process(const std::string& pid) {
        std::lock_guard<std::mutex> lock(mu_);
        processes_.erase(pid);
    }

    void increment_counter(const std::string& name, std::uint64_t delta = 1) {
        std::lock_guard<std::mutex> lock(mu_);
        counters_[name] += delta;
    }

    void set_counter(const std::string& name, std::uint64_t value) {
        std::lock_guard<std::mutex> lock(mu_);
        counters_[name] = value;
    }

    SystemSnapshot snapshot() const {
        std::lock_guard<std::mutex> lock(mu_);
        SystemSnapshot snap;
        snap.timestamp = std::chrono::steady_clock::now();
        snap.process_count = processes_.size();
        snap.total_memory_bytes = 0;
        for (const auto& [pid, info] : processes_) {
            snap.processes.push_back(info);
            snap.total_memory_bytes += info.memory_bytes;
        }
        return snap;
    }

    std::string render_text() const {
        std::lock_guard<std::mutex> lock(mu_);
        std::ostringstream ss;
        ss << "========================================\n";
        ss << "   GARUDA GVM OBSERVER DASHBOARD\n";
        ss << "========================================\n";
        ss << "Total Processes : " << processes_.size() << "\n";

        std::uint64_t total_mem = 0;
        for (const auto& [pid, info] : processes_) {
            total_mem += info.memory_bytes;
        }
        ss << "Total Memory    : " << total_mem << " bytes\n\n";

        ss << "--- System Metrics ---\n";
        for (const auto& [name, val] : counters_) {
            ss << "  " << name << " : " << val << "\n";
        }
        ss << "\n--- Active Processes ---\n";
        for (const auto& [pid, info] : processes_) {
            ss << "  PID: " << info.pid << " | Name: " << info.name
               << " | State: " << info.state << " | Reductions: " << info.reductions
               << " | Memory: " << info.memory_bytes << " B\n";
        }
        ss << "========================================\n";
        return ss.str();
    }

    std::size_t process_count() const {
        std::lock_guard<std::mutex> lock(mu_);
        return processes_.size();
    }

    std::uint64_t get_counter(const std::string& name) const {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = counters_.find(name);
        if (it != counters_.end()) return it->second;
        return 0;
    }
};

} // namespace garuda::beam
