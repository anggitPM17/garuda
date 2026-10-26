#pragma once
// Garuda BEAM-Core: 4-Tier Priority Scheduler (`priority_scheduler.hpp`)
// Fitur DEWA Erlang: Priority Queues (Max, High, Normal, Low) untuk mengutamakan supervisor & system heartbeats.
#include "runtime.hpp"

#include <array>
#include <deque>
#include <iostream>
#include <mutex>

namespace garuda::beam {

enum class Priority { Max = 0, High = 1, Normal = 2, Low = 3 };

struct PriorityProcessRef {
    Pid pid;
    Priority priority = Priority::Normal;
};

class PriorityScheduler {
public:
    void enqueue(Pid pid, Priority prio) {
        std::lock_guard<std::mutex> lock(mu_);
        queues_[static_cast<std::size_t>(prio)].push_back(pid);
    }

    std::optional<Pid> pop_next() {
        std::lock_guard<std::mutex> lock(mu_);
        // 1. Max priority -> selalu dikosongkan dahulu
        for (std::size_t p = 0; p < 4; ++p) {
            if (!queues_[p].empty()) {
                Pid pid = queues_[p].front();
                queues_[p].pop_front();
                return pid;
            }
        }
        return std::nullopt;
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(mu_);
        for (const auto& q : queues_) {
            if (!q.empty()) return false;
        }
        return true;
    }

private:
    mutable std::mutex mu_;
    std::array<std::deque<Pid>, 4> queues_;
};

} // namespace garuda::beam
