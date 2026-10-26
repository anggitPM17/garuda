#pragma once
// Garuda Language Embedded IoT Engine: Dedicated MQTT Client Header (`mqtt_client.hpp`)
#include "gvm_micro.hpp"

#include <string>
#include <functional>

namespace garuda::beam::micro {

// Dedicated MqttClient Wrapper for explicit header inclusion
using DedicatedMqttClient = MqttClient;

} // namespace garuda::beam::micro
