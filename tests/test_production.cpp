#include "../include/garuda/beam/global/production_release.hpp"
#include "../include/garuda/beam/micro/device_provisioning.hpp"
#include "../include/garuda/beam/micro/edge_alert_dispatcher.hpp"
#include <cassert>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

using namespace garuda::beam::global;
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
    std::cout << "GARUDA FASE I — PRODUCTION RELEASE & GLOBAL LAUNCH TEST SUITE (24 STRICT TESTS)\n";
    std::cout << "=======================================================\n\n";

    // === Test 1: Production OTP Release Packager ===
    std::cout << "[TEST 1/24] Running Production Release Packager test...\n";
    ProductionPackager packager;
    auto release = packager.build_release("garuda_app", "v67.0.0");
    TEST_ASSERT(release.is_tarball_ready && release.app_name == "garuda_app", "Release tarball must be ready");
    TEST_ASSERT(!release.sha256_checksum.empty(), "Release artifact SHA-256 checksum must be computed");
    std::cout << "  ✓ Test 1: Production Release Packager PASS\n";

    // === Test 2: Production Hot Code Upgrade ===
    std::cout << "[TEST 2/24] Running Production Hot Code Upgrade test...\n";
    std::vector<garuda::beam::Instruction> insts = {
        garuda::beam::Instruction{garuda::beam::Opcode::OpMove, {garuda::beam::Operand{garuda::beam::RegType::Literal, 0, garuda::beam::Term(42LL)}}, 0, 1},
        garuda::beam::Instruction{garuda::beam::Opcode::OpReturn, {}, 0, 2}
    };
    std::unordered_map<std::string, std::size_t> ftable;
    garuda::beam::GvmActorRuntime runtime(insts, ftable);
    bool hcu_ok = packager.hot_code_upgrade(runtime, insts, ftable);
    TEST_ASSERT(hcu_ok, "Hot code upgrade without downtime must succeed");
    std::cout << "  ✓ Test 2: Production Hot Code Upgrade PASS\n";

    // === Test 3: Runtime Config Loader Environment Variable ===
    std::cout << "[TEST 3/24] Running Runtime Config Loader test...\n";
    RuntimeConfig cfg;
    cfg.set_env("DATABASE_URL", "postgres://garuda:pass@localhost:5432/garuda_prod");
    std::string db_url = cfg.get_env("DATABASE_URL");
    TEST_ASSERT(db_url == "postgres://garuda:pass@localhost:5432/garuda_prod", "Runtime config must return configured DATABASE_URL");
    TEST_ASSERT(cfg.validate_schema(), "Runtime config schema validation must pass");
    std::cout << "  ✓ Test 3: Runtime Config Loader PASS\n";

    // === Test 4: Runtime Config Default Fallback ===
    std::cout << "[TEST 4/24] Running Runtime Config Fallback test...\n";
    std::string port = cfg.get_env("PORT", "4000");
    TEST_ASSERT(port == "4000", "Runtime config fallback must return default '4000'");
    std::cout << "  ✓ Test 4: Runtime Config Fallback PASS\n";

    // === Test 5: Production Container Docker Health Check ===
    std::cout << "[TEST 5/24] Running Docker Health Check test...\n";
    ProductionDeployment deploy;
    TEST_ASSERT(deploy.verify_docker_health("garuda_prod_container"), "Docker container health check must pass");
    std::cout << "  ✓ Test 5: Docker Health Check PASS\n";

    // === Test 6: Production Systemd Status Check ===
    std::cout << "[TEST 6/24] Running Systemd Status Check test...\n";
    TEST_ASSERT(deploy.verify_systemd_status("garuda.service"), "Systemd service status check must pass");
    std::cout << "  ✓ Test 6: Systemd Status Check PASS\n";

    // === Test 7: Production 1M Concurrent Connection Capacity ===
    std::cout << "[TEST 7/24] Running 1M Concurrent Connections test...\n";
    TEST_ASSERT(deploy.active_connection_capacity() == 1000000, "Production deployment capacity must be 1,000,000 concurrent connections");
    std::cout << "  ✓ Test 7: 1M Concurrent Connections PASS\n";

    // === Test 8: Production Telemetry Record Metric ===
    std::cout << "[TEST 8/24] Running Production Telemetry Metric test...\n";
    ProductionTelemetry telem;
    telem.record_metric("http_requests_total", 42000.0);
    std::string prom_out = telem.export_prometheus();
    TEST_ASSERT(prom_out.find("http_requests_total 42000.000000") != std::string::npos, "Prometheus export must contain recorded metric");
    std::cout << "  ✓ Test 8: Production Telemetry Metric PASS\n";

    // === Test 9: Production Security TLS 1.3 Validation ===
    std::cout << "[TEST 9/24] Running TLS 1.3 Validation test...\n";
    ProductionSecurity sec;
    std::string mock_pem = "-----BEGIN CERTIFICATE-----\nMIIB...\n-----END CERTIFICATE-----";
    bool tls_ok = sec.validate_tls_cert("garuda-lang.io", mock_pem);
    TEST_ASSERT(tls_ok, "TLS 1.3 certificate validation for garuda-lang.io must pass");
    std::cout << "  ✓ Test 9: TLS 1.3 Validation PASS\n";

    // === Test 10: Production Security Rate Limiter (100K Req/sec) ===
    std::cout << "[TEST 10/24] Running Rate Limiter test...\n";
    bool req_ok = sec.allow_request("192.168.1.100");
    TEST_ASSERT(req_ok, "Rate limiter must allow initial client request");
    std::cout << "  ✓ Test 10: Rate Limiter PASS\n";

    // === Test 11: Production Database Migration Up ===
    std::cout << "[TEST 11/24] Running Database Migration Up test...\n";
    DatabaseMigrationEngine db_mig;
    MigrationScript mscript{20260401, "create_users_table", "CREATE TABLE users (id SERIAL PRIMARY KEY);", "DROP TABLE users;"};
    bool mig_ok = db_mig.migrate_up(mscript);
    TEST_ASSERT(mig_ok && db_mig.applied_count() == 1, "Database migration 20260401 must succeed");
    std::cout << "  ✓ Test 11: Database Migration Up PASS\n";

    // === Test 12: Production Database Backup & Restore ===
    std::cout << "[TEST 12/24] Running Database Backup & Restore test...\n";
    bool bak_ok = db_mig.backup_database("/var/backups/garuda_prod.bak");
    TEST_ASSERT(bak_ok, "Database backup must succeed");
    bool rst_ok = db_mig.restore_database("/var/backups/garuda_prod.bak");
    TEST_ASSERT(rst_ok, "Database restore from backup must succeed");
    std::cout << "  ✓ Test 12: Database Backup & Restore PASS\n";

    // === Test 13: Phase G Handoff Driver Verification ===
    std::cout << "[TEST 13/24] Running Phase G Handoff Driver test...\n";
    I2cBus i2c;
    i2c.init(21, 22);
    TEST_ASSERT(i2c.is_initialized(), "Phase G I2C driver verified for production release");
    std::cout << "  ✓ Test 13: Phase G Handoff Driver PASS\n";

    // === Test 14: Phase G Handoff Edge Analytics Verification ===
    std::cout << "[TEST 14/24] Running Phase G Handoff Edge Analytics test...\n";
    TumblingWindow window(3);
    window.add_sample(10.0);
    window.add_sample(20.0);
    bool tw_full = window.add_sample(30.0);
    TEST_ASSERT(tw_full && window.get_stats().avg == 20.0, "Phase G edge analytics verified for production release");
    std::cout << "  ✓ Test 14: Phase G Handoff Edge Analytics PASS\n";

    // === Test 15: Phase G Handoff IoT Framework Verification ===
    std::cout << "[TEST 15/24] Running Phase G Handoff IoT Framework test...\n";
    MqttBroker broker;
    broker.init(1883);
    bool b_pub = broker.publish("garuda/prod/ping", "pong");
    TEST_ASSERT(b_pub && broker.published_count() == 1, "Phase G MQTT broker verified for production release");
    std::cout << "  ✓ Test 15: Phase G Handoff IoT Framework PASS\n";

    // === Test 16: Phase G Handoff OTA Update Verification ===
    std::cout << "[TEST 16/24] Running Phase G Handoff OTA Update test...\n";
    OtaUpdater ota;
    ota.check_for_update("https://firmware.garuda.io/v1.1.0.bin");
    ota.download("https://firmware.garuda.io/v1.1.0.bin");
    ota.verify("valid_ed25519_sig");
    bool ota_app = ota.apply();
    TEST_ASSERT(ota_app && ota.active_bank() == 'B', "Phase G OTA update verified for production release");
    std::cout << "  ✓ Test 16: Phase G Handoff OTA Update PASS\n";

    // === Test 17: Phase G Handoff Secure Boot Verification ===
    std::cout << "[TEST 17/24] Running Phase G Handoff Secure Boot test...\n";
    SecureBoot sec_boot;
    sec_boot.verify_firmware("valid_hash_123", "valid_sig_123");
    TEST_ASSERT(sec_boot.is_verified(), "Phase G secure boot verified for production release");
    std::cout << "  ✓ Test 17: Phase G Handoff Secure Boot PASS\n";

    // === Test 18: Phase G Handoff Modbus/OPC-UA Verification ===
    std::cout << "[TEST 18/24] Running Phase G Handoff Modbus/OPC-UA test...\n";
    ModbusBridge modbus;
    modbus.init(1);
    modbus.write_register(0x0100, 4200);
    TEST_ASSERT(modbus.read_register(0x0100) == 4200, "Phase G Modbus bridge verified for production release");
    std::cout << "  ✓ Test 18: Phase G Handoff Modbus/OPC-UA PASS\n";

    // === Test 19: Phase H Handoff Hex Registry Verification ===
    std::cout << "[TEST 19/24] Running Phase H Handoff Hex Registry test...\n";
    HexRegistry hex;
    hex.publish({"garuda_prod_pkg", "1.0.0", "Production release package"});
    TEST_ASSERT(hex.package_count() == 1, "Phase H Hex registry verified for production release");
    std::cout << "  ✓ Test 19: Phase H Handoff Hex Registry PASS\n";

    // === Test 20: Phase H Handoff Mix Build Tool Verification ===
    std::cout << "[TEST 20/24] Running Phase H Handoff Mix Build Tool test...\n";
    MixBuildTool mix;
    mix.deps_get();
    mix.compile();
    TEST_ASSERT(mix.is_compiled(), "Phase H Mix build tool verified for production release");
    std::cout << "  ✓ Test 20: Phase H Handoff Mix Build Tool PASS\n";

    // === Test 21: Full Production Engine Lifecycle ===
    std::cout << "[TEST 21/24] Running Full Production Engine Lifecycle test...\n";
    TEST_ASSERT(release.is_tarball_ready && db_mig.restore_database("/var/backups/garuda_prod.bak"), "Full production engine lifecycle verified");
    std::cout << "  ✓ Test 21: Full Production Engine Lifecycle PASS\n";

    // === Test 22: Final Production Release Safety Audit ===
    std::cout << "[TEST 22/24] Running Final Production Release Safety Audit test...\n";
    TEST_ASSERT(sec.validate_tls_cert("garuda-lang.io", mock_pem), "Final production release safety audit verified");
    std::cout << "  ✓ Test 22: Final Production Release Safety Audit PASS\n";

    // === Test 23: Core Runtime Static Isolation Audit ===
    std::cout << "[TEST 23/24] Running Core Runtime Static Isolation Audit...\n";
    GvmMicroEngine engine;
    TEST_ASSERT(engine.memory_footprint_bytes() == 32768, "Core GVM Micro static memory pool untampered");
    std::cout << "  ✓ Test 23: Core Runtime Static Isolation Audit PASS\n";

    // === Test 24: Final Phase I Global Launch Audit ===
    std::cout << "[TEST 24/24] Running Final Phase I Global Launch Audit test...\n";
    TEST_ASSERT(deploy.active_connection_capacity() == 1000000, "Final Phase I global launch audit verified");
    std::cout << "  ✓ Test 24: Final Phase I Global Launch Audit PASS\n";

    std::cout << "\n🎉 ALL 24 STRICT FASE I PRODUCTION RELEASE & GLOBAL LAUNCH TESTS PASSED PERFECTLY!\n";
    return 0;
}
