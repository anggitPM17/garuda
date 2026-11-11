#include <iostream>
#include <cassert>
#include <string>

#include "garuda/beam/gvm/hot_reload_live.hpp"

using namespace garuda::beam::gvm;

void test_function_registration_and_lookup() {
    GvmLiveHotReloader reloader;

    reloader.register_function("hitung_total", 10);
    reloader.register_function("proses_pesan", 25);

    assert(reloader.active_function_count() == 2);

    size_t offset = 0;
    assert(reloader.get_function_offset("hitung_total", offset) && offset == 10);
    assert(reloader.get_function_version("hitung_total") == 1);
    (void)offset;

    std::cout << "[PASS] test_function_registration_and_lookup" << std::endl;
}

void test_hot_code_swap() {
    GvmLiveHotReloader reloader;

    reloader.register_function("hitung_total", 10);

    // Live Hot Code Reloading / Code Swap
    bool swap_ok = reloader.hot_swap_function("hitung_total", 85);
    assert(swap_ok && "Hot code swap failed");
    (void)swap_ok;

    size_t new_offset = 0;
    assert(reloader.get_function_offset("hitung_total", new_offset) && new_offset == 85);
    assert(reloader.get_function_version("hitung_total") == 2);
    (void)new_offset;

    std::cout << "[PASS] test_hot_code_swap" << std::endl;
}

int main() {
    std::cout << "--- RUNNING GVM LIVE HOT RELOAD TESTS ---" << std::endl;
    test_function_registration_and_lookup();
    test_hot_code_swap();
    std::cout << "--- ALL GVM LIVE HOT RELOAD TESTS PASSED 100% ---" << std::endl;
    return 0;
}
