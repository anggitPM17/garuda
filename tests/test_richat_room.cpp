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
    std::cout << "R2 - RICHAT ROOM TESTS\n";
    std::cout << "========================================\n\n";

    std::string test_dir = "./test_richat_room_data";
    std::error_code ec;
    std::filesystem::remove_all(test_dir, ec);

    {
        JalurLangitUltimate db(test_dir);
        RichatGateway gateway(db);
        NusantaraBroker::get_instance().start();

        std::string last_status, last_room_id, last_session;
        NusantaraBroker::get_instance().register_entity(EntityTarget::RICHAT_APP, [&](const NusantaraMessage& m) {
            auto s_it = m.payload.find("status");
            if (s_it != m.payload.end()) last_status = s_it->second;
            auto r_it = m.payload.find("room_id");
            if (r_it != m.payload.end()) last_room_id = r_it->second;
            auto sess_it = m.payload.find("session_token");
            if (sess_it != m.payload.end()) last_session = sess_it->second;
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

        // Test 1: Create Room
        std::cout << "[Test 1] Create Room...\n";
        {
            NusantaraMessage msg;
            msg.message_id = "m1";
            msg.sender = EntityTarget::RICHAT_APP;
            msg.receiver = EntityTarget::JALUR_LANGIT;
            msg.action = NusantaraAction::RICHAT_CREATE_ROOM;
            msg.payload["session_token"] = last_session;
            msg.payload["room_name"] = "Grup Garuda";

            gateway.handle_message(msg);
            NusantaraBroker::get_instance().process_queue();
            CHECK(last_status == "200", "Create room harus 200");
            CHECK(!last_room_id.empty(), "Room ID harus ada");
        }
        std::cout << "PASS\n\n";

        // Test 2: Join Room
        std::cout << "[Test 2] Join Room...\n";
        {
            NusantaraMessage msg;
            msg.message_id = "m2";
            msg.sender = EntityTarget::RICHAT_APP;
            msg.receiver = EntityTarget::JALUR_LANGIT;
            msg.action = NusantaraAction::RICHAT_JOIN_ROOM;
            msg.payload["session_token"] = last_session;
            msg.payload["room_id"] = last_room_id;

            gateway.handle_message(msg);
            NusantaraBroker::get_instance().process_queue();
            CHECK(last_status == "200", "Join room harus 200");
        }
        std::cout << "PASS\n\n";

        // Test 3: Send Room Message
        std::cout << "[Test 3] Send Room Message...\n";
        {
            NusantaraMessage msg;
            msg.message_id = "m3";
            msg.sender = EntityTarget::RICHAT_APP;
            msg.receiver = EntityTarget::JALUR_LANGIT;
            msg.action = NusantaraAction::RICHAT_SEND_ROOM_MESSAGE;
            msg.payload["session_token"] = last_session;
            msg.payload["room_id"] = last_room_id;
            msg.payload["content"] = "Halo semua!";
            msg.payload["client_msg_id"] = "cmsg_room_001";

            gateway.handle_message(msg);
            NusantaraBroker::get_instance().process_queue();
            CHECK(last_status == "200", "Send room message harus 200");
        }
        std::cout << "PASS\n\n";

        NusantaraBroker::get_instance().stop();
    }

    std::filesystem::remove_all(test_dir, ec);

    std::cout << "========================================\n";
    std::cout << "R2 - RICHAT ROOM SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
