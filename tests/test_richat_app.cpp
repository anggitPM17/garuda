// =====================================================================
// FASE G8.1 - RICHAT APPLICATION TESTS
// Memverifikasi richat_app.hpp
// =====================================================================
#include "../include/garuda/richat/richat_app.hpp"
#include <iostream>
#include <string>
#include <filesystem>
#include <cstdlib>
#include <thread>
#include <system_error>

using namespace garuda::richat;

#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

int main() {
    std::cout << "========================================\n";
    std::cout << "FASE G8.1 - RICHAT APPLICATION TESTS\n";
    std::cout << "========================================\n\n";

    std::string test_dir = "./test_richat_app_db";
    std::error_code ec;
    std::filesystem::remove_all(test_dir, ec);

    {
        RichatAppConfig cfg;
        cfg.db_dir = test_dir;
        RichatApp app(cfg);

        // Test 1: App Init & Start
        std::cout << "[Test 1] App Init & Start...\n";
        {
            CHECK(!app.is_running(), "App awal harus STOPPED");
            bool started = app.start();
            CHECK(started, "app.start() harus return true");
            CHECK(app.is_running(), "is_running() harus true");
            CHECK(app.state_string() == "RUNNING", "State string harus RUNNING");
        }
        std::cout << "PASS\n\n";

        // Test 2: Mnesia Schema Ready
        std::cout << "[Test 2] Mnesia Schema Ready...\n";
        {
            auto* mnesia = app.mnesia();
            CHECK(mnesia != nullptr, "mnesia() instance harus non-null");

            mnesia->write("richat_user", garuda::beam::Term(std::string("u_app_1")), garuda::beam::Term(std::string("Alice App")));
            auto res = mnesia->read("richat_user", garuda::beam::Term(std::string("u_app_1")));
            CHECK(res.has_value() && *res->get_if<std::string>() == "Alice App", "Read data Mnesia dari app harus Alice App");
        }
        std::cout << "PASS\n\n";

        // Test 3: Double Start Rejection
        std::cout << "[Test 3] Double Start Rejection...\n";
        {
            bool double_start = app.start();
            CHECK(!double_start, "Start kedua harus ditolak (return false)");
            CHECK(app.is_running(), "State tetap RUNNING");
        }
        std::cout << "PASS\n\n";

        // Test 4: Graceful Stop
        std::cout << "[Test 4] Graceful Stop...\n";
        {
            bool stopped = app.stop();
            CHECK(stopped, "app.stop() harus return true");
            CHECK(!app.is_running(), "is_running() harus false setelah stop");
            CHECK(app.state_string() == "STOPPED", "State string harus STOPPED");
            CHECK(app.mnesia() == nullptr, "Mnesia instance harus reset (null)");
        }
        std::cout << "PASS\n\n";

        // Test 5: Health Check & Uptime
        std::cout << "[Test 5] Health Check & Uptime...\n";
        {
            CHECK(!app.health_check(), "Health check harus false saat stopped");

            app.start();
            CHECK(app.health_check(), "Health check harus true saat running");

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            CHECK(app.uptime_ms() >= 10, "Uptime ms harus >= 10");

            app.stop();
        }
        std::cout << "PASS\n\n";
    }

    std::filesystem::remove_all(test_dir, ec);

    std::cout << "========================================\n";
    std::cout << "FASE G8.1 - RICHAT APPLICATION SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
