#pragma once
// =====================================================================
// FASE D.1.4 - Autopilot Runtime: Integrator self-healing end-to-end
// =====================================================================
#include "anomaly_detector.hpp"
#include "rca_engine.hpp"
#include "action_engine.hpp"

#include <string>
#include <vector>
#include <mutex>
#include <sstream>
#include <chrono>

namespace garuda::autopilot {

struct AutopilotEvent {
    std::chrono::steady_clock::time_point timestamp;
    std::string event_type;  // "anomaly_detected", "rca_completed", "action_proposed", "action_executed"
    std::string details;
};

class AutopilotRuntime {
private:
    AnomalyDetector detector_;
    RcaEngine rca_;
    ActionEngine actions_;
    std::vector<AutopilotEvent> event_log_;
    mutable std::mutex mu_;

public:
    AutopilotRuntime() = default;

    AnomalyDetector& detector() { return detector_; }
    RcaEngine& rca() { return rca_; }
    ActionEngine& actions() { return actions_; }

    void set_mode(ActionMode mode) {
        actions_.set_mode(mode);
    }

    void tick() {
        std::lock_guard<std::mutex> lock(mu_);
        auto anomalies = detector_.active_anomalies();

        for (const auto& anomaly : anomalies) {
            // Log anomaly_detected event
            AutopilotEvent ev1;
            ev1.timestamp = std::chrono::steady_clock::now();
            ev1.event_type = "anomaly_detected";
            ev1.details = "Anomaly ID: " + anomaly.id + " (" + anomaly.description + ")";
            event_log_.push_back(ev1);

            // Analyze RCA
            auto rc = rca_.analyze(anomaly);
            AutopilotEvent ev2;
            ev2.timestamp = std::chrono::steady_clock::now();
            ev2.event_type = "rca_completed";
            ev2.details = "Probable cause: " + rc.probable_cause + " (confidence: " + std::to_string(rc.confidence) + ")";
            event_log_.push_back(ev2);

            // Propose Action
            auto act = actions_.propose(rc);
            if (act.has_value()) {
                AutopilotEvent ev3;
                ev3.timestamp = std::chrono::steady_clock::now();
                ev3.event_type = "action_proposed";
                ev3.details = "Action proposed: " + act->description;
                event_log_.push_back(ev3);

                if (actions_.mode() == ActionMode::AUTOPILOT && !act->requires_approval) {
                    AutopilotEvent ev4;
                    ev4.timestamp = std::chrono::steady_clock::now();
                    ev4.event_type = "action_executed";
                    ev4.details = "Auto-executed action: " + act->id;
                    event_log_.push_back(ev4);
                }
            }
        }
    }

    std::vector<AutopilotEvent> event_log() const {
        std::lock_guard<std::mutex> lock(mu_);
        return event_log_;
    }

    std::size_t anomaly_count() const {
        return detector_.anomaly_count();
    }

    std::size_t pending_action_count() const {
        return actions_.pending_count();
    }

    std::size_t executed_action_count() const {
        return actions_.executed_count();
    }

    std::string render_status() const {
        std::lock_guard<std::mutex> lock(mu_);
        std::stringstream ss;
        ss << "=======================================================\n";
        ss << "   GARUDA BEAM — AUTOPILOT AI STATUS DASHBOARD\n";
        ss << "=======================================================\n";
        ss << " Mode             : " << (actions_.mode() == ActionMode::AUTOPILOT ? "AUTOPILOT" : "MANUAL") << "\n";
        ss << " Active Anomalies : " << detector_.anomaly_count() << "\n";
        ss << " Pending Actions  : " << actions_.pending_count() << "\n";
        ss << " Executed Actions : " << actions_.executed_count() << "\n";
        ss << " Event Log Count  : " << event_log_.size() << "\n";
        ss << "-------------------------------------------------------\n";
        ss << " Recent Events:\n";
        int start = std::max(0, static_cast<int>(event_log_.size()) - 5);
        for (std::size_t i = static_cast<std::size_t>(start); i < event_log_.size(); ++i) {
            ss << "  [" << event_log_[i].event_type << "] " << event_log_[i].details << "\n";
        }
        ss << "=======================================================\n";
        return ss.str();
    }
};

} // namespace garuda::autopilot
