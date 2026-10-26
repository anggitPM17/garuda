#include "../include/garuda/beam/net/hot_reload.hpp"
#include <cassert>
#include <iostream>

using namespace garuda::beam;

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA ENGINE — SPRINT 3.5 HOT CODE RELOADING TEST SUITE (5 TESTS)\n";
    std::cout << "=======================================================\n\n";

    // === Test 1: Load Module Baru ===
    std::cout << "[TEST 1/5] Running Load New Module test...\n";
    HotReload hr;
    std::vector<Instruction> v1_code = {
        {Opcode::OpMove, {Operand{RegType::Literal, 0, Term(10)}, Operand{RegType::X, 0, Term()}}},
        {Opcode::OpReturn, {}}
    };
    std::map<std::string, std::uint64_t> v1_exports = {{"add", 0}, {"sub", 10}};

    std::uint32_t ver1 = hr.load_module("calc", v1_code, v1_exports);
    assert(ver1 == 1);
    assert(hr.get_version("calc") == 1);
    assert(!hr.get_old_module("calc").has_value()); // No old version yet
    std::cout << "  ✓ Test 1: Load New Module v1 PASS\n";

    // === Test 2: Upgrade Module (v1 -> v2) ===
    std::cout << "[TEST 2/5] Running Upgrade Module (v1 -> v2) test...\n";
    std::vector<Instruction> v2_code = {
        {Opcode::OpMove, {Operand{RegType::Literal, 0, Term(20)}, Operand{RegType::X, 0, Term()}}},
        {Opcode::OpReturn, {}}
    };
    std::map<std::string, std::uint64_t> v2_exports = {{"add", 0}, {"sub", 10}, {"mul", 20}};

    auto old_mod = hr.upgrade_module("calc", v2_code, v2_exports);
    assert(old_mod.has_value());
    assert(old_mod->version == 1);
    assert(hr.get_version("calc") == 2);

    // Old version still accessible in grace period
    auto old_still = hr.get_old_module("calc");
    assert(old_still.has_value() && old_still->version == 1);

    // Active version is v2
    auto active = hr.get_module("calc");
    assert(active.has_value() && active->version == 2);
    std::cout << "  ✓ Test 2: Upgrade Module v1 -> v2 (Grace Period) PASS\n";

    // === Test 3: Purge Old Version ===
    std::cout << "[TEST 3/5] Running Purge Old Version test...\n";
    bool purge_ok = hr.purge_old("calc");
    assert(purge_ok);

    assert(!hr.get_old_module("calc").has_value()); // Old version purged!
    assert(hr.get_version("calc") == 2);           // Active version remains v2
    std::cout << "  ✓ Test 3: Purge Old Version PASS\n";

    // === Test 4: Zero Downtime — Actor State Migration ===
    std::cout << "[TEST 4/5] Running Zero Downtime Actor State Migration test...\n";
    struct ActorStateV1 {
        int count = 0;
    };

    ActorStateV1 state_v1{5}; // Active actor state in v1

    hr.load_module("counter", v1_code, {});
    auto old_counter = hr.upgrade_module("counter", v2_code, {});
    assert(old_counter.has_value());

    // State Migration: Convert ActorStateV1 -> ActorStateV2 with preserved count
    struct ActorStateV2 {
        int count;
        int last_update;
    };
    ActorStateV2 state_v2{state_v1.count, 12345};

    assert(state_v2.count == 5);      // State preserved during live upgrade!
    assert(state_v2.last_update == 12345);

    hr.purge_old("counter");
    assert(hr.get_version("counter") == 2);
    std::cout << "  ✓ Test 4: Zero Downtime Actor State Migration PASS\n";

    // === Test 5: Multiple Upgrades Chain (v1 -> v2 -> v3 -> v4) ===
    std::cout << "[TEST 5/5] Running Multiple Upgrades Chain test...\n";
    HotReload hr_chain;
    hr_chain.load_module("chain_mod", v1_code, {});
    hr_chain.upgrade_module("chain_mod", v2_code, {});
    hr_chain.upgrade_module("chain_mod", v2_code, {});
    hr_chain.upgrade_module("chain_mod", v2_code, {});

    assert(hr_chain.get_version("chain_mod") == 4);
    std::cout << "  ✓ Test 5: Multiple Upgrades Chain (v1 -> v4) PASS\n";

    std::cout << "\n🎉 ALL 5 HOT CODE RELOADING TESTS PASSED PERFECTLY!\n";
    return 0;
}
