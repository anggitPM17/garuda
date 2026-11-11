#include <iostream>
#include <cassert>
#include <vector>
#include <string>

#include "garuda/mobile/native_bridge.hpp"

using namespace garuda::mobile;

void test_emit_to_ui() {
    GvmNativeBridge bridge;

    std::vector<BridgeMessage> received_messages;
    bridge.register_ui_listener([&](const BridgeMessage& msg) {
        received_messages.push_back(msg);
    });

    bridge.emit_to_ui("chat_feed", "{\"text\": \"Salam dari Garuda!\"}");

    assert(received_messages.size() == 1);
    assert(received_messages[0].channel == "chat_feed");
    assert(received_messages[0].payload == "{\"text\": \"Salam dari Garuda!\"}");
    assert(received_messages[0].timestamp_ms > 0);

    std::cout << "[PASS] test_emit_to_ui" << std::endl;
}

void test_dispatch_from_ui() {
    GvmNativeBridge bridge;

    bridge.register_action_handler("kirim_pesan", [](const std::string& payload) {
        return "{\"status\": \"ok\", \"received\": " + payload + "}";
    });

    assert(bridge.has_handler("kirim_pesan"));
    assert(!bridge.has_handler("non_existent_channel"));

    std::string response = bridge.dispatch_from_ui("kirim_pesan", "{\"id\": 101}");
    assert(response == "{\"status\": \"ok\", \"received\": {\"id\": 101}}");

    std::cout << "[PASS] test_dispatch_from_ui" << std::endl;
}

void test_unknown_channel_dispatch() {
    GvmNativeBridge bridge;

    std::string response = bridge.dispatch_from_ui("channel_unknown", "{}");
    assert(response.find("Channel handler not found") != std::string::npos);

    std::cout << "[PASS] test_unknown_channel_dispatch" << std::endl;
}

int main() {
    std::cout << "--- RUNNING NATIVE BRIDGE TESTS ---" << std::endl;
    test_emit_to_ui();
    test_dispatch_from_ui();
    test_unknown_channel_dispatch();
    std::cout << "--- ALL NATIVE BRIDGE TESTS PASSED 100% ---" << std::endl;
    return 0;
}
