#pragma once
// =====================================================================
// FASE D.1.2 - RCA Engine: Analisis akar masalah dari anomali & log
// =====================================================================
#include "anomaly_detector.hpp"
#include <string>
#include <vector>
#include <mutex>
#include <optional>
#include <algorithm>
#include <cstddef>

namespace garuda::autopilot {

struct RootCause {
    std::string anomaly_id;
    std::string probable_cause;
    double confidence = 0.0;  // 0.0 - 1.0
    std::vector<std::string> evidence;  // log lines / metrics
    std::string recommended_action;
};

class RcaEngine {
private:
    std::vector<std::string> recent_logs_;
    mutable std::mutex mu_;

public:
    RcaEngine() = default;

    void ingest_log(const std::string& line) {
        std::lock_guard<std::mutex> lock(mu_);
        recent_logs_.push_back(line);
        if (recent_logs_.size() > 1000) {
            recent_logs_.erase(recent_logs_.begin()); // Keep last 1000 logs
        }
    }

    RootCause analyze(const Anomaly& anomaly) const {
        std::lock_guard<std::mutex> lock(mu_);
        RootCause rc;
        rc.anomaly_id = anomaly.id;

        bool found_log = false;
        std::vector<std::string> matched_logs;

        for (const auto& log : recent_logs_) {
            std::string log_lower = log;
            std::transform(log_lower.begin(), log_lower.end(), log_lower.begin(), ::tolower);

            switch (anomaly.type) {
                case AnomalyType::HIGH_CPU:
                    if (log_lower.find("cpu") != std::string::npos || log_lower.find("spike") != std::string::npos || log_lower.find("high load") != std::string::npos) {
                        found_log = true;
                        matched_logs.push_back(log);
                    }
                    break;
                case AnomalyType::HIGH_MEMORY:
                    if (log_lower.find("memory") != std::string::npos || log_lower.find("oom") != std::string::npos || log_lower.find("leak") != std::string::npos) {
                        found_log = true;
                        matched_logs.push_back(log);
                    }
                    break;
                case AnomalyType::HIGH_ERROR_RATE:
                    if (log_lower.find("error") != std::string::npos || log_lower.find("exception") != std::string::npos || log_lower.find("5xx") != std::string::npos) {
                        found_log = true;
                        matched_logs.push_back(log);
                    }
                    break;
                case AnomalyType::REPEATED_CRASH:
                    if (log_lower.find("segfault") != std::string::npos || log_lower.find("panic") != std::string::npos || log_lower.find("crash") != std::string::npos) {
                        found_log = true;
                        matched_logs.push_back(log);
                    }
                    break;
                case AnomalyType::SLOW_RESPONSE:
                    if (log_lower.find("timeout") != std::string::npos || log_lower.find("slow") != std::string::npos) {
                        found_log = true;
                        matched_logs.push_back(log);
                    }
                    break;
                case AnomalyType::DISK_FULL:
                    if (log_lower.find("disk") != std::string::npos || log_lower.find("space") != std::string::npos) {
                        found_log = true;
                        matched_logs.push_back(log);
                    }
                    break;
                case AnomalyType::NETWORK_ERROR:
                    if (log_lower.find("connection") != std::string::npos || log_lower.find("network") != std::string::npos) {
                        found_log = true;
                        matched_logs.push_back(log);
                    }
                    break;
            }
        }

        rc.evidence = matched_logs;

        if (found_log) {
            switch (anomaly.type) {
                case AnomalyType::HIGH_CPU:
                    rc.probable_cause = "Proses intensif CPU terdeteksi di log (" + std::to_string(matched_logs.size()) + " peristiwa)";
                    rc.confidence = 0.85;
                    rc.recommended_action = "alert";
                    break;
                case AnomalyType::HIGH_MEMORY:
                    rc.probable_cause = "Potensi kebocoran memori terdeteksi di log";
                    rc.confidence = 0.88;
                    rc.recommended_action = "restart";
                    break;
                case AnomalyType::HIGH_ERROR_RATE:
                    rc.probable_cause = "Lonjakan error runtime terdeteksi di log";
                    rc.confidence = 0.80;
                    rc.recommended_action = "alert";
                    break;
                case AnomalyType::REPEATED_CRASH:
                    rc.probable_cause = "Siklus crash / segfault berulang terdeteksi di log";
                    rc.confidence = 0.95;
                    rc.recommended_action = "rollback";
                    break;
                case AnomalyType::SLOW_RESPONSE:
                    rc.probable_cause = "Saturasi I/O atau latensi jaringan tinggi terdeteksi di log";
                    rc.confidence = 0.70;
                    rc.recommended_action = "alert";
                    break;
                case AnomalyType::DISK_FULL:
                    rc.probable_cause = "Kapasitas disk penyimpanan hampir penuh";
                    rc.confidence = 0.95;
                    rc.recommended_action = "alert";
                    break;
                case AnomalyType::NETWORK_ERROR:
                    rc.probable_cause = "Gangguan koneksi jaringan terdeteksi";
                    rc.confidence = 0.75;
                    rc.recommended_action = "alert";
                    break;
            }
        } else {
            rc.probable_cause = "Penyebab anomali belum dapat dikorelasikan dengan log yang ada";
            rc.confidence = 0.35;
            rc.recommended_action = "alert";
        }

        return rc;
    }

    std::size_t log_count() const {
        std::lock_guard<std::mutex> lock(mu_);
        return recent_logs_.size();
    }

    void clear_logs() {
        std::lock_guard<std::mutex> lock(mu_);
        recent_logs_.clear();
    }
};

} // namespace garuda::autopilot
