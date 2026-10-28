#include "../include/garuda/autopilot/autopilot_ai.hpp"
#include "../include/garuda/autopilot/mock_llm_client.hpp"
#include <iostream>
#include <filesystem>
#include <cstdlib>
#include <thread>
#include <system_error>

using namespace garuda::autopilot;
using namespace garuda::jalur_langit;

#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

int main() {
    std::cout << "========================================\n";
    std::cout << "FASE 5.3 - AUTOPILOT AI INTEGRATION TESTS\n";
    std::cout << "========================================\n\n";

    std::string test_dir = "./test_autopilot_data";
    std::error_code ec;
    std::filesystem::remove_all(test_dir, ec);

    {
        // Setup dependencies
        JalurLangitUltimate db(test_dir);
        AdminGate gate;
        gate.set_admin_token("SECRET");

        // HealthMonitor (default healthy)
        HealthMonitor health([]() {
            HealthSnapshot s; s.db_ok = true; s.ram_usage_mb = 100; s.latency_ms = 10; return s;
        });

        // MockLLM (selalu sukses)
        MockLLMClient mock_llm(MockLLMClient::Behavior::SUCCESS, "fun fixed() { return 1 }");
        AICodeFixer fixer(mock_llm);

        AutopilotAI autopilot(gate, health, fixer, db);
        const std::string err = "NullPointerException";
        const std::string ctx = "class A {}";
        const std::string file = "A.kt";

        // Test 1: MANUAL mode -> diblokir
        std::cout << "[Test 1] MANUAL mode...\n";
        {
            gate.set_mode(AutopilotMode::MANUAL, "SECRET");
            auto res = autopilot.handle_error(err, ctx, file);
            CHECK(!res.executed, "Harus diblokir di MANUAL");
            CHECK(res.reason == "gate_blocked_not_autopilot", "Reason harus gate_blocked");
            CHECK(mock_llm.call_count() == 0, "LLM tidak boleh dipanggil");
        }
        std::cout << "PASS\n\n";

        // Test 2: STOP mode -> diblokir
        std::cout << "[Test 2] STOP mode...\n";
        {
            gate.set_mode(AutopilotMode::STOP, "SECRET");
            auto res = autopilot.handle_error(err, ctx, file);
            CHECK(!res.executed, "Harus diblokir di STOP");
            CHECK(mock_llm.call_count() == 0, "LLM tidak boleh dipanggil");
        }
        std::cout << "PASS\n\n";

        // Test 3: AUTOPILOT mode + Unhealthy -> diblokir
        std::cout << "[Test 3] AUTOPILOT + Unhealthy...\n";
        {
            gate.set_mode(AutopilotMode::AUTOPILOT, "SECRET");
            HealthMonitor sick_health([]() {
                HealthSnapshot s; s.db_ok = false; return s;
            });
            AutopilotAI sick_autopilot(gate, sick_health, fixer, db);

            auto res = sick_autopilot.handle_error(err, ctx, file);
            CHECK(!res.executed, "Harus diblokir karena sakit");
            CHECK(res.reason == "health_unhealthy", "Reason harus health_unhealthy");
            CHECK(mock_llm.call_count() == 0, "LLM tidak boleh dipanggil");
        }
        std::cout << "PASS\n\n";

        // Test 4: AUTOPILOT mode + Healthy -> Sukses + Audit Log
        std::cout << "[Test 4] AUTOPILOT + Healthy -> Sukses & Log...\n";
        {
            gate.set_mode(AutopilotMode::AUTOPILOT, "SECRET");
            auto res = autopilot.handle_error(err, ctx, file);

            CHECK(res.executed, "Harus dieksekusi");
            CHECK(res.fix_success, "Fix harus sukses");
            CHECK(mock_llm.call_count() == 1, "LLM dipanggil 1x");

            // Verifikasi Audit Log masuk ke Jalur Langit
            auto logs = db.get_storage().scan("audit_log");
            CHECK(logs.size() == 1, "Harus ada 1 baris audit_log");

            auto& row = logs[0];
            CHECK(std::get<std::string>(row.columns["target_file"]) == file, "File harus sesuai");
            CHECK(std::get<bool>(row.columns["success"]) == true, "Success harus true");
            CHECK(std::get<std::string>(row.columns["llm_model"]) == "MockLLM", "Model harus MockLLM");
        }
        std::cout << "PASS\n\n";
    }

    std::filesystem::remove_all(test_dir, ec);

    std::cout << "========================================\n";
    std::cout << "AUTOPILOT AI INTEGRATION SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
