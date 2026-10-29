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
    std::cout << "R4 - RICHAT STATUS & MOMENT TESTS\n";
    std::cout << "========================================\n\n";

    std::string test_dir = "./test_richat_status_data";
    std::error_code ec;
    std::filesystem::remove_all(test_dir, ec);

    {
        JalurLangitUltimate db(test_dir);
        RichatGateway gateway(db);
        NusantaraBroker::get_instance().start();

        std::string last_status, last_session, last_id;
        NusantaraBroker::get_instance().register_entity(EntityTarget::RICHAT_APP, [&](const NusantaraMessage& m) {
            auto s_it = m.payload.find("status");
            if (s_it != m.payload.end()) last_status = s_it->second;
            auto sess_it = m.payload.find("session_token");
            if (sess_it != m.payload.end()) last_session = sess_it->second;
            auto id_it = m.payload.find("status_id");
            if (id_it != m.payload.end()) last_id = id_it->second;
            auto mid_it = m.payload.find("moment_id");
            if (mid_it != m.payload.end()) last_id = mid_it->second;
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

        // Test 1: Create Status
        std::cout << "[Test 1] Create Status...\n";
        {
            NusantaraMessage msg;
            msg.message_id = "m1";
            msg.sender = EntityTarget::RICHAT_APP;
            msg.receiver = EntityTarget::JALUR_LANGIT;
            msg.action = NusantaraAction::RICHAT_CREATE_STATUS;
            msg.payload["session_token"] = last_session;
            msg.payload["content"] = "Halo dunia!";
            msg.payload["media_url"] = "http://example.com/image.jpg";

            gateway.handle_message(msg);
            NusantaraBroker::get_instance().process_queue();
            CHECK(last_status == "200", "Create status harus 200");
            CHECK(!last_id.empty(), "Status ID harus ada");
        }
        std::cout << "PASS\n\n";

        // Test 2: Get Statuses
        std::cout << "[Test 2] Get Statuses...\n";
        {
            NusantaraMessage msg;
            msg.message_id = "m2";
            msg.sender = EntityTarget::RICHAT_APP;
            msg.receiver = EntityTarget::JALUR_LANGIT;
            msg.action = NusantaraAction::RICHAT_GET_STATUSES;
            msg.payload["session_token"] = last_session;

            gateway.handle_message(msg);
            NusantaraBroker::get_instance().process_queue();
            CHECK(last_status == "200", "Get statuses harus 200");
        }
        std::cout << "PASS\n\n";

        // Test 3: Create Moment
        std::cout << "[Test 3] Create Moment...\n";
        {
            NusantaraMessage msg;
            msg.message_id = "m3";
            msg.sender = EntityTarget::RICHAT_APP;
            msg.receiver = EntityTarget::JALUR_LANGIT;
            msg.action = NusantaraAction::RICHAT_CREATE_MOMENT;
            msg.payload["session_token"] = last_session;
            msg.payload["media_url"] = "http://example.com/moment.jpg";
            msg.payload["caption"] = "Liburan!";
            msg.payload["visibility"] = "public";

            gateway.handle_message(msg);
            NusantaraBroker::get_instance().process_queue();
            CHECK(last_status == "200", "Create moment harus 200");
            CHECK(!last_id.empty(), "Moment ID harus ada");
        }
        std::cout << "PASS\n\n";

        // Test 4: Get Moments
        std::cout << "[Test 4] Get Moments...\n";
        {
            NusantaraMessage msg;
            msg.message_id = "m4";
            msg.sender = EntityTarget::RICHAT_APP;
            msg.receiver = EntityTarget::JALUR_LANGIT;
            msg.action = NusantaraAction::RICHAT_GET_MOMENTS;
            msg.payload["session_token"] = last_session;

            gateway.handle_message(msg);
            NusantaraBroker::get_instance().process_queue();
            CHECK(last_status == "200", "Get moments harus 200");
        }
        std::cout << "PASS\n\n";

        // Test 5: Like Moment
        std::cout << "[Test 5] Like Moment...\n";
        {
            NusantaraMessage msg;
            msg.message_id = "m5";
            msg.sender = EntityTarget::RICHAT_APP;
            msg.receiver = EntityTarget::JALUR_LANGIT;
            msg.action = NusantaraAction::RICHAT_LIKE_MOMENT;
            msg.payload["session_token"] = last_session;
            msg.payload["moment_id"] = last_id; // ID dari moment sebelumnya

            gateway.handle_message(msg);
            NusantaraBroker::get_instance().process_queue();
            CHECK(last_status == "200", "Like moment harus 200");
        }
        std::cout << "PASS\n\n";

        NusantaraBroker::get_instance().stop();
    }

    std::filesystem::remove_all(test_dir, ec);

    std::cout << "========================================\n";
    std::cout << "R4 - RICHAT STATUS & MOMENT SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
