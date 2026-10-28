// =====================================================================
// FASE G1.3 - GARUDA APPLICATION BEHAVIOUR TEST SUITE
// Memverifikasi application.hpp setara Erlang/OTP application.erl
// =====================================================================
#include "../include/garuda/beam/otp/application.hpp"
#include <iostream>
#include <memory>
#include <string>
#include <cstdlib>
#include <atomic>

using namespace garuda::beam;

#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

class SampleApp : public Application {
public:
    std::atomic<bool> start_called{false};
    std::atomic<bool> stop_called{false};
    Pid sup_pid;

    Pid start(Runtime& rt) override {
        start_called.store(true);
        SupervisorFlags flags;
        flags.strategy = Strategy::OneForOne;
        std::vector<ChildSpec> specs = {};
        sup_pid = start_supervisor(rt, flags, specs);
        return sup_pid;
    }

    void stop(Runtime& rt) override {
        (void)rt;
        stop_called.store(true);
    }
};

int main() {
    std::cout << "========================================\n";
    std::cout << "FASE G1.3 - GARUDA APPLICATION TESTS\n";
    std::cout << "========================================\n\n";

    // =================================================================
    // Test 1: Application start & stop callbacks
    // =================================================================
    std::cout << "[Test 1] Start & stop callbacks...\n";
    {
        Runtime rt;
        SampleApp app;

        Pid top_sup = app.start(rt);
        CHECK(top_sup.valid(), "Top supervisor pid harus valid");
        CHECK(app.start_called.load(), "start(Runtime&) harus dipanggil");

        app.stop(rt);
        CHECK(app.stop_called.load(), "stop(Runtime&) harus dipanggil");
    }
    std::cout << "PASS\n\n";

    // =================================================================
    // Test 2: Environment variables set_env & get_env
    // =================================================================
    std::cout << "[Test 2] Environment variables set_env & get_env...\n";
    {
        Application::set_env("richat", "max_connections", Term(static_cast<std::int64_t>(5000)));
        Application::set_env("richat", "node_name", Term(std::string("node1@localhost")));

        auto max_conn = Application::get_env("richat", "max_connections");
        CHECK(max_conn.has_value(), "env max_connections harus ada");
        const std::int64_t* conn_val = max_conn->get_if<std::int64_t>();
        CHECK(conn_val && *conn_val == 5000, "max_connections harus 5000");

        auto node_name = Application::get_env("richat", "node_name");
        CHECK(node_name.has_value(), "env node_name harus ada");
        const std::string* name_val = node_name->get_if<std::string>();
        CHECK(name_val && *name_val == "node1@localhost", "node_name harus node1@localhost");
    }
    std::cout << "PASS\n\n";

    // =================================================================
    // Test 3: Multiple independent applications with env
    // =================================================================
    std::cout << "[Test 3] Multiple independent apps with env...\n";
    {
        Runtime rt;
        SampleApp app_a;
        SampleApp app_b;

        Application::set_env("app_a", "port", Term(static_cast<std::int64_t>(8080)));
        Application::set_env("app_b", "port", Term(static_cast<std::int64_t>(9090)));

        Pid pid_a = app_a.start(rt);
        Pid pid_b = app_b.start(rt);
        rt.run_until_idle();

        CHECK(pid_a.valid() && pid_b.valid(), "Pids aplikasi A & B harus valid");

        auto port_a = Application::get_env("app_a", "port");
        auto port_b = Application::get_env("app_b", "port");
        CHECK(port_a && *port_a->get_if<std::int64_t>() == 8080, "Port App A harus 8080");
        CHECK(port_b && *port_b->get_if<std::int64_t>() == 9090, "Port App B harus 9090");

        app_a.stop(rt);
        app_b.stop(rt);
    }
    std::cout << "PASS\n\n";

    // =================================================================
    // Test 4: Environment overwrite and non-existent key lookups
    // =================================================================
    std::cout << "[Test 4] Env overwrite & non-existent keys...\n";
    {
        Application::set_env("system", "status", atom("initializing"));
        auto status1 = Application::get_env("system", "status");
        CHECK(status1 && is_atom(*status1, "initializing"), "Status awal harus initializing");

        // Overwrite
        Application::set_env("system", "status", atom("running"));
        auto status2 = Application::get_env("system", "status");
        CHECK(status2 && is_atom(*status2, "running"), "Status ter-update harus running");

        // Non-existent
        auto missing_app = Application::get_env("unknown_app", "key");
        CHECK(!missing_app.has_value(), "App tidak ada harus std::nullopt");

        auto missing_key = Application::get_env("system", "missing_key");
        CHECK(!missing_key.has_value(), "Key tidak ada harus std::nullopt");
    }
    std::cout << "PASS\n\n";

    // =================================================================
    // Test 5: Full application lifecycle integration
    // =================================================================
    std::cout << "[Test 5] Full lifecycle integration...\n";
    {
        Runtime rt;
        SampleApp full_app;

        Application::set_env("full_app", "state", atom("active"));
        Pid sup_pid = full_app.start(rt);
        rt.run_until_idle();

        CHECK(full_app.start_called.load(), "App start_called true");
        CHECK(sup_pid.valid(), "Top supervisor valid");

        auto env_state = Application::get_env("full_app", "state");
        CHECK(env_state && is_atom(*env_state, "active"), "Env state active");

        full_app.stop(rt);
        CHECK(full_app.stop_called.load(), "App stop_called true");
    }
    std::cout << "PASS\n\n";

    std::cout << "========================================\n";
    std::cout << "FASE G1.3 - APPLICATION SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
