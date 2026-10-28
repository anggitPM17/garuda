#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <cstdlib>

#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

namespace richat_contract {

bool validate_login(const std::map<std::string, std::string>& payload) {
    auto auth_it = payload.find("auth_token");
    auto dev_it = payload.find("device_id");
    if (auth_it == payload.end() || auth_it->second.empty()) return false;
    if (dev_it == payload.end() || dev_it->second.empty()) return false;
    return true;
}

bool validate_send_message(const std::map<std::string, std::string>& payload) {
    auto sess_it = payload.find("session_token");
    auto rec_it = payload.find("receiver_id");
    auto cnt_it = payload.find("content");
    auto cmsg_it = payload.find("client_msg_id");

    if (sess_it == payload.end() || sess_it->second.empty()) return false;
    if (rec_it == payload.end() || rec_it->second.empty()) return false;
    if (cnt_it == payload.end() || cnt_it->second.empty() || cnt_it->second.size() > 4096) return false;
    if (cmsg_it == payload.end() || cmsg_it->second.empty()) return false;
    return true;
}

bool validate_get_messages(const std::map<std::string, std::string>& payload) {
    auto sess_it = payload.find("session_token");
    auto with_it = payload.find("with_user_id");

    if (sess_it == payload.end() || sess_it->second.empty()) return false;
    if (with_it == payload.end() || with_it->second.empty()) return false;
    return true;
}

bool validate_ack_delivered(const std::map<std::string, std::string>& payload) {
    auto sess_it = payload.find("session_token");
    auto msg_it = payload.find("message_id");

    if (sess_it == payload.end() || sess_it->second.empty()) return false;
    if (msg_it == payload.end() || msg_it->second.empty()) return false;
    return true;
}

} // namespace richat_contract

int main() {
    std::cout << "========================================\n";
    std::cout << "R0 - RICHAT API CONTRACT VALIDATION TESTS\n";
    std::cout << "========================================\n\n";

    // Test 1: RICHAT_LOGIN validation
    std::cout << "[Test 1] RICHAT_LOGIN payload validation...\n";
    {
        std::map<std::string, std::string> valid_p = {{"auth_token", "abc123token"}, {"device_id", "android-001"}};
        std::map<std::string, std::string> invalid_p1 = {{"auth_token", "abc123token"}};
        std::map<std::string, std::string> invalid_p2 = {{"device_id", "android-001"}};
        std::map<std::string, std::string> invalid_p3 = {{"auth_token", ""}, {"device_id", "android-001"}};

        CHECK(richat_contract::validate_login(valid_p), "Valid login payload harus pass");
        CHECK(!richat_contract::validate_login(invalid_p1), "Missing device_id harus fail");
        CHECK(!richat_contract::validate_login(invalid_p2), "Missing auth_token harus fail");
        CHECK(!richat_contract::validate_login(invalid_p3), "Empty auth_token harus fail");
    }
    std::cout << "PASS\n\n";

    // Test 2: RICHAT_SEND_MESSAGE validation
    std::cout << "[Test 2] RICHAT_SEND_MESSAGE payload validation...\n";
    {
        std::map<std::string, std::string> valid_p = {
            {"session_token", "sess_101"},
            {"receiver_id", "usr_102"},
            {"content", "Halo Bro"},
            {"client_msg_id", "cmsg_001"}
        };
        std::map<std::string, std::string> invalid_no_cmsg = {
            {"session_token", "sess_101"},
            {"receiver_id", "usr_102"},
            {"content", "Halo Bro"}
        };
        std::map<std::string, std::string> invalid_empty_content = {
            {"session_token", "sess_101"},
            {"receiver_id", "usr_102"},
            {"content", ""},
            {"client_msg_id", "cmsg_001"}
        };
        std::map<std::string, std::string> invalid_huge_content = {
            {"session_token", "sess_101"},
            {"receiver_id", "usr_102"},
            {"content", std::string(4097, 'X')},
            {"client_msg_id", "cmsg_001"}
        };

        CHECK(richat_contract::validate_send_message(valid_p), "Valid send_message payload harus pass");
        CHECK(!richat_contract::validate_send_message(invalid_no_cmsg), "Missing client_msg_id harus fail");
        CHECK(!richat_contract::validate_send_message(invalid_empty_content), "Empty content harus fail");
        CHECK(!richat_contract::validate_send_message(invalid_huge_content), "Content > 4096 bytes harus fail");
    }
    std::cout << "PASS\n\n";

    // Test 3: RICHAT_GET_MESSAGES validation
    std::cout << "[Test 3] RICHAT_GET_MESSAGES payload validation...\n";
    {
        std::map<std::string, std::string> valid_p = {{"session_token", "sess_101"}, {"with_user_id", "usr_102"}};
        std::map<std::string, std::string> invalid_p = {{"session_token", "sess_101"}, {"with_user_id", ""}};

        CHECK(richat_contract::validate_get_messages(valid_p), "Valid get_messages payload harus pass");
        CHECK(!richat_contract::validate_get_messages(invalid_p), "Empty with_user_id harus fail");
    }
    std::cout << "PASS\n\n";

    // Test 4: RICHAT_ACK_DELIVERED validation
    std::cout << "[Test 4] RICHAT_ACK_DELIVERED payload validation...\n";
    {
        std::map<std::string, std::string> valid_p = {{"session_token", "sess_101"}, {"message_id", "msg_9001"}};
        std::map<std::string, std::string> invalid_p = {{"session_token", "sess_101"}};

        CHECK(richat_contract::validate_ack_delivered(valid_p), "Valid ack payload harus pass");
        CHECK(!richat_contract::validate_ack_delivered(invalid_p), "Missing message_id harus fail");
    }
    std::cout << "PASS\n\n";

    // Test 5: Verify schema file presence
    std::cout << "[Test 5] Checking test_richat_contract_schema.json existence...\n";
    {
        std::ifstream f("tests/test_richat_contract_schema.json");
        CHECK(f.good(), "Schema JSON file harus ada di tests/test_richat_contract_schema.json");
    }
    std::cout << "PASS\n\n";

    std::cout << "========================================\n";
    std::cout << "RICHAT API CONTRACT SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
