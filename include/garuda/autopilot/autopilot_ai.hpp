#pragma once
// =====================================================================
// FASE 5.3 - AUTOPILOT AI ORCHESTRATOR
// Menyatukan AdminGate, HealthMonitor, AICodeFixer, & Audit Log.
// =====================================================================
#include "admin_gate.hpp"
#include "health_monitor.hpp"
#include "ai_code_fixer.hpp"
#include "../jalur_langit/jalur_langit_ultimate.hpp"
#include <string>
#include <iostream>
#include <chrono>

namespace garuda::autopilot {

class AutopilotAI {
public:
    struct ProcessResult {
        bool executed = false;
        bool fix_success = false;
        std::string reason; // alasan jika tidak dieksekusi
        std::string patch;
    };

private:
    AdminGate& gate_;
    HealthMonitor& health_;
    AICodeFixer& fixer_;
    jalur_langit::JalurLangitUltimate& db_;

public:
    AutopilotAI(AdminGate& gate, HealthMonitor& health, AICodeFixer& fixer, jalur_langit::JalurLangitUltimate& db)
        : gate_(gate), health_(health), fixer_(fixer), db_(db) {}

    ProcessResult handle_error(const std::string& error_log,
                               const std::string& file_context,
                               const std::string& target_file) {
        ProcessResult res;

        // Layer 1: Zero-Trust Mode Check
        if (!gate_.can_execute_autonomous()) {
            res.reason = "gate_blocked_not_autopilot";
            return res;
        }

        // Layer 2: Health Check
        if (!health_.is_healthy()) {
            res.reason = "health_unhealthy";
            return res;
        }

        // Layer 3: Eksekusi AI Code Fixer
        res.executed = true;
        auto fix_res = fixer_.fix(error_log, file_context, target_file);
        res.fix_success = fix_res.success;
        res.patch = fix_res.patch;

        // Layer 4: Catat Audit Log ke Jalur Langit (via direct storage API)
        try {
            jalur_langit::Row log_row;
            log_row.columns["action"] = std::string("AUTOPILOT_FIX");
            log_row.columns["target_file"] = target_file;
            log_row.columns["success"] = fix_res.success;
            log_row.columns["error_msg"] = fix_res.error;
            log_row.columns["llm_model"] = fix_res.llm_model;

            // Key unik untuk log: timestamp_nanoseconds
            auto now = std::chrono::steady_clock::now().time_since_epoch().count();
            std::string key = "log_" + std::to_string(now);

            db_.get_storage().put("audit_log", key, log_row);
        } catch (const std::exception& e) {
            // Fail-safe: jika audit log gagal, jangan crash Autopilot
            std::cerr << "[AUTOPILOT] Audit log failed: " << e.what() << "\n";
        }

        return res;
    }
};

} // namespace garuda::autopilot
