#include "../include/garuda/autopilot/admin_gate.hpp"
#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <cstdlib>

using namespace garuda::autopilot;

#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

int main() {
    std::cout << "========================================\n";
    std::cout << "FASE 5.1 - ADMIN GATE TESTS\n";
    std::cout << "========================================\n\n";

    // Test 1: Default mode = MANUAL, tidak bisa eksekusi
    std::cout << "[Test 1] Default mode MANUAL...\n";
    {
        AdminGate gate;
        CHECK(gate.get_mode() == AutopilotMode::MANUAL, "Default harus MANUAL");
        CHECK(!gate.can_execute_autonomous(), "MANUAL tidak boleh eksekusi");
    }
    std::cout << "PASS\n\n";

    // Test 2: Set AUTOPILOT dengan token valid
    std::cout << "[Test 2] Set AUTOPILOT dengan token valid...\n";
    {
        AdminGate gate;
        gate.set_admin_token("SECRET_ADMIN");
        CHECK(gate.set_mode(AutopilotMode::AUTOPILOT, "SECRET_ADMIN"), "Set mode harus berhasil");
        CHECK(gate.get_mode() == AutopilotMode::AUTOPILOT, "Mode harus AUTOPILOT");
        CHECK(gate.can_execute_autonomous(), "AUTOPILOT harus bisa eksekusi");
    }
    std::cout << "PASS\n\n";

    // Test 3: Set mode tanpa token / token salah -> gagal
    std::cout << "[Test 3] Set mode tanpa token / token salah...\n";
    {
        AdminGate gate;
        gate.set_admin_token("SECRET_ADMIN");
        CHECK(!gate.set_mode(AutopilotMode::AUTOPILOT, ""), "Token kosong harus gagal");
        CHECK(!gate.set_mode(AutopilotMode::AUTOPILOT, "WRONG"), "Token salah harus gagal");
        CHECK(gate.get_mode() == AutopilotMode::MANUAL, "Mode tetap MANUAL");

        AdminGate gate2; // belum set token
        CHECK(!gate2.set_mode(AutopilotMode::AUTOPILOT, "ANY"), "Belum ada token harus gagal");
    }
    std::cout << "PASS\n\n";

    // Test 4: Mode STOP -> tidak bisa eksekusi, dan force_stop bekerja
    std::cout << "[Test 4] STOP mode & force_stop...\n";
    {
        AdminGate gate;
        gate.set_admin_token("SECRET_ADMIN");
        CHECK(gate.set_mode(AutopilotMode::STOP, "SECRET_ADMIN"), "Set STOP harus berhasil");
        CHECK(!gate.can_execute_autonomous(), "STOP tidak boleh eksekusi");

        // force_stop dari AUTOPILOT
        gate.set_mode(AutopilotMode::AUTOPILOT, "SECRET_ADMIN");
        CHECK(gate.can_execute_autonomous(), "AUTOPILOT aktif");
        gate.force_stop();
        CHECK(gate.get_mode() == AutopilotMode::STOP, "force_stop harus jadi STOP");
        CHECK(!gate.can_execute_autonomous(), "Setelah force_stop tidak boleh eksekusi");
    }
    std::cout << "PASS\n\n";

    // Test 5: Thread-safety smoke test
    std::cout << "[Test 5] Thread-safety smoke test...\n";
    {
        AdminGate gate;
        gate.set_admin_token("SECRET_ADMIN");
        std::atomic<int> reads{0};
        std::atomic<bool> failed{false};

        std::thread reader([&]() {
            for (int i = 0; i < 10000; ++i) {
                (void)gate.can_execute_autonomous();
                reads.fetch_add(1);
            }
        });
        std::thread writer([&]() {
            for (int i = 0; i < 1000; ++i) {
                if (!gate.set_mode(AutopilotMode::AUTOPILOT, "SECRET_ADMIN")) failed.store(true);
                if (!gate.set_mode(AutopilotMode::MANUAL, "SECRET_ADMIN")) failed.store(true);
            }
        });
        reader.join();
        writer.join();

        CHECK(reads.load() == 10000, "Reader harus selesai 10000x");
        CHECK(!failed.load(), "Writer tidak boleh gagal set_mode dengan token valid");
    }
    std::cout << "PASS\n\n";

    std::cout << "========================================\n";
    std::cout << "ADMIN GATE SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
