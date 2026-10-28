#include "../include/garuda/richat/richat_gateway.hpp"
#include "../include/garuda/jalur_langit/jalur_langit_ultimate.hpp"
#include <iostream>
#include <filesystem>
#include <cstdlib>
#include <system_error>

using namespace garuda::richat;
using namespace garuda::nusantara;
using namespace garuda::jalur_langit;

#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

int main() {
    std::cout << "========================================\n";
    std::cout << "R3 - RICHAT OBSERVABILITY TESTS\n";
    std::cout << "========================================\n\n";

    std::string test_dir = "./test_richat_obs_data";
    std::error_code ec;
    std::filesystem::remove_all(test_dir, ec);

    {
        JalurLangitUltimate db(test_dir);
        RichatGateway gateway(db);
        NusantaraBroker::get_instance().start();

        // Dummy receiver agar pesan tidak nyangkut
        NusantaraBroker::get_instance().register_entity(EntityTarget::RICHAT_APP, [](const NusantaraMessage&){});

        // Kirim beberapa request (sukses & gagal)
        NusantaraMessage login_msg;
        login_msg.message_id = "m1";
        login_msg.sender = EntityTarget::RICHAT_APP;
        login_msg.receiver = EntityTarget::JALUR_LANGIT;
        login_msg.action = NusantaraAction::RICHAT_LOGIN;
        login_msg.payload["auth_token"] = "token_abc";
        login_msg.payload["device_id"] = "android_001";

        gateway.handle_message(login_msg); // Sukses
        gateway.handle_message(login_msg); // Sukses

        NusantaraMessage bad_msg;
        bad_msg.message_id = "m2";
        bad_msg.sender = EntityTarget::RICHAT_APP;
        bad_msg.receiver = EntityTarget::JALUR_LANGIT;
        bad_msg.action = NusantaraAction::RICHAT_LOGIN; // Missing fields
        gateway.handle_message(bad_msg); // Error 400

        NusantaraBroker::get_instance().process_queue();

        // Test 1: Metrics Prometheus format
        std::cout << "[Test 1] Metrics Prometheus format...\n";
        std::string metrics = gateway.get_metrics();
        CHECK(metrics.find("richat_requests_total 3") != std::string::npos, "Total request harus 3");
        CHECK(metrics.find("richat_errors_total 1") != std::string::npos, "Total error harus 1");
        CHECK(metrics.find("action=\"RICHAT_LOGIN\"} 3") != std::string::npos, "Action RICHAT_LOGIN harus 3");
        std::cout << "PASS\n\n";

        // Test 2: Tracer JSON export
        std::cout << "[Test 2] Tracer JSON export...\n";
        std::string traces = gateway.get_traces();
        CHECK(traces.find("RICHAT_LOGIN") != std::string::npos, "Trace harus mencatat RICHAT_LOGIN");
        std::cout << "PASS\n\n";

        NusantaraBroker::get_instance().stop();
    }

    std::filesystem::remove_all(test_dir, ec);

    std::cout << "========================================\n";
    std::cout << "R3 - RICHAT OBSERVABILITY SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
