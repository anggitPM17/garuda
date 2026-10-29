#pragma once
// =====================================================================
// FASE G8.1 - RICHAT Application behaviour (setara richat_app.erl)
// =====================================================================
#include "../beam/otp/application.hpp"
#include "../beam/otp/supervisor.hpp"
#include "../beam/otp/mnesia.hpp"
#include <string>
#include <memory>
#include <mutex>
#include <atomic>
#include <chrono>

namespace garuda::richat {

struct RichatAppConfig {
    std::string app_name = "richat";
    std::string version = "1.0.0-garuda";
    std::string db_dir = "./richat_db";
    bool enable_telemetry = true;
    bool enable_webrtc = true;
    bool enable_groups = true;
};

enum class AppState { STOPPED, STARTING, RUNNING, STOPPING, FAILED };

class RichatApp {
private:
    RichatAppConfig config_;
    AppState state_ = AppState::STOPPED;
    mutable std::mutex mu_;
    std::unique_ptr<garuda::beam::Mnesia> mnesia_;
    std::atomic<std::uint64_t> started_at_ms_{0};

public:
    explicit RichatApp(RichatAppConfig config = {})
        : config_(std::move(config)) {}

    bool start() {
        std::lock_guard<std::mutex> lock(mu_);
        if (state_ == AppState::RUNNING || state_ == AppState::STARTING) {
            return false;
        }

        state_ = AppState::STARTING;
        try {
            mnesia_ = std::make_unique<garuda::beam::Mnesia>(config_.db_dir);
            mnesia_->init_richat_schema();
            started_at_ms_.store(static_cast<std::uint64_t>(
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now().time_since_epoch()).count()));
            state_ = AppState::RUNNING;
            return true;
        } catch (...) {
            state_ = AppState::FAILED;
            return false;
        }
    }

    bool stop() {
        std::lock_guard<std::mutex> lock(mu_);
        if (state_ == AppState::STOPPED) return false;

        state_ = AppState::STOPPING;
        mnesia_.reset();
        started_at_ms_.store(0);
        state_ = AppState::STOPPED;
        return true;
    }

    AppState state() const {
        std::lock_guard<std::mutex> lock(mu_);
        return state_;
    }

    std::string state_string() const {
        std::lock_guard<std::mutex> lock(mu_);
        switch (state_) {
            case AppState::STOPPED: return "STOPPED";
            case AppState::STARTING: return "STARTING";
            case AppState::RUNNING: return "RUNNING";
            case AppState::STOPPING: return "STOPPING";
            case AppState::FAILED: return "FAILED";
        }
        return "UNKNOWN";
    }

    bool is_running() const {
        std::lock_guard<std::mutex> lock(mu_);
        return state_ == AppState::RUNNING;
    }

    garuda::beam::Mnesia* mnesia() {
        std::lock_guard<std::mutex> lock(mu_);
        return mnesia_.get();
    }

    RichatAppConfig config() const {
        std::lock_guard<std::mutex> lock(mu_);
        return config_;
    }

    bool health_check() const {
        std::lock_guard<std::mutex> lock(mu_);
        return state_ == AppState::RUNNING && mnesia_ != nullptr;
    }

    std::uint64_t uptime_ms() const {
        std::lock_guard<std::mutex> lock(mu_);
        if (state_ != AppState::RUNNING || started_at_ms_.load() == 0) return 0;
        std::uint64_t now = static_cast<std::uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count());
        return now >= started_at_ms_.load() ? (now - started_at_ms_.load()) : 0;
    }
};

} // namespace garuda::richat
