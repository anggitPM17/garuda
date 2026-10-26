#pragma once
// Garuda Language Embedded IoT Engine: Device Provisioning (`device_provisioning.hpp`)
#include "lorawan.hpp"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <map>
#include <string>

namespace garuda::beam::micro {

struct DeviceCredentials {
    std::string device_id;
    std::string token;
    bool is_active = true;
};

class DeviceProvisioning {
public:
    DeviceProvisioning() = default;

    bool provision(const std::string& device_id, const std::string& initial_token) {
        devices_[device_id] = {device_id, initial_token, true};
        return true;
    }

    bool is_provisioned(const std::string& device_id) const {
        auto it = devices_.find(device_id);
        return it != devices_.end() && it->second.is_active;
    }

    bool rotate_credentials(const std::string& device_id, const std::string& new_token) {
        auto it = devices_.find(device_id);
        if (it != devices_.end() && it->second.is_active) {
            it->second.token = new_token;
            return true;
        }
        return false;
    }

    bool revoke(const std::string& device_id) {
        auto it = devices_.find(device_id);
        if (it != devices_.end()) {
            it->second.is_active = false;
            return true;
        }
        return false;
    }

private:
    std::map<std::string, DeviceCredentials> devices_;
};

} // namespace garuda::beam::micro
