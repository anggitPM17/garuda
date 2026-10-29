#pragma once
// =====================================================================
// FASE C.5.4 - Audit Log: Catat aksi sensitif (JSON structured ring buffer)
// =====================================================================
#include <string>
#include <vector>
#include <mutex>
#include <chrono>
#include <optional>
#include <algorithm>
#include <sstream>

namespace garuda::security {

struct AuditEntry {
    std::chrono::system_clock::time_point timestamp;
    std::string user_id;
    std::string action;       // "login", "register", "delete_message", etc.
    std::string resource;     // "user:123", "message:456"
    std::string status;       // "success", "failure"
    std::string ip_address;
    std::string details;      // JSON details
};

class AuditLog {
private:
    std::vector<AuditEntry> entries_;
    mutable std::mutex mu_;
    std::size_t max_entries_ = 10000;  // 10k entry max ring buffer
    bool enabled_ = true;

public:
    AuditLog() = default;
    explicit AuditLog(std::size_t max_entries) : max_entries_(max_entries) {}

    void record(const AuditEntry& entry) {
        std::lock_guard<std::mutex> lock(mu_);
        if (!enabled_) return;

        entries_.push_back(entry);
        if (entries_.size() > max_entries_) {
            entries_.erase(entries_.begin()); // Maintain ring buffer
        }
    }

    void record_action(const std::string& user_id,
                       const std::string& action,
                       const std::string& resource,
                       const std::string& status,
                       const std::string& ip_address = "",
                       const std::string& details = "") {
        AuditEntry entry;
        entry.timestamp = std::chrono::system_clock::now();
        entry.user_id = user_id;
        entry.action = action;
        entry.resource = resource;
        entry.status = status;
        entry.ip_address = ip_address;
        entry.details = details;
        record(entry);
    }

    std::vector<AuditEntry> get_entries(std::size_t limit = 100) const {
        std::lock_guard<std::mutex> lock(mu_);
        std::vector<AuditEntry> out;
        std::size_t count = std::min(limit, entries_.size());
        out.assign(entries_.rbegin(), entries_.rbegin() + count); // Most recent first
        return out;
    }

    std::vector<AuditEntry> get_entries_by_user(const std::string& user_id, std::size_t limit = 100) const {
        std::lock_guard<std::mutex> lock(mu_);
        std::vector<AuditEntry> out;
        for (auto it = entries_.rbegin(); it != entries_.rend() && out.size() < limit; ++it) {
            if (it->user_id == user_id) {
                out.push_back(*it);
            }
        }
        return out;
    }

    std::vector<AuditEntry> get_entries_by_action(const std::string& action, std::size_t limit = 100) const {
        std::lock_guard<std::mutex> lock(mu_);
        std::vector<AuditEntry> out;
        for (auto it = entries_.rbegin(); it != entries_.rend() && out.size() < limit; ++it) {
            if (it->action == action) {
                out.push_back(*it);
            }
        }
        return out;
    }

    std::string export_json(std::size_t limit = 1000) const {
        std::lock_guard<std::mutex> lock(mu_);
        std::stringstream ss;
        ss << "{\"entries\":[";

        std::size_t count = std::min(limit, entries_.size());
        for (std::size_t i = 0; i < count; ++i) {
            const auto& e = entries_[entries_.size() - count + i];
            if (i > 0) ss << ",";
            ss << "{\"user_id\":\"" << e.user_id
               << "\",\"action\":\"" << e.action
               << "\",\"resource\":\"" << e.resource
               << "\",\"status\":\"" << e.status
               << "\",\"ip\":\"" << e.ip_address
               << "\"}";
        }
        ss << "]}";
        return ss.str();
    }

    std::size_t entry_count() const {
        std::lock_guard<std::mutex> lock(mu_);
        return entries_.size();
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mu_);
        entries_.clear();
    }

    void set_enabled(bool enabled) {
        std::lock_guard<std::mutex> lock(mu_);
        enabled_ = enabled;
    }

    bool is_enabled() const {
        std::lock_guard<std::mutex> lock(mu_);
        return enabled_;
    }
};

} // namespace garuda::security
