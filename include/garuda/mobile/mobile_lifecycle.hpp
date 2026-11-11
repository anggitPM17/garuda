#ifndef GARUDA_MOBILE_LIFECYCLE_HPP
#define GARUDA_MOBILE_LIFECYCLE_HPP

#include <string>
#include <functional>
#include <vector>
#include <iostream>
#include <mutex>

namespace garuda {
namespace mobile {

enum class MobileLifecycleState {
    CREATED,
    STARTED,
    RESUMED,
    PAUSED,
    STOPPED,
    DESTROYED
};

enum class TrimMemoryLevel {
    RUNNING_LOW,
    MODERATE,
    COMPLETE
};

class GvmMobileLifecycleManager {
private:
    MobileLifecycleState current_state_ = MobileLifecycleState::DESTROYED;
    bool power_save_active_ = false;
    mutable std::mutex lifecycle_mutex_;

    std::vector<std::function<void(MobileLifecycleState)>> state_change_callbacks_;
    std::vector<std::function<void(TrimMemoryLevel)>> trim_memory_callbacks_;

public:
    GvmMobileLifecycleManager() = default;

    void register_state_callback(std::function<void(MobileLifecycleState)> cb) {
        std::lock_guard<std::mutex> lock(lifecycle_mutex_);
        state_change_callbacks_.push_back(cb);
    }

    void register_trim_memory_callback(std::function<void(TrimMemoryLevel)> cb) {
        std::lock_guard<std::mutex> lock(lifecycle_mutex_);
        trim_memory_callbacks_.push_back(cb);
    }

    void on_create() {
        std::lock_guard<std::mutex> lock(lifecycle_mutex_);
        current_state_ = MobileLifecycleState::CREATED;
        power_save_active_ = false;
        notify_state_change(current_state_);
    }

    void on_start() {
        std::lock_guard<std::mutex> lock(lifecycle_mutex_);
        current_state_ = MobileLifecycleState::STARTED;
        notify_state_change(current_state_);
    }

    void on_resume() {
        std::lock_guard<std::mutex> lock(lifecycle_mutex_);
        current_state_ = MobileLifecycleState::RESUMED;
        power_save_active_ = false;
        notify_state_change(current_state_);
    }

    void on_pause() {
        std::lock_guard<std::mutex> lock(lifecycle_mutex_);
        current_state_ = MobileLifecycleState::PAUSED;
        power_save_active_ = true; // Activate power-saving mode for GVM Actor Runtime
        notify_state_change(current_state_);
    }

    void on_stop() {
        std::lock_guard<std::mutex> lock(lifecycle_mutex_);
        current_state_ = MobileLifecycleState::STOPPED;
        power_save_active_ = true;
        notify_state_change(current_state_);
    }

    void on_trim_memory(TrimMemoryLevel level) {
        std::lock_guard<std::mutex> lock(lifecycle_mutex_);
        for (const auto& cb : trim_memory_callbacks_) {
            if (cb) cb(level);
        }
    }

    void on_destroy() {
        std::lock_guard<std::mutex> lock(lifecycle_mutex_);
        current_state_ = MobileLifecycleState::DESTROYED;
        power_save_active_ = true;
        notify_state_change(current_state_);
    }

    MobileLifecycleState get_current_state() const {
        std::lock_guard<std::mutex> lock(lifecycle_mutex_);
        return current_state_;
    }

    bool is_power_save_active() const {
        std::lock_guard<std::mutex> lock(lifecycle_mutex_);
        return power_save_active_;
    }

    static std::string state_to_string(MobileLifecycleState state) {
        switch (state) {
            case MobileLifecycleState::CREATED: return "CREATED";
            case MobileLifecycleState::STARTED: return "STARTED";
            case MobileLifecycleState::RESUMED: return "RESUMED";
            case MobileLifecycleState::PAUSED: return "PAUSED";
            case MobileLifecycleState::STOPPED: return "STOPPED";
            case MobileLifecycleState::DESTROYED: return "DESTROYED";
            default: return "UNKNOWN";
        }
    }

private:
    void notify_state_change(MobileLifecycleState new_state) {
        for (const auto& cb : state_change_callbacks_) {
            if (cb) cb(new_state);
        }
    }
};

} // namespace mobile
} // namespace garuda

#endif // GARUDA_MOBILE_LIFECYCLE_HPP
