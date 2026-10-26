#pragma once
// Garuda Language Embedded Industrial Engine: Device Fleet Management (`fleet_manager.hpp`)
#include "opcua_bridge.hpp"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace garuda::beam::micro {

struct FleetDevice {
    std::string device_id;
    std::string metadata;
    std::string firmware_url;
    bool is_active = true;
};

class FleetManager {
public:
    FleetManager() = default;

    bool register_device(const std::string& device_id, const std::string& metadata) {
        devices_[device_id] = {device_id, metadata, "v1.0.0", true};
        return true;
    }

    bool update_device(const std::string& device_id, const std::string& firmware_url) {
        auto it = devices_.find(device_id);
        if (it != devices_.end() && it->second.is_active) {
            it->second.firmware_url = firmware_url;
            return true;
        }
        return false;
    }

    std::vector<FleetDevice> query_devices(const std::string& filter) const {
        std::vector<FleetDevice> res;
        for (const auto& [id, dev] : devices_) {
            if (dev.is_active && (filter.empty() || id.find(filter) != std::string::npos)) {
                res.push_back(dev);
            }
        }
        return res;
    }

    bool revoke_device(const std::string& device_id) {
        auto it = devices_.find(device_id);
        if (it != devices_.end()) {
            it->second.is_active = false;
            return true;
        }
        return false;
    }

    std::size_t device_count() const { return devices_.size(); }

private:
    std::map<std::string, FleetDevice> devices_;
};

} // namespace garuda::beam::micro
