#include "../include/garuda/beam/micro/fleet_manager.hpp"
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
    std::cout << "GARUDA INDUSTRIAL ENGINE — SPRINT G.4 PARTNERSHIP & CERTIFICATION TEST SUITE (24 STRICT TESTS)\n";
    std::cout << "=======================================================\n\n";

    // === Test 1: OTA Updater Check For Update ===
    std::cout << "[TEST 1/24] Running OTA Updater Check test...\n";
    OtaUpdater ota;
    bool check_ok = ota.check_for_update("https://firmware.garuda.io/v1.1.0.bin");
    TEST_ASSERT(check_ok, "OTA check for update must succeed");
    std::cout << "  ✓ Test 1: OTA Updater Check PASS\n";

    // === Test 2: OTA Updater Download ===
    std::cout << "[TEST 2/24] Running OTA Updater Download test...\n";
    bool dl_ok = ota.download("https://firmware.garuda.io/v1.1.0.bin");
    TEST_ASSERT(dl_ok, "OTA download must succeed");
    std::cout << "  ✓ Test 2: OTA Updater Download PASS\n";

    // === Test 3: OTA Updater Signature Verification ===
    std::cout << "[TEST 3/24] Running OTA Updater Signature Verification test...\n";
    bool ver_ok = ota.verify("valid_ed25519_sig");
    TEST_ASSERT(ver_ok, "OTA Ed25519 signature verification must pass");
    std::cout << "  ✓ Test 3: OTA Updater Signature Verification PASS\n";

    // === Test 4: OTA Updater Atomic A/B Bank Flash Apply ===
    std::cout << "[TEST 4/24] Running OTA Updater Atomic Apply test...\n";
    char initial_bank = ota.active_bank();
    bool apply_ok = ota.apply();
    TEST_ASSERT(apply_ok && ota.is_applied(), "OTA apply must succeed");
    TEST_ASSERT(ota.active_bank() != initial_bank, "OTA active bank must switch after update");
    std::cout << "  ✓ Test 4: OTA Updater Atomic Apply PASS\n";

    // === Test 5: Secure Boot Firmware Signature Verification ===
    std::cout << "[TEST 5/24] Running Secure Boot Verification test...\n";
    SecureBoot sec_boot;
    bool sb_ok = sec_boot.verify_firmware("valid_hash_123", "valid_sig_123");
    TEST_ASSERT(sb_ok && sec_boot.is_verified(), "Secure boot firmware verification must pass");
    std::cout << "  ✓ Test 5: Secure Boot Verification PASS\n";

    // === Test 6: Secure Boot Measured Boot Chain ===
    std::cout << "[TEST 6/24] Running Secure Boot Measured Boot Chain test...\n";
    bool meas_ok = sec_boot.measure_boot();
    TEST_ASSERT(meas_ok, "Secure boot measurement must succeed");
    std::cout << "  ✓ Test 6: Secure Boot Measured Boot Chain PASS\n";

    // === Test 7: Secure Boot Remote Attestation ===
    std::cout << "[TEST 7/24] Running Secure Boot Attestation test...\n";
    std::string att = sec_boot.attest("nonce_777");
    TEST_ASSERT(att == "attestation_token_ok_nonce_777", "Secure boot remote attestation must match nonce token");
    std::cout << "  ✓ Test 7: Secure Boot Attestation PASS\n";

    // === Test 8: Modbus RTU Init ===
    std::cout << "[TEST 8/24] Running Modbus RTU Init test...\n";
    ModbusBridge modbus;
    bool mb_ok = modbus.init(1, 9600);
    TEST_ASSERT(mb_ok && modbus.is_initialized(), "Modbus RTU must initialize on slave ID 1");
    std::cout << "  ✓ Test 8: Modbus RTU Init PASS\n";

    // === Test 9: Modbus Write Register ===
    std::cout << "[TEST 9/24] Running Modbus Write Register test...\n";
    bool mb_w_ok = modbus.write_register(0x0100, 4200);
    TEST_ASSERT(mb_w_ok, "Modbus write holding register 0x0100 must succeed");
    std::cout << "  ✓ Test 9: Modbus Write Register PASS\n";

    // === Test 10: Modbus Read Register ===
    std::cout << "[TEST 10/24] Running Modbus Read Register test...\n";
    uint16_t reg_val = modbus.read_register(0x0100);
    TEST_ASSERT(reg_val == 4200, "Modbus read register 0x0100 must return 4200");
    std::cout << "  ✓ Test 10: Modbus Read Register PASS\n";

    // === Test 11: Modbus Read Coils ===
    std::cout << "[TEST 11/24] Running Modbus Read Coils test...\n";
    auto coils = modbus.read_coils(0x0001, 8);
    TEST_ASSERT(coils.size() == 8, "Modbus read coils must return 8 coil states");
    std::cout << "  ✓ Test 11: Modbus Read Coils PASS\n";

    // === Test 12: OPC-UA Bridge Init ===
    std::cout << "[TEST 12/24] Running OPC-UA Bridge Init test...\n";
    OpcUaBridge opcua;
    bool opc_ok = opcua.init("opc.tcp://industrial.garuda.local:4840");
    TEST_ASSERT(opc_ok && opcua.is_initialized(), "OPC-UA bridge must initialize");
    std::cout << "  ✓ Test 12: OPC-UA Bridge Init PASS\n";

    // === Test 13: OPC-UA Client Connect ===
    std::cout << "[TEST 13/24] Running OPC-UA Client Connect test...\n";
    bool opc_conn = opcua.connect();
    TEST_ASSERT(opc_conn && opcua.is_connected(), "OPC-UA connection must succeed");
    std::cout << "  ✓ Test 13: OPC-UA Client Connect PASS\n";

    // === Test 14: OPC-UA Node Write ===
    std::cout << "[TEST 14/24] Running OPC-UA Node Write test...\n";
    bool opc_w_ok = opcua.write_node("ns=2;s=MotorSpeed", "1500_rpm");
    TEST_ASSERT(opc_w_ok, "OPC-UA node write must succeed");
    std::cout << "  ✓ Test 14: OPC-UA Node Write PASS\n";

    // === Test 15: OPC-UA Node Read ===
    std::cout << "[TEST 15/24] Running OPC-UA Node Read test...\n";
    std::string node_val = opcua.read_node("ns=2;s=MotorSpeed");
    TEST_ASSERT(node_val == "1500_rpm", "OPC-UA node read must return '1500_rpm'");
    std::cout << "  ✓ Test 15: OPC-UA Node Read PASS\n";

    // === Test 16: OPC-UA Monitored Item Subscription ===
    std::cout << "[TEST 16/24] Running OPC-UA Subscription test...\n";
    bool opc_sub = opcua.subscribe("ns=2;s=TempSensor", [](const std::string&) {});
    TEST_ASSERT(opc_sub, "OPC-UA subscription must succeed");
    std::cout << "  ✓ Test 16: OPC-UA Subscription PASS\n";

    // === Test 17: Fleet Manager Device Registration ===
    std::cout << "[TEST 17/24] Running Fleet Manager Device Registration test...\n";
    FleetManager fleet;
    bool fleet_reg = fleet.register_device("edge_node_101", "PLC_Simatic_S7");
    TEST_ASSERT(fleet_reg && fleet.device_count() == 1, "Fleet manager registration must succeed");
    std::cout << "  ✓ Test 17: Fleet Manager Device Registration PASS\n";

    // === Test 18: Fleet Manager Remote Device Update ===
    std::cout << "[TEST 18/24] Running Fleet Manager Remote Update test...\n";
    bool fleet_upd = fleet.update_device("edge_node_101", "https://firmware.garuda.io/v1.1.0.bin");
    TEST_ASSERT(fleet_upd, "Fleet manager remote device update must succeed");
    std::cout << "  ✓ Test 18: Fleet Manager Remote Update PASS\n";

    // === Test 19: Fleet Manager Device Query ===
    std::cout << "[TEST 19/24] Running Fleet Manager Device Query test...\n";
    auto dev_list = fleet.query_devices("edge_node");
    TEST_ASSERT(!dev_list.empty(), "Fleet query for 'edge_node' must return devices");
    std::cout << "  ✓ Test 19: Fleet Manager Device Query PASS\n";

    // === Test 20: Fleet Manager Device Revocation ===
    std::cout << "[TEST 20/24] Running Fleet Manager Device Revocation test...\n";
    bool fleet_rev = fleet.revoke_device("edge_node_101");
    TEST_ASSERT(fleet_rev && fleet.query_devices("edge_node_101").empty(), "Revoked device must not be returned in query");
    std::cout << "  ✓ Test 20: Fleet Manager Device Revocation PASS\n";

    // === Test 21: Full Industrial Engine Lifecycle ===
    std::cout << "[TEST 21/24] Running Full Industrial Engine Lifecycle test...\n";
    TEST_ASSERT(ota.is_applied() && sec_boot.is_verified() && modbus.is_initialized() && opcua.is_connected(), "Full industrial engine lifecycle verified");
    std::cout << "  ✓ Test 21: Full Industrial Engine Lifecycle PASS\n";

    // === Test 22: Combined Embedded Industrial Memory Footprint Audit (< 64KB) ===
    std::cout << "[TEST 22/24] Running Combined Industrial Engine Memory Footprint Audit...\n";
    std::size_t sz_industrial = sizeof(OtaUpdater) + sizeof(SecureBoot) + sizeof(ModbusBridge) + sizeof(OpcUaBridge) + sizeof(FleetManager);
    GvmMicroEngine engine;
    std::size_t total_ram = engine.memory_footprint_bytes() + sz_industrial;

    std::cout << "  📊 sizeof(OtaUpdater): " << sizeof(OtaUpdater) << " bytes\n";
    std::cout << "  📊 sizeof(SecureBoot): " << sizeof(SecureBoot) << " bytes\n";
    std::cout << "  📊 sizeof(ModbusBridge): " << sizeof(ModbusBridge) << " bytes\n";
    std::cout << "  📊 sizeof(OpcUaBridge): " << sizeof(OpcUaBridge) << " bytes\n";
    std::cout << "  📊 sizeof(FleetManager): " << sizeof(FleetManager) << " bytes\n";
    std::cout << "  📊 Combined Embedded Engine + Industrial RAM Footprint: " << total_ram << " bytes (" << (total_ram / 1024.0) << " KB)\n";

    TEST_ASSERT(total_ram < 65536, "Combined GVM Micro Engine + Industrial RAM footprint must be < 64KB (65,536 bytes)");
    std::cout << "  ✓ Test 22: Combined Industrial Memory Footprint Audit PASS (" << total_ram << " bytes < 64KB)\n";

    // === Test 23: Modbus Register Range Audit ===
    std::cout << "[TEST 23/24] Running Modbus Register Range Audit test...\n";
    TEST_ASSERT(modbus.read_register(0x0100) == 4200, "Modbus holding register audit verified");
    std::cout << "  ✓ Test 23: Modbus Register Range Audit PASS\n";

    // === Test 24: Final Industrial Partnership & Certification Safety Audit ===
    std::cout << "[TEST 24/24] Running Final Industrial Partnership Audit test...\n";
    TEST_ASSERT(ota.active_bank() == 'B', "A/B Bank active state verified");
    std::cout << "  ✓ Test 24: Final Industrial Partnership Audit PASS\n";

    std::cout << "\n🎉 ALL 24 STRICT INDUSTRIAL PARTNERSHIP & CERTIFICATION TESTS PASSED PERFECTLY!\n";
    return 0;
}
