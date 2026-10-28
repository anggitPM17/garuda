// =====================================================================
// FASE G6.3 - GARUDA OBSERVER TESTS
// Memverifikasi observer.hpp setara Erlang observer
// =====================================================================
#include "../include/garuda/beam/shell/observer.hpp"
#include <iostream>
#include <string>
#include <cstdlib>

using namespace garuda::beam;

#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

int main() {
    std::cout << "========================================\n";
    std::cout << "FASE G6.3 - GARUDA OBSERVER TESTS\n";
    std::cout << "========================================\n\n";

    Observer obs;

    // Test 1: Register Process
    std::cout << "[Test 1] Register Process...\n";
    {
        ProcessInfo p1{"<0.100.0>", "richat_gateway", "running", 15000, 102400};
        ProcessInfo p2{"<0.101.0>", "jalur_langit_wal", "waiting", 8200, 204800};
        ProcessInfo p3{"<0.102.0>", "autopilot_ai", "running", 30000, 512000};

        obs.register_process(p1);
        obs.register_process(p2);
        obs.register_process(p3);

        CHECK(obs.process_count() == 3, "Process count harus 3");
    }
    std::cout << "PASS\n\n";

    // Test 2: Unregister Process
    std::cout << "[Test 2] Unregister Process...\n";
    {
        obs.unregister_process("<0.101.0>");
        CHECK(obs.process_count() == 2, "Process count sisa 2 setelah unregister");
    }
    std::cout << "PASS\n\n";

    // Test 3: Counter Increment & Set
    std::cout << "[Test 3] Counter Increment & Set...\n";
    {
        obs.increment_counter("messages_processed", 1);
        obs.increment_counter("messages_processed", 2);
        CHECK(obs.get_counter("messages_processed") == 3, "messages_processed counter harus 3");

        obs.set_counter("active_connections", 150);
        CHECK(obs.get_counter("active_connections") == 150, "active_connections counter harus 150");
    }
    std::cout << "PASS\n\n";

    // Test 4: System Snapshot
    std::cout << "[Test 4] System Snapshot...\n";
    {
        auto snap = obs.snapshot();
        CHECK(snap.process_count == 2, "Snapshot process count harus 2");
        CHECK(snap.total_memory_bytes == (102400 + 512000), "Total memory bytes harus 614400");
        CHECK(snap.processes.size() == 2, "Snapshot processes vector size = 2");
    }
    std::cout << "PASS\n\n";

    // Test 5: Render Text Output
    std::cout << "[Test 5] Render Text Output...\n";
    {
        std::string text = obs.render_text();
        CHECK(!text.empty(), "Render text harus non-kosong");
        CHECK(text.find("GARUDA GVM OBSERVER DASHBOARD") != std::string::npos, "Dashboard title match");
        CHECK(text.find("richat_gateway") != std::string::npos, "Process richat_gateway match");
        CHECK(text.find("messages_processed : 3") != std::string::npos, "Metric messages_processed match");
    }
    std::cout << "PASS\n\n";

    std::cout << "========================================\n";
    std::cout << "FASE G6.3 - OBSERVER SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
