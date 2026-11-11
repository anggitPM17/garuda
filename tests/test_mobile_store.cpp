#include <iostream>
#include <cassert>
#include <string>
#include <vector>

#include "garuda/mobile/mobile_store.hpp"

using namespace garuda::mobile;

void test_kv_store_operations() {
    GvmMobileStore store;

    store.put("user_token", "jwt_token_12345");
    store.put("theme_mode", "dark");

    std::string token;
    assert(store.get("user_token", token) && token == "jwt_token_12345");

    std::string theme;
    assert(store.get("theme_mode", theme) && theme == "dark");

    std::string missing;
    assert(!store.get("missing_key", missing));

    assert(store.remove("theme_mode"));
    assert(!store.get("theme_mode", theme));

    std::cout << "[PASS] test_kv_store_operations" << std::endl;
}

void test_offline_action_queue() {
    GvmMobileStore store;

    // Simulate offline state
    store.set_online_status(false);
    assert(!store.is_online());

    uint64_t act1 = store.queue_action("SEND_MESSAGE", "{\"chat_id\": 10, \"text\": \"Halo Offline!\"}");
    uint64_t act2 = store.queue_action("READ_RECEIPT", "{\"message_id\": 500}");
    (void)act1;
    (void)act2;

    assert(store.pending_count() == 2);
    auto pending = store.get_pending_actions();
    assert(pending.size() == 2);
    assert(pending[0].id == act1 && pending[0].action_type == "SEND_MESSAGE");
    assert(pending[1].id == act2 && pending[1].action_type == "READ_RECEIPT");

    // Re-connect online and sync
    store.set_online_status(true);
    assert(store.is_online());

    assert(store.mark_action_synced(act1));
    assert(store.pending_count() == 1);

    size_t cleared = store.clear_synced_actions();
    assert(cleared == 1);
    assert(store.pending_count() == 1);

    assert(store.mark_action_synced(act2));
    assert(store.pending_count() == 0);

    cleared = store.clear_synced_actions();
    assert(cleared == 1);

    std::cout << "[PASS] test_offline_action_queue" << std::endl;
}

int main() {
    std::cout << "--- RUNNING MOBILE OFFLINE STORE TESTS ---" << std::endl;
    test_kv_store_operations();
    test_offline_action_queue();
    std::cout << "--- ALL MOBILE OFFLINE STORE TESTS PASSED 100% ---" << std::endl;
    return 0;
}
