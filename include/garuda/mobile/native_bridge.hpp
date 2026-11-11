#ifndef GARUDA_MOBILE_NATIVE_BRIDGE_HPP
#define GARUDA_MOBILE_NATIVE_BRIDGE_HPP

#include <string>
#include <functional>
#include <map>
#include <vector>
#include <mutex>
#include <chrono>
#include <iostream>

namespace garuda {
namespace mobile {

struct BridgeMessage {
    std::string channel;
    std::string payload;
    uint64_t timestamp_ms = 0;
};

class GvmNativeBridge {
private:
    std::map<std::string, std::function<std::string(const std::string&)>> action_handlers_;
    std::vector<std::function<void(const BridgeMessage&)>> ui_listeners_;
    mutable std::mutex bridge_mutex_;

public:
    GvmNativeBridge() = default;

    void register_ui_listener(std::function<void(const BridgeMessage&)> listener) {
        std::lock_guard<std::mutex> lock(bridge_mutex_);
        ui_listeners_.push_back(listener);
    }

    void register_action_handler(const std::string& channel, std::function<std::string(const std::string&)> handler) {
        std::lock_guard<std::mutex> lock(bridge_mutex_);
        action_handlers_[channel] = handler;
    }

    void emit_to_ui(const std::string& channel, const std::string& payload) {
        BridgeMessage msg;
        msg.channel = channel;
        msg.payload = payload;
        msg.timestamp_ms = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());

        std::lock_guard<std::mutex> lock(bridge_mutex_);
        for (const auto& listener : ui_listeners_) {
            if (listener) listener(msg);
        }
    }

    std::string dispatch_from_ui(const std::string& channel, const std::string& payload) {
        std::function<std::string(const std::string&)> handler;
        {
            std::lock_guard<std::mutex> lock(bridge_mutex_);
            auto it = action_handlers_.find(channel);
            if (it != action_handlers_.end()) {
                handler = it->second;
            }
        }

        if (handler) {
            return handler(payload);
        }

        std::cerr << "[GvmNativeBridge] Warning: No handler registered for UI channel '" << channel << "'" << std::endl;
        return "{\"status\": \"error\", \"message\": \"Channel handler not found\"}";
    }

    bool has_handler(const std::string& channel) const {
        std::lock_guard<std::mutex> lock(bridge_mutex_);
        return action_handlers_.find(channel) != action_handlers_.end();
    }

    void clear_listeners() {
        std::lock_guard<std::mutex> lock(bridge_mutex_);
        ui_listeners_.clear();
        action_handlers_.clear();
    }
};

} // namespace mobile
} // namespace garuda

#endif // GARUDA_MOBILE_NATIVE_BRIDGE_HPP
