#pragma once
// =====================================================================
// FASE G2.4 - GARUDA PROCESS GROUP (pg.hpp)
// Setara Erlang/OTP pg / pg2 - Process Group Management
// =====================================================================
#include "../core/runtime.hpp"

#include <string>
#include <vector>
#include <map>
#include <set>
#include <mutex>

namespace garuda::beam {

class ProcessGroup {
private:
    Runtime& rt_;
    mutable std::mutex mu_;
    std::map<std::string, std::set<Pid>> groups_;

public:
    explicit ProcessGroup(Runtime& rt) : rt_(rt) {}

    bool join(const std::string& group_name, Pid pid) {
        if (!pid.valid()) return false;
        std::lock_guard<std::mutex> lock(mu_);
        groups_[group_name].insert(pid);
        return true;
    }

    bool leave(const std::string& group_name, Pid pid) {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = groups_.find(group_name);
        if (it == groups_.end()) return false;
        bool erased = it->second.erase(pid) > 0;
        if (it->second.empty()) {
            groups_.erase(it);
        }
        return erased;
    }

    std::vector<Pid> get_members(const std::string& group_name) const {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = groups_.find(group_name);
        if (it == groups_.end()) return {};
        return std::vector<Pid>(it->second.begin(), it->second.end());
    }

    std::vector<std::string> get_groups() const {
        std::lock_guard<std::mutex> lock(mu_);
        std::vector<std::string> result;
        for (const auto& [name, members] : groups_) {
            if (!members.empty()) {
                result.push_back(name);
            }
        }
        return result;
    }

    std::size_t broadcast(const std::string& group_name, Term msg) {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = groups_.find(group_name);
        if (it == groups_.end()) return 0;

        std::size_t count = 0;
        for (Pid pid : it->second) {
            rt_.send(Pid{}, pid, tuple({atom("pg_msg"), Term(group_name), msg}));
            count++;
        }
        return count;
    }
};

} // namespace garuda::beam
