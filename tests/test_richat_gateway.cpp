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
    std::cout << "R1 - RICHAT GATEWAY TESTS\n";
    std::cout << "========================================\n\n";

    std::string test_dir = "./test_richat_data";
    std::error_code ec;
    std::filesystem::remove_all(test_dir, ec);

    {
        JalurLangitUltimate db(test_dir);
        RichatGateway gateway(db);
        NusantaraBroker::get_instance().start();

        // Register receiver untuk menangkap response
        std::string last_status, last_session;
        NusantaraBroker::get_instance().register_entity(EntityTarget::RICHAT_APP, [&](const NusantaraMessage& m) {
            auto s_it = m.payload.find("status");
            if (s_it != m.payload.end()) last_status = s_it->second;
            auto sess_it = m.payload.find("session_token");
            if (sess_it != m.payload.end()) last_session = sess_it->second;
        });

        // Test 1: Login valid
        std::cout << "[Test 1] Login valid...\n";
        {
            NusantaraMessage msg;
            msg.message_id = "m1";
            msg.sender = EntityTarget::RICHAT_APP;
            msg.receiver = EntityTarget::JALUR_LANGIT;
            msg.action = NusantaraAction::RICHAT_LOGIN;
            msg.payload["auth_token"] = "token_abc";
            msg.payload["device_id"] = "android_001";

            gateway.handle_message(msg);
            NusantaraBroker::get_instance().process_queue();
            CHECK(last_status == "200", "Login harus 200");
            CHECK(!last_session.empty(), "Session token harus ada");
        }
        std::cout << "PASS\n\n";

        // Test 2: Login invalid (missing field)
        std::cout << "[Test 2] Login invalid...\n";
        {
            NusantaraMessage msg;
            msg.message_id = "m2";
            msg.sender = EntityTarget::RICHAT_APP;
            msg.receiver = EntityTarget::JALUR_LANGIT;
            msg.action = NusantaraAction::RICHAT_LOGIN;
            msg.payload["auth_token"] = "token_abc"; // missing device_id

            gateway.handle_message(msg);
            NusantaraBroker::get_instance().process_queue();
            CHECK(last_status == "400", "Login invalid harus 400");
        }
        std::cout << "PASS\n\n";

        // Test 3: Send message sukses
        std::cout << "[Test 3] Send message...\n";
        {
            NusantaraMessage msg;
            msg.message_id = "m3";
            msg.sender = EntityTarget::RICHAT_APP;
            msg.receiver = EntityTarget::JALUR_LANGIT;
            msg.action = NusantaraAction::RICHAT_SEND_MESSAGE;
            msg.payload["session_token"] = last_session;
            msg.payload["receiver_id"] = "usr_002";
            msg.payload["content"] = "Halo!";
            msg.payload["client_msg_id"] = "cmsg_001";

            gateway.handle_message(msg);
            NusantaraBroker::get_instance().process_queue();
            CHECK(last_status == "200", "Send message harus 200");
        }
        std::cout << "PASS\n\n";

        // Test 4: Send message invalid session
        std::cout << "[Test 4] Send message invalid session...\n";
        {
            NusantaraMessage msg;
            msg.message_id = "m4";
            msg.sender = EntityTarget::RICHAT_APP;
            msg.receiver = EntityTarget::JALUR_LANGIT;
            msg.action = NusantaraAction::RICHAT_SEND_MESSAGE;
            msg.payload["session_token"] = "invalid_session";
            msg.payload["receiver_id"] = "usr_002";
            msg.payload["content"] = "Halo!";
            msg.payload["client_msg_id"] = "cmsg_002";

            gateway.handle_message(msg);
            NusantaraBroker::get_instance().process_queue();
            CHECK(last_status == "401", "Invalid session harus 401");
        }
        std::cout << "PASS\n\n";

        NusantaraBroker::get_instance().stop();
    }

    std::filesystem::remove_all(test_dir, ec);

    std::cout << "========================================\n";
    std::cout << "R1 - RICHAT GATEWAY SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
