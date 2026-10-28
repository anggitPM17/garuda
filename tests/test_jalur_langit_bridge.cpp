#include "../include/garuda/jalur_langit/jalur_langit_bridge.hpp"
#include "../include/garuda/nusantara/nusantara_broker.hpp"
#include <iostream>
#include <filesystem>
#include <cstdlib>
#include <string>
#include <system_error>
#include <thread>

using namespace garuda::jalur_langit;
using namespace garuda::nusantara;

#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

int main() {
    std::cout << "========================================\n";
    std::cout << "JALUR LANGIT BRIDGE SECURITY TESTS\n";
    std::cout << "========================================\n\n";

    std::string test_dir = "./test_bridge_data";
    std::error_code ec;
    std::filesystem::remove_all(test_dir, ec);

    {
        JalurLangitBridge bridge(test_dir);
        bridge.set_auth_token("SECRET_TOKEN_123");
        bridge.start();

        // Counter response tracking
        std::string last_status;
        std::string last_message;

        NusantaraBroker::get_instance().register_entity(
            EntityTarget::AUTOPILOT_AI,
            [&last_status, &last_message](const NusantaraMessage& msg) {
                auto s_it = msg.payload.find("status");
                if (s_it != msg.payload.end()) last_status = s_it->second;
                auto m_it = msg.payload.find("message");
                if (m_it != msg.payload.end()) last_message = m_it->second;
            }
        );

        // Test 1: Auth Rejection (Tanpa Token)
        std::cout << "[Test 1] Auth Rejection (Tanpa Token)...\n";
        NusantaraMessage msg1;
        msg1.message_id = "m1";
        msg1.sender = EntityTarget::AUTOPILOT_AI;
        msg1.receiver = EntityTarget::JALUR_LANGIT;
        msg1.action = NusantaraAction::JALUR_LANGIT_CREATE_TABLE;
        msg1.payload["table_name"] = "users";

        NusantaraBroker::get_instance().send_message(msg1);
        NusantaraBroker::get_instance().process_queue();

        CHECK(last_status == "401", "Pesan tanpa token harus ditolak dengan HTTP 401");
        std::cout << "PASS: Auth Rejection bekerja (401).\n\n";

        // Test 2: Auth Valid
        std::cout << "[Test 2] Auth Valid...\n";
        NusantaraMessage msg2 = msg1;
        msg2.message_id = "m2";
        msg2.payload["auth_token"] = "SECRET_TOKEN_123";

        NusantaraBroker::get_instance().send_message(msg2);
        NusantaraBroker::get_instance().process_queue();

        CHECK(last_status == "200", "Pesan dengan token valid harus sukses 200");
        std::cout << "PASS: Auth Valid bekerja (200).\n\n";

        // Test 3: Invalid Table Name (DROP)
        std::cout << "[Test 3] Invalid Table Name (Sanitization)...\n";
        NusantaraMessage msg3 = msg2;
        msg3.message_id = "m3";
        msg3.payload["table_name"] = "DROP TABLE users;";

        NusantaraBroker::get_instance().send_message(msg3);
        NusantaraBroker::get_instance().process_queue();

        CHECK(last_status == "400", "Table name 'DROP TABLE users;' harus ditolak (400)");
        std::cout << "PASS: Invalid Table Name ditolak (400).\n\n";

        // Test 4: Rate Limiting
        std::cout << "[Test 4] Rate Limiting Check...\n";
        bool rate_limit_hit = false;
        for (int i = 0; i < 110; ++i) {
            NusantaraMessage msg_rl = msg2;
            msg_rl.message_id = "m_rl_" + std::to_string(i);
            NusantaraBroker::get_instance().send_message(msg_rl);
            NusantaraBroker::get_instance().process_queue();
            if (last_status == "429") {
                rate_limit_hit = true;
            }
        }
        CHECK(rate_limit_hit, "Rate limit > 100 req/sec harus memicu HTTP 429");
        std::cout << "PASS: Rate Limiting bekerja (429).\n\n";

        // Tunggu 1100ms agar rate limit window ter-reset
        std::this_thread::sleep_for(std::chrono::milliseconds(1100));

        // Test 5: Query Execution (INSERT + SELECT)
        std::cout << "[Test 5] Query Execution (INSERT + SELECT)...\n";
        NusantaraMessage msg_ins = msg2;
        msg_ins.message_id = "m_ins";
        msg_ins.action = NusantaraAction::JALUR_LANGIT_INSERT;
        msg_ins.payload["table_name"] = "users";
        msg_ins.payload["query"] = "INSERT INTO users VALUES (1, 'Budi', 25)";

        NusantaraBroker::get_instance().send_message(msg_ins);
        NusantaraBroker::get_instance().process_queue();
        CHECK(last_status == "200", "Query INSERT harus sukses");

        NusantaraMessage msg_sel = msg2;
        msg_sel.message_id = "m_sel";
        msg_sel.action = NusantaraAction::JALUR_LANGIT_QUERY;
        msg_sel.payload["query"] = "SELECT * FROM users";

        NusantaraBroker::get_instance().send_message(msg_sel);
        NusantaraBroker::get_instance().process_queue();
        CHECK(last_status == "200", "Query SELECT harus sukses");
        std::cout << "PASS: Query Execution bekerja (INSERT & SELECT 200).\n\n";

        // Test 6: Forbidden Action Whitelist
        std::cout << "[Test 6] Forbidden Action Whitelist...\n";
        NusantaraMessage msg_forbid = msg2;
        msg_forbid.message_id = "m_forbid";
        msg_forbid.action = NusantaraAction::UPDATE_CODE;

        NusantaraBroker::get_instance().send_message(msg_forbid);
        NusantaraBroker::get_instance().process_queue();
        CHECK(last_status == "403", "Aksi non-whitelist ke Jalur Langit harus ditolak (403)");
        std::cout << "PASS: Action Whitelist bekerja (403).\n\n";

        // Test 7: Payload Too Large (>1MB)
        std::cout << "[Test 7] Payload Too Large Check...\n";
        NusantaraMessage msg_big = msg2;
        msg_big.message_id = "m_big";
        msg_big.payload["huge"] = std::string(1050000, 'X');

        NusantaraBroker::get_instance().send_message(msg_big);
        NusantaraBroker::get_instance().process_queue();
        CHECK(last_status == "413", "Payload > 1MB harus ditolak dengan HTTP 413");
        std::cout << "PASS: Payload Too Large Check bekerja (413).\n\n";

        // Test 8: SQL Injection Attempt
        std::cout << "[Test 8] SQL Injection Prevention...\n";
        NusantaraMessage msg_sqli = msg2;
        msg_sqli.message_id = "m_sqli";
        msg_sqli.action = NusantaraAction::JALUR_LANGIT_QUERY;
        msg_sqli.payload["query"] = "SELECT * FROM users; DROP TABLE users";

        NusantaraBroker::get_instance().send_message(msg_sqli);
        NusantaraBroker::get_instance().process_queue();
        CHECK(last_status == "400", "SQL Injection dengan semicolon harus ditolak (400)");
        std::cout << "PASS: SQL Injection Prevention bekerja (400).\n\n";

        // Test 9: Stats Monitoring
        std::cout << "[Test 9] Stats Monitoring Check...\n";
        auto stats = bridge.get_stats();
        CHECK(stats.commands_received > 0, "commands_received harus > 0");
        CHECK(stats.commands_rejected > 0, "commands_rejected harus > 0");
        std::cout << "PASS: Stats Monitoring terverifikasi.\n\n";

        // Test 10: Zero Trust for Autopilot AI
        std::cout << "[Test 10] Zero-Trust for Autopilot AI...\n";
        NusantaraMessage msg_no_token = msg1; // Autopilot tanpa token
        msg_no_token.sender = EntityTarget::AUTOPILOT_AI;

        NusantaraBroker::get_instance().send_message(msg_no_token);
        NusantaraBroker::get_instance().process_queue();
        CHECK(last_status == "401", "Zero trust: Autopilot AI tanpa token tetap ditolak (401)");
        std::cout << "PASS: Zero-Trust for Autopilot AI terverifikasi (401).\n\n";

        bridge.stop();
    }

    std::filesystem::remove_all(test_dir, ec);

    std::cout << "========================================\n";
    std::cout << "JALUR LANGIT BRIDGE SECURITY SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
