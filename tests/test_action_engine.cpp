// =====================================================================
// FASE D.1.3 - GARUDA ACTION ENGINE TESTS
// Memverifikasi action_engine.hpp
// =====================================================================
#include "../include/garuda/autopilot/action_engine.hpp"
#include <iostream>
#include <string>
#include <cstdlib>

using namespace garuda::autopilot;

#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

int main() {
    std::cout << "========================================\n";
    std::cout << "FASE D.1.3 - GARUDA ACTION ENGINE TESTS\n";
    std::cout << "========================================\n\n";

    ActionEngine engine;

    // Test 1: Manual Mode Propose
    std::cout << "[Test 1] Manual Mode Propose...\n";
    {
        CHECK(engine.mode() == ActionMode::MANUAL, "Default mode harus MANUAL");

        RootCause rc;
        rc.anomaly_id = "anom_100";
        rc.recommended_action = "restart";
        rc.probable_cause = "High Memory Leak";

        auto act = engine.propose(rc);
        CHECK(act.has_value(), "propose() harus return Action");
        CHECK(engine.pending_count() == 1, "pending_count harus 1 di MANUAL mode");
        CHECK(act->type == ActionType::RESTART_SERVICE, "ActionType harus RESTART_SERVICE");
    }
    std::cout << "PASS\n\n";

    // Test 2: Approve Action
    std::cout << "[Test 2] Approve Action...\n";
    {
        auto pending = engine.pending_actions();
        CHECK(!pending.empty(), "Pending actions harus ada");

        std::string act_id = pending[0].id;
        auto res = engine.approve(act_id);
        CHECK(res.success == true, "approve() harus return success = true");
        CHECK(engine.pending_count() == 0, "pending_count berkurang jadi 0");
        CHECK(engine.executed_count() == 1, "executed_count bertambah jadi 1");
    }
    std::cout << "PASS\n\n";

    // Test 3: Reject Action
    std::cout << "[Test 3] Reject Action...\n";
    {
        RootCause rc;
        rc.recommended_action = "rollback";
        engine.propose(rc);
        CHECK(engine.pending_count() == 1, "pending_count = 1");

        auto pending = engine.pending_actions();
        bool rejected = engine.reject(pending[0].id);
        CHECK(rejected, "reject() harus return true");
        CHECK(engine.pending_count() == 0, "pending_count = 0 setelah reject");
    }
    std::cout << "PASS\n\n";

    // Test 4: Autopilot Mode Auto-Execute Safe Action
    std::cout << "[Test 4] Autopilot Mode Auto-Execute Safe Action...\n";
    {
        engine.set_mode(ActionMode::AUTOPILOT);
        CHECK(engine.mode() == ActionMode::AUTOPILOT, "Mode ter-update ke AUTOPILOT");

        RootCause rc;
        rc.recommended_action = "alert"; // Safe action (requires_approval = false)
        engine.propose(rc);

        CHECK(engine.pending_count() == 0, "Safe action di AUTOPILOT mode langsung dieksekusi (pending = 0)");
        CHECK(engine.executed_count() == 2, "executed_count bertambah jadi 2");
    }
    std::cout << "PASS\n\n";

    // Test 5: Execute Hook
    std::cout << "[Test 5] Execute Hook...\n";
    {
        bool hook_called = false;
        engine.set_mode(ActionMode::MANUAL);
        engine.set_execute_hook([&](const Action& a) {
            hook_called = true;
            return a.type == ActionType::RESTART_SERVICE;
        });

        RootCause rc;
        rc.recommended_action = "restart";
        auto act = engine.propose(rc);

        auto res = engine.approve(act->id);
        CHECK(hook_called, "Execute hook harus dipanggil saat approve");
        CHECK(res.success == true, "Execute hook return true -> res.success = true");
    }
    std::cout << "PASS\n\n";

    std::cout << "========================================\n";
    std::cout << "FASE D.1.3 - ACTION ENGINE SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
