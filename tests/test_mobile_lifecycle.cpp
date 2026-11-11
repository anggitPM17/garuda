#include <iostream>
#include <cassert>
#include <vector>

#include "garuda/mobile/mobile_lifecycle.hpp"

using namespace garuda::mobile;

void test_lifecycle_transitions() {
    GvmMobileLifecycleManager mgr;

    std::vector<MobileLifecycleState> transition_log;
    mgr.register_state_callback([&](MobileLifecycleState state) {
        transition_log.push_back(state);
    });

    mgr.on_create();
    assert(mgr.get_current_state() == MobileLifecycleState::CREATED);

    mgr.on_start();
    assert(mgr.get_current_state() == MobileLifecycleState::STARTED);

    mgr.on_resume();
    assert(mgr.get_current_state() == MobileLifecycleState::RESUMED);

    mgr.on_pause();
    assert(mgr.get_current_state() == MobileLifecycleState::PAUSED);

    mgr.on_stop();
    assert(mgr.get_current_state() == MobileLifecycleState::STOPPED);

    mgr.on_destroy();
    assert(mgr.get_current_state() == MobileLifecycleState::DESTROYED);

    assert(transition_log.size() == 6);
    assert(GvmMobileLifecycleManager::state_to_string(MobileLifecycleState::RESUMED) == "RESUMED");

    std::cout << "[PASS] test_lifecycle_transitions" << std::endl;
}

void test_power_save_mode_toggle() {
    GvmMobileLifecycleManager mgr;

    mgr.on_create();
    assert(!mgr.is_power_save_active());

    mgr.on_resume();
    assert(!mgr.is_power_save_active());

    mgr.on_pause();
    assert(mgr.is_power_save_active() && "on_pause should activate power saving mode");

    mgr.on_resume();
    assert(!mgr.is_power_save_active() && "on_resume should deactivate power saving mode");

    std::cout << "[PASS] test_power_save_mode_toggle" << std::endl;
}

void test_trim_memory_callbacks() {
    GvmMobileLifecycleManager mgr;

    bool trimmed = false;
    TrimMemoryLevel received_level = TrimMemoryLevel::RUNNING_LOW;

    mgr.register_trim_memory_callback([&](TrimMemoryLevel level) {
        trimmed = true;
        received_level = level;
    });

    mgr.on_trim_memory(TrimMemoryLevel::COMPLETE);
    assert(trimmed && "Trim memory callback should be invoked");
    assert(received_level == TrimMemoryLevel::COMPLETE);

    std::cout << "[PASS] test_trim_memory_callbacks" << std::endl;
}

int main() {
    std::cout << "--- RUNNING MOBILE LIFECYCLE TESTS ---" << std::endl;
    test_lifecycle_transitions();
    test_power_save_mode_toggle();
    test_trim_memory_callbacks();
    std::cout << "--- ALL MOBILE LIFECYCLE TESTS PASSED 100% ---" << std::endl;
    return 0;
}
