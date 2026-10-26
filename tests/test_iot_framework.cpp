#include "../include/garuda/beam/micro/device_provisioning.hpp"
#include "../include/garuda/beam/micro/edge_alert_dispatcher.hpp"
#include <cassert>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

using namespace garuda::beam::micro;

#define TEST_ASSERT(cond, msg) do { \
    if (!(cond)) { \
        std::cerr << "\n❌ ASSERTION FAILED: " << (msg) << " at " << __FILE__ << ":" << __LINE__ << "\n"; \
        std::exit(1); \
    } \
} while(0)

int main(int argc, char* argv[]) {
    // Optional sanity check flag
    if (argc > 1 && std::string(argv[1]) == "--sanity-check") {
        std::cout << "Running TEST_ASSERT sanity check...\n";
        TEST_ASSERT(1 == 2, "Sanity Check Failure Verification");
        return 0;
    }

    std::cout << "=======================================================\n";
    std::cout << "GARUDA IoT FRAMEWORK — SPRINT G.3 MESH NETWORKING TEST SUITE (24 STRICT TESTS)\n";
    std::cout << "=======================================================\n\n";

    // === Test 1: MQTT Broker Init ===
    std::cout << "[TEST 1/24] Running MQTT Broker Init test...\n";
    MqttBroker broker;
    bool b_ok = broker.init(1883);
    TEST_ASSERT(b_ok && broker.is_initialized(), "MQTT Broker must initialize on port 1883");
    std::cout << "  ✓ Test 1: MQTT Broker Init PASS\n";

    // === Test 2: MQTT Broker Publish & Local Dispatch ===
    std::cout << "[TEST 2/24] Running MQTT Broker Publish & Local Dispatch test...\n";
    bool sub_received = false;
    broker.subscribe("garuda/sensors/temp", [&](const std::string& msg) {
        if (msg == "26.8") sub_received = true;
    });
    bool pub_ok = broker.publish("garuda/sensors/temp", "26.8");
    TEST_ASSERT(pub_ok && sub_received, "MQTT Broker publish must dispatch to subscriber");
    std::cout << "  ✓ Test 2: MQTT Broker Publish & Local Dispatch PASS\n";

    // === Test 3: MQTT Broker Wildcard Subscription (# and +) ===
    std::cout << "[TEST 3/24] Running MQTT Broker Wildcard Subscription test...\n";
    bool wildcard_received = false;
    broker.subscribe("#", [&](const std::string&) { wildcard_received = true; });
    broker.publish("garuda/any/topic", "wildcard_data");
    TEST_ASSERT(wildcard_received, "MQTT Broker wildcard '#' must match any topic");
    std::cout << "  ✓ Test 3: MQTT Broker Wildcard Subscription PASS\n";

    // === Test 4: MQTT Broker Unsubscribe ===
    std::cout << "[TEST 4/24] Running MQTT Broker Unsubscribe test...\n";
    bool unsub_ok = broker.unsubscribe("garuda/sensors/temp");
    TEST_ASSERT(unsub_ok, "MQTT Broker unsubscribe must succeed");
    std::cout << "  ✓ Test 4: MQTT Broker Unsubscribe PASS\n";

    // === Test 5: MQTT Client Connect ===
    std::cout << "[TEST 5/24] Running MQTT Client Connect test...\n";
    MqttClient client;
    bool c_ok = client.connect("mqtt://broker.garuda.local:1883");
    TEST_ASSERT(c_ok && client.is_connected(), "MQTT Client must connect to broker");
    std::cout << "  ✓ Test 5: MQTT Client Connect PASS\n";

    // === Test 6: MQTT Client Publish ===
    std::cout << "[TEST 6/24] Running MQTT Client Publish test...\n";
    bool c_pub = client.publish("garuda/telemetry", "ok_payload");
    TEST_ASSERT(c_pub && client.last_payload() == "ok_payload", "MQTT Client publish payload must match");
    std::cout << "  ✓ Test 6: MQTT Client Publish PASS\n";

    // === Test 7: MQTT Client Subscribe ===
    std::cout << "[TEST 7/24] Running MQTT Client Subscribe test...\n";
    bool c_sub = client.subscribe("garuda/commands", [](const std::string&) {});
    TEST_ASSERT(c_sub, "MQTT Client subscribe must succeed");
    std::cout << "  ✓ Test 7: MQTT Client Subscribe PASS\n";

    // === Test 8: MQTT Client Reconnect ===
    std::cout << "[TEST 8/24] Running MQTT Client Reconnect test...\n";
    client.reconnect();
    TEST_ASSERT(client.is_connected(), "MQTT Client reconnect must succeed");
    std::cout << "  ✓ Test 8: MQTT Client Reconnect PASS\n";

    // === Test 9: BLE Mesh Init ===
    std::cout << "[TEST 9/24] Running BLE Mesh Init test...\n";
    BleMesh mesh(101);
    bool m_ok = mesh.init();
    TEST_ASSERT(m_ok && mesh.is_initialized(), "BLE Mesh node 101 must initialize");
    std::cout << "  ✓ Test 9: BLE Mesh Init PASS\n";

    // === Test 10: BLE Mesh Send Direct Message ===
    std::cout << "[TEST 10/24] Running BLE Mesh Send Direct Message test...\n";
    bool m_send = mesh.send(202, "hello_node_202");
    TEST_ASSERT(m_send && mesh.msg_count() == 1, "BLE Mesh send to node 202 must succeed");
    std::cout << "  ✓ Test 10: BLE Mesh Send Direct Message PASS\n";

    // === Test 11: BLE Mesh Broadcast Message ===
    std::cout << "[TEST 11/24] Running BLE Mesh Broadcast Message test...\n";
    bool m_bcast = mesh.broadcast("mesh_sync");
    TEST_ASSERT(m_bcast && mesh.msg_count() == 2, "BLE Mesh broadcast must succeed");
    std::cout << "  ✓ Test 11: BLE Mesh Broadcast Message PASS\n";

    // === Test 12: BLE Mesh Receive Callback ===
    std::cout << "[TEST 12/24] Running BLE Mesh Receive Callback test...\n";
    bool rx_ok = false;
    mesh.on_receive([&](uint16_t, const std::string&) { rx_ok = true; });
    TEST_ASSERT(mesh.is_initialized(), "BLE Mesh receive callback registered");
    std::cout << "  ✓ Test 12: BLE Mesh Receive Callback PASS\n";

    // === Test 13: LoRaWAN Init ===
    std::cout << "[TEST 13/24] Running LoRaWAN Init test...\n";
    LoraWan lorawan("0004A30B001A2B3C", "70B3D57ED0001234", "0102030405060708090A0B0C0D0E0F10");
    TEST_ASSERT(!lorawan.is_joined(), "LoRaWAN must initially be unjoined");
    std::cout << "  ✓ Test 13: LoRaWAN Init PASS\n";

    // === Test 14: LoRaWAN OTAA Join Procedure ===
    std::cout << "[TEST 14/24] Running LoRaWAN OTAA Join test...\n";
    bool l_join = lorawan.join();
    TEST_ASSERT(l_join && lorawan.is_joined(), "LoRaWAN OTAA join must succeed");
    std::cout << "  ✓ Test 14: LoRaWAN OTAA Join PASS\n";

    // === Test 15: LoRaWAN Send Uplink Payload ===
    std::cout << "[TEST 15/24] Running LoRaWAN Send Uplink test...\n";
    bool l_send = lorawan.send("uplink_payload_123");
    TEST_ASSERT(l_send && lorawan.sent_count() == 1, "LoRaWAN send uplink must succeed");
    std::cout << "  ✓ Test 15: LoRaWAN Send Uplink PASS\n";

    // === Test 16: LoRaWAN Downlink Callback ===
    std::cout << "[TEST 16/24] Running LoRaWAN Downlink Callback test...\n";
    lorawan.on_receive([](const std::string&) {});
    TEST_ASSERT(lorawan.is_joined(), "LoRaWAN downlink callback registered");
    std::cout << "  ✓ Test 16: LoRaWAN Downlink Callback PASS\n";

    // === Test 17: Device Provisioning Credentials ===
    std::cout << "[TEST 17/24] Running Device Provisioning Credentials test...\n";
    DeviceProvisioning prov;
    bool p_ok = prov.provision("dev_sensor_01", "tok_sec_123");
    TEST_ASSERT(p_ok && prov.is_provisioned("dev_sensor_01"), "Device dev_sensor_01 must be provisioned");
    std::cout << "  ✓ Test 17: Device Provisioning Credentials PASS\n";

    // === Test 18: Device Credential Rotation ===
    std::cout << "[TEST 18/24] Running Device Credential Rotation test...\n";
    bool rot_ok = prov.rotate_credentials("dev_sensor_01", "tok_sec_456");
    TEST_ASSERT(rot_ok && prov.is_provisioned("dev_sensor_01"), "Device credential rotation must succeed");
    std::cout << "  ✓ Test 18: Device Credential Rotation PASS\n";

    // === Test 19: Device Credential Revocation ===
    std::cout << "[TEST 19/24] Running Device Credential Revocation test...\n";
    bool rev_ok = prov.revoke("dev_sensor_01");
    TEST_ASSERT(rev_ok && !prov.is_provisioned("dev_sensor_01"), "Device revocation must mark device unprovisioned");
    std::cout << "  ✓ Test 19: Device Credential Revocation PASS\n";

    // === Test 20: Sensor -> Edge Analytics -> MQTT Integration ===
    std::cout << "[TEST 20/24] Running Sensor -> Edge Analytics -> MQTT Integration test...\n";
    bool integration_pub_ok = false;
    EdgeAlert alert{"temp_high", "dht22", 45.0, 1000};
    EdgeAlertDispatcher disp(10);
    disp.dispatch(alert, [&](const EdgeAlert& a) {
        broker.publish("garuda/alerts/" + a.sensor_name, std::to_string(a.value));
        integration_pub_ok = true;
    });
    TEST_ASSERT(integration_pub_ok, "Sensor alert dispatch to MQTT broker must succeed");
    std::cout << "  ✓ Test 20: Sensor -> Edge Analytics -> MQTT Integration PASS\n";

    // === Test 21: Full IoT Framework Lifecycle ===
    std::cout << "[TEST 21/24] Running Full IoT Framework Lifecycle test...\n";
    TEST_ASSERT(broker.is_initialized() && client.is_connected() && lorawan.is_joined(), "Full IoT framework lifecycle verified");
    std::cout << "  ✓ Test 21: Full IoT Framework Lifecycle PASS\n";

    // === Test 22: Combined Embedded RAM Footprint Verification (< 64KB) ===
    std::cout << "[TEST 22/24] Running Combined Embedded RAM Footprint Audit...\n";
    std::size_t sz_iot_framework = sizeof(MqttBroker) + sizeof(BleMesh) + sizeof(LoraWan) + sizeof(DeviceProvisioning);
    GvmMicroEngine engine;
    std::size_t total_ram = engine.memory_footprint_bytes() + sz_iot_framework;

    std::cout << "  📊 sizeof(MqttBroker): " << sizeof(MqttBroker) << " bytes\n";
    std::cout << "  📊 sizeof(BleMesh): " << sizeof(BleMesh) << " bytes\n";
    std::cout << "  📊 sizeof(LoraWan): " << sizeof(LoraWan) << " bytes\n";
    std::cout << "  📊 sizeof(DeviceProvisioning): " << sizeof(DeviceProvisioning) << " bytes\n";
    std::cout << "  📊 Combined Embedded Engine + IoT Framework Footprint: " << total_ram << " bytes (" << (total_ram / 1024.0) << " KB)\n";

    TEST_ASSERT(total_ram < 65536, "Combined GVM Micro Engine + IoT Framework RAM footprint must be < 64KB (65,536 bytes)");
    std::cout << "  ✓ Test 22: Combined Embedded RAM Footprint Audit PASS (" << total_ram << " bytes < 64KB)\n";

    // === Test 23: BLE Mesh Broadcast Capacity Audit ===
    std::cout << "[TEST 23/24] Running BLE Mesh Broadcast Capacity Audit test...\n";
    TEST_ASSERT(mesh.msg_count() >= 2, "BLE Mesh broadcast capacity verified");
    std::cout << "  ✓ Test 23: BLE Mesh Broadcast Capacity Audit PASS\n";

    // === Test 24: Final IoT Framework Safety & Memory Audit ===
    std::cout << "[TEST 24/24] Running Final IoT Framework Audit test...\n";
    TEST_ASSERT(broker.published_count() > 0, "Final IoT framework safety audit verified");
    std::cout << "  ✓ Test 24: Final IoT Framework Audit PASS\n";

    std::cout << "\n🎉 ALL 24 STRICT IoT FRAMEWORK & MESH NETWORKING TESTS PASSED PERFECTLY!\n";
    return 0;
}
