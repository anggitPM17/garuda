// =====================================================================
// FASE G5.3 - GARUDA TYPING INDICATOR TESTS
// Memverifikasi typing_indicator.hpp
// =====================================================================
#include "../include/garuda/beam/webrtc/typing_indicator.hpp"
#include <iostream>
#include <string>
#include <cstdlib>

using namespace garuda::beam;

#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

int main() {
    std::cout << "========================================\n";
    std::cout << "FASE G5.3 - GARUDA TYPING INDICATOR TESTS\n";
    std::cout << "========================================\n\n";

    TypingIndicator ti;

    // Test 1: Start Typing
    std::cout << "[Test 1] Start Typing...\n";
    {
        ti.start_typing("user_101", "conv_1");
        CHECK(ti.is_typing("user_101", "conv_1"), "user_101 harus typing di conv_1");
        CHECK(ti.active_count() == 1, "Active typing count harus 1");
    }
    std::cout << "PASS\n\n";

    // Test 2: Stop Typing
    std::cout << "[Test 2] Stop Typing...\n";
    {
        ti.stop_typing("user_101", "conv_1");
        CHECK(!ti.is_typing("user_101", "conv_1"), "user_101 tidak typing setelah stop");
        CHECK(ti.active_count() == 0, "Active typing count harus 0");
    }
    std::cout << "PASS\n\n";

    // Test 3: Multiple Users Typing
    std::cout << "[Test 3] Multiple Users Typing...\n";
    {
        ti.start_typing("user_a", "conv_group_1");
        ti.start_typing("user_b", "conv_group_1");
        ti.start_typing("user_c", "conv_group_1");

        auto typing = ti.get_typing_users("conv_group_1");
        CHECK(typing.size() == 3, "Harus ada 3 user typing di conv_group_1");
    }
    std::cout << "PASS\n\n";

    // Test 4: Expired Typing State
    std::cout << "[Test 4] Expired Typing State...\n";
    {
        ti.start_typing("user_exp", "conv_group_1", false, 0); // ttl = 0 (immediately expired)
        CHECK(!ti.is_typing("user_exp", "conv_group_1"), "user_exp harus expired");
    }
    std::cout << "PASS\n\n";

    // Test 5: Sweep Expired
    std::cout << "[Test 5] Sweep Expired...\n";
    {
        std::size_t swept = ti.sweep_expired();
        CHECK(swept == 1, "Sweep harus menghapus 1 expired state");
        CHECK(ti.active_count() == 3, "Sisa 3 active typing users");
    }
    std::cout << "PASS\n\n";

    std::cout << "========================================\n";
    std::cout << "FASE G5.3 - TYPING INDICATOR SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
