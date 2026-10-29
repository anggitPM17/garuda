// =====================================================================
// FASE A.1 - RICHAT CHAT API TESTS
// Memverifikasi richat_chat_api.hpp
// =====================================================================
#include "../include/garuda/richat/richat_chat_api.hpp"
#include <iostream>
#include <string>
#include <cstdlib>

using namespace garuda::richat;

#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

int main() {
    std::cout << "========================================\n";
    std::cout << "FASE A.1 - RICHAT CHAT API TESTS\n";
    std::cout << "========================================\n\n";

    RichatChatApi api;

    // Test 1: Send Message
    std::cout << "[Test 1] Send Message...\n";
    {
        auto msg_opt = api.send_message("usr_a", "usr_b", "Halo B, ini A!");
        CHECK(msg_opt.has_value(), "send_message harus return ChatMessage");
        CHECK(msg_opt->sender_id == "usr_a", "Sender ID harus usr_a");
        CHECK(msg_opt->recipient_id == "usr_b", "Recipient ID harus usr_b");
        CHECK(msg_opt->status == "sent", "Status harus sent");
    }
    std::cout << "PASS\n\n";

    // Test 2: Get Messages
    std::cout << "[Test 2] Get Messages...\n";
    {
        api.send_message("usr_b", "usr_a", "Halo A, ini balasan B!");
        api.send_message("usr_a", "usr_b", "Pesan ke-3");

        auto history = api.get_messages("usr_a", "usr_b");
        CHECK(history.size() == 3, "get_messages harus mengembalikan 3 pesan antara usr_a dan usr_b");
    }
    std::cout << "PASS\n\n";

    // Test 3: Get Messages Filter User Isolation
    std::cout << "[Test 3] Get Messages Filter User Isolation...\n";
    {
        api.send_message("usr_a", "usr_c", "Pesan rahasia untuk C");
        auto history_ab = api.get_messages("usr_a", "usr_b");
        CHECK(history_ab.size() == 3, "Pesan untuk usr_c tidak boleh masuk ke riwayat usr_a & usr_b");

        auto history_ac = api.get_messages("usr_a", "usr_c");
        CHECK(history_ac.size() == 1, "Pesan untuk usr_c harus berjumlah 1");
    }
    std::cout << "PASS\n\n";

    // Test 4: Get Single Message by ID
    std::cout << "[Test 4] Get Single Message by ID...\n";
    {
        auto msg_opt = api.send_message("usr_a", "usr_b", "Test ID lookup");
        CHECK(msg_opt.has_value(), "send_message OK");

        auto found_opt = api.get_message(msg_opt->id);
        CHECK(found_opt.has_value(), "get_message() harus menemukan pesan berdasarkan ID");
        CHECK(found_opt->content == "Test ID lookup", "Content harus match");
    }
    std::cout << "PASS\n\n";

    // Test 5: Soft Delete Message
    std::cout << "[Test 5] Soft Delete Message...\n";
    {
        auto msg_opt = api.send_message("usr_a", "usr_b", "Pesan untuk dihapus");
        CHECK(msg_opt.has_value(), "send_message OK");

        bool deleted = api.delete_message(msg_opt->id, "usr_a");
        CHECK(deleted, "delete_message harus return true");

        auto found_opt = api.get_message(msg_opt->id);
        CHECK(found_opt.has_value() && found_opt->status == "deleted", "Status pesan harus ter-update 'deleted'");

        auto history = api.get_messages("usr_a", "usr_b");
        auto it = std::find_if(history.begin(), history.end(), [&](const ChatMessage& m) { return m.id == msg_opt->id; });
        CHECK(it == history.end(), "Pesan yang terhapus tidak boleh muncul di get_messages()");
    }
    std::cout << "PASS\n\n";

    std::cout << "========================================\n";
    std::cout << "FASE A.1 - RICHAT CHAT API SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
