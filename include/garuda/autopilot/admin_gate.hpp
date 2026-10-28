#pragma once
// =====================================================================
// FASE 5.1 - ADMIN GATE
// Gerbang mode Autopilot AI: MANUAL / AUTOPILOT / STOP.
// Hanya pemegang admin_token yang boleh mengubah mode.
// =====================================================================
#include <atomic>
#include <mutex>
#include <string>
#include <stdexcept>

namespace garuda::autopilot {

enum class AutopilotMode {
    MANUAL,     // AI tidak boleh eksekusi otomatis
    AUTOPILOT,  // AI boleh eksekusi otomatis
    STOP        // Darurat: AI berhenti total, tidak bisa diubah tanpa token
};

inline const char* to_string(AutopilotMode m) {
    switch (m) {
        case AutopilotMode::MANUAL:    return "MANUAL";
        case AutopilotMode::AUTOPILOT: return "AUTOPILOT";
        case AutopilotMode::STOP:      return "STOP";
    }
    return "UNKNOWN";
}

class AdminGateError : public std::runtime_error {
public:
    explicit AdminGateError(const std::string& msg) : std::runtime_error(msg) {}
};

class AdminGate {
private:
    std::atomic<AutopilotMode> mode_{AutopilotMode::MANUAL};
    std::string admin_token_;
    mutable std::mutex mutex_;

public:
    AdminGate() = default;

    // Set token admin. Wajib dipanggil sebelum set_mode().
    void set_admin_token(const std::string& token) {
        std::lock_guard<std::mutex> lock(mutex_);
        admin_token_ = token;
    }

    bool has_admin_token() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return !admin_token_.empty();
    }

    // Ubah mode. Return true jika berhasil, false jika token salah/kosong.
    // Fail-safe: tidak throw untuk token salah (return false).
    bool set_mode(AutopilotMode new_mode, const std::string& admin_token) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (admin_token_.empty()) return false;          // belum ada token
        if (admin_token != admin_token_) return false;   // token salah
        mode_.store(new_mode, std::memory_order_release);
        return true;
    }

    AutopilotMode get_mode() const {
        return mode_.load(std::memory_order_acquire);
    }

    // Inti keamanan: AI hanya boleh eksekusi jika mode == AUTOPILOT.
    bool can_execute_autonomous() const {
        return get_mode() == AutopilotMode::AUTOPILOT;
    }

    // Reset darurat: paksa ke STOP tanpa token (untuk kill-switch fisik).
    // Hanya bisa turun ke STOP, tidak bisa naik.
    void force_stop() {
        mode_.store(AutopilotMode::STOP, std::memory_order_release);
    }
};

} // namespace garuda::autopilot
