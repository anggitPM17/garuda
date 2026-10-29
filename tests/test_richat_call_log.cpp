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
    std::cout << "R5 - RICHAT CALL LOG TESTS\n";
    std::cout << "========================================\n\n";

    std::string test_dir = "./test_richat_call_data";
    std::error_code ec;
    std::filesystem::remove_all(test_dir, ec);

    {
        JalurLangitUltimate db(test_dir);
        RichatGateway gateway(db);
        NusantaraBroker::get_instance().start();

        std::string last_status, last_session, last_call_id;
        NusantaraBroker::get_instance().register_entity(EntityTarget::RICHAT_APP, [&](const NusantaraMessage& m) {
            auto s_it = m.payload.find("status");
            if (s_it != m.payload.end()) last_status = s_it->second;
            auto sess_it = m.payload.find("session_token");
            if (sess_it != m.payload.end()) last_session = sess_it->second;
            auto c_it = m.payload.find("call_id");
            if (c_it != m.payload.end()) last_call_id = c_it->second;
        });

        // Login dulu
        NusantaraMessage login_msg;
        login_msg.message_id = "login1";
        login_msg.sender = EntityTarget::RICHAT_APP;
        login_msg.receiver = EntityTarget::JALUR_LANGIT;
        login_msg.action = NusantaraAction::RICHAT_LOGIN;
        login_msg.payload["auth_token"] = "token_abc";
        login_msg.payload["device_id"] = "android_001";
        gateway.handle_message(login_msg);
        NusantaraBroker::get_instance().process_queue();
        CHECK(last_status == "200", "Login harus 200");

        // Test 1: Start Call (Video)
        std::cout << "[Test 1] Start Call...\n";
        {
            NusantaraMessage msg;
            msg.message_id = "m1";
            msg.sender = EntityTarget::RICHAT_APP;
            msg.receiver = EntityTarget::JALUR_LANGIT;
            msg.action = NusantaraAction::RICHAT_START_CALL;
            msg.payload["session_token"] = last_session;
            msg.payload["receiver_id"] = "usr_target";
            msg.payload["type"] = "video";

            gateway.handle_message(msg);
            NusantaraBroker::get_instance().process_queue();
            CHECK(last_status == "200", "Start call harus 200");
            CHECK(!last_call_id.empty(), "Call ID harus ada");
        }
        std::cout << "PASS\n\n";

        // Test 2: End Call (Answered)
        std::cout << "[Test 2] End Call...\n";
        {
            NusantaraMessage msg;
            msg.message_id = "m2";
            msg.sender = EntityTarget::RICHAT_APP;
            msg.receiver = EntityTarget::JALUR_LANGIT;
            msg.action = NusantaraAction::RICHAT_END_CALL;
            msg.payload["session_token"] = last_session;
            msg.payload["call_id"] = last_call_id;
            msg.payload["status"] = "answered";

            gateway.handle_message(msg);
            NusantaraBroker::get_instance().process_queue();
            CHECK(last_status == "200", "End call harus 200");
        }
        std::cout << "PASS\n\n";

        // Test 3: Get Call Logs
        std::cout << "[Test 3] Get Call Logs...\n";
        {
            NusantaraMessage msg;
            msg.message_id = "m3";
            msg.sender = EntityTarget::RICHAT_APP;
            msg.receiver = EntityTarget::JALUR_LANGIT;
            msg.action = NusantaraAction::RICHAT_GET_CALL_LOGS;
            msg.payload["session_token"] = last_session;

            gateway.handle_message(msg);
            NusantaraBroker::get_instance().process_queue();
            CHECK(last_status == "200", "Get call logs harus 200");
        }
        std::cout << "PASS\n\n";

        // Test 4: Invalid Call Type
        std::cout << "[Test 4] Invalid Call Type...\n";
        {
            NusantaraMessage msg;
            msg.message_id = "m4";
            msg.sender = EntityTarget::RICHAT_APP;
            msg.receiver = EntityTarget::JALUR_LANGIT;
            msg.action = NusantaraAction::RICHAT_START_CALL;
            msg.payload["session_token"] = last_session;
            msg.payload["receiver_id"] = "usr_target";
            msg.payload["type"] = "hologram"; // invalid

            gateway.handle_message(msg);
            NusantaraBroker::get_instance().process_queue();
            CHECK(last_status == "400", "Invalid call type harus 400");
        }
        std::cout << "PASS\n\n";

        NusantaraBroker::get_instance().stop();
    }

    std::filesystem::remove_all(test_dir, ec);

    std::cout << "========================================\n";
    std::cout << "R5 - RICHAT CALL LOG SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
