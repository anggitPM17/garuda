#pragma once
// =====================================================================
// FASE D.1.3 - Action Engine: Eksekusi aksi perbaikan (dengan safety)
// =====================================================================
#include "rca_engine.hpp"
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <functional>
#include <optional>
#include <chrono>
#include <algorithm>
#include <cstddef>

namespace garuda::autopilot {

enum class ActionMode { MANUAL, AUTOPILOT };

enum class ActionType {
    RESTART_SERVICE,
    ROLLBACK_CONFIG,
    ALERT_ADMIN,
    APPLY_PATCH,
    SCALE_UP,
    NO_ACTION
};

struct Action {
    std::string id;
    ActionType type = ActionType::NO_ACTION;
    std::string description;
    std::string target;
    bool requires_approval = true;
    std::chrono::steady_clock::time_point created_at;
};

struct ActionResult {
    std::string action_id;
    bool success = false;
    std::string error;
    std::chrono::microseconds duration{0};
};

class ActionEngine {
private:
    ActionMode mode_ = ActionMode::MANUAL;
    std::vector<Action> pending_actions_;
    std::vector<ActionResult> action_history_;
    mutable std::mutex mu_;
    std::function<bool(const Action&)> on_execute_;  // Hook untuk test/mock execution

public:
    ActionEngine() = default;

    void set_mode(ActionMode mode) {
        std::lock_guard<std::mutex> lock(mu_);
        mode_ = mode;
    }

    ActionMode mode() const {
        std::lock_guard<std::mutex> lock(mu_);
        return mode_;
    }

    void set_execute_hook(std::function<bool(const Action&)> hook) {
        std::lock_guard<std::mutex> lock(mu_);
        on_execute_ = std::move(hook);
    }

    std::optional<Action> propose(const RootCause& rca) {
        std::lock_guard<std::mutex> lock(mu_);
        Action act;
        act.id = "act_" + std::to_string(pending_actions_.size() + action_history_.size() + 1);
        act.created_at = std::chrono::steady_clock::now();

        if (rca.recommended_action == "restart") {
            act.type = ActionType::RESTART_SERVICE;
            act.description = "Restart service due to: " + rca.probable_cause;
            act.requires_approval = true;
        } else if (rca.recommended_action == "rollback") {
            act.type = ActionType::ROLLBACK_CONFIG;
            act.description = "Rollback config/build due to: " + rca.probable_cause;
            act.requires_approval = true;
        } else if (rca.recommended_action == "alert") {
            act.type = ActionType::ALERT_ADMIN;
            act.description = "Send notification alert to admin: " + rca.probable_cause;
            act.requires_approval = false; // Always safe to alert
        } else if (rca.recommended_action == "patch") {
            act.type = ActionType::APPLY_PATCH;
            act.description = "Apply hot patch for: " + rca.probable_cause;
            act.requires_approval = true;
        } else {
            act.type = ActionType::NO_ACTION;
            act.description = "No automated action needed";
            act.requires_approval = false;
        }

        if (mode_ == ActionMode::AUTOPILOT && !act.requires_approval) {
            // Auto execute
            ActionResult res;
            res.action_id = act.id;
            res.success = on_execute_ ? on_execute_(act) : true;
            action_history_.push_back(res);
            return act;
        }

        pending_actions_.push_back(act);
        return act;
    }

    ActionResult approve(const std::string& action_id) {
        std::lock_guard<std::mutex> lock(mu_);
        ActionResult res;
        res.action_id = action_id;

        auto it = std::find_if(pending_actions_.begin(), pending_actions_.end(),
            [&](const Action& a) { return a.id == action_id; });

        if (it != pending_actions_.end()) {
            auto t0 = std::chrono::steady_clock::now();
            res.success = on_execute_ ? on_execute_(*it) : true;
            auto t1 = std::chrono::steady_clock::now();
            res.duration = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0);

            pending_actions_.erase(it);
            action_history_.push_back(res);
        } else {
            res.success = false;
            res.error = "Action ID tidak ditemukan di pending_actions";
        }

        return res;
    }

    bool reject(const std::string& action_id) {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = std::find_if(pending_actions_.begin(), pending_actions_.end(),
            [&](const Action& a) { return a.id == action_id; });

        if (it != pending_actions_.end()) {
            pending_actions_.erase(it);
            return true;
        }
        return false;
    }

    std::vector<Action> pending_actions() const {
        std::lock_guard<std::mutex> lock(mu_);
        return pending_actions_;
    }

    std::vector<ActionResult> history() const {
        std::lock_guard<std::mutex> lock(mu_);
        return action_history_;
    }

    std::size_t pending_count() const {
        std::lock_guard<std::mutex> lock(mu_);
        return pending_actions_.size();
    }

    std::size_t executed_count() const {
        std::lock_guard<std::mutex> lock(mu_);
        return action_history_.size();
    }
};

} // namespace garuda::autopilot
