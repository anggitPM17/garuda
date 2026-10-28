// =====================================================================
// FASE G1.2 - GARUDA SUPERVISOR TEST SUITE
// Memverifikasi supervisor.hpp setara Erlang/OTP supervisor.erl
// =====================================================================
#include "../include/garuda/beam/otp/supervisor.hpp"
#include <iostream>
#include <memory>
#include <string>
#include <cstdlib>
#include <atomic>

using namespace garuda::beam;

#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

// Worker state yang menyimpan Pid untuk crash nanti
struct WorkerState {
    std::atomic<int> start_count{0};
    std::atomic<int> stop_count{0};
    Pid pid;  // diisi saat start
};

// Spawn worker yang menunggu pesan "crash" atau "normal_exit"
inline Pid spawn_test_worker(Process& sup, std::shared_ptr<WorkerState> state) {
    auto inited = std::make_shared<bool>(false);
    return sup.spawn_link([state, inited](Process& p) -> Status {
        if (!*inited) {
            *inited = true;
            state->start_count.fetch_add(1);
            state->pid = p.self();
        }
        while (true) {
            auto m = p.receive();
            if (!m) return Status::Wait;
            const Term& t = m->body;
            if (is_atom(t, "crash")) {
                state->stop_count.fetch_add(1);
                return p.stop(atom("crash"));
            }
            if (is_atom(t, "normal_exit")) {
                state->stop_count.fetch_add(1);
                return p.stop(atom("normal"));
            }
        }
    });
}

// Helper untuk crash worker: kirim pesan "crash"
inline void crash_worker(Runtime& rt, Pid worker_pid) {
    if (worker_pid.valid()) {
        rt.send(Pid{}, worker_pid, atom("crash"));
    }
}

int main() {
    std::cout << "========================================\n";
    std::cout << "FASE G1.2 - GARUDA SUPERVISOR TESTS\n";
    std::cout << "========================================\n\n";

    // =================================================================
    // Test 1: OneForOne — crash 1 child, hanya dia yang restart
    // =================================================================
    std::cout << "[Test 1] OneForOne strategy...\n";
    {
        Runtime rt;
        SupervisorFlags flags;
        flags.strategy = Strategy::OneForOne;
        flags.max_restarts = 10;
        flags.period_ms = 5000;

        auto s1 = std::make_shared<WorkerState>();
        auto s2 = std::make_shared<WorkerState>();
        auto s3 = std::make_shared<WorkerState>();

        std::vector<ChildSpec> specs = {
            {"worker_1", [s1](Process& sup) { return spawn_test_worker(sup, s1); }, Restart::Permanent},
            {"worker_2", [s2](Process& sup) { return spawn_test_worker(sup, s2); }, Restart::Permanent},
            {"worker_3", [s3](Process& sup) { return spawn_test_worker(sup, s3); }, Restart::Permanent},
        };

        Pid sup = start_supervisor(rt, flags, specs);
        CHECK(sup.valid(), "Supervisor harus valid");
        rt.run_until_idle();

        CHECK(s1->start_count.load() == 1, "Worker 1 start 1x");
        CHECK(s2->start_count.load() == 1, "Worker 2 start 1x");
        CHECK(s3->start_count.load() == 1, "Worker 3 start 1x");

        // Crash worker 2
        crash_worker(rt, s2->pid);
        rt.run_until_idle();

        // Verifikasi: worker 1 & 3 tidak restart, worker 2 restart
        CHECK(s1->start_count.load() == 1, "Worker 1 TIDAK restart (OneForOne)");
        CHECK(s2->start_count.load() == 2, "Worker 2 restart 1x (jadi total 2x)");
        CHECK(s3->start_count.load() == 1, "Worker 3 TIDAK restart (OneForOne)");
    }
    std::cout << "PASS\n\n";

    // =================================================================
    // Test 2: OneForAll — crash 1 child, semua restart
    // =================================================================
    std::cout << "[Test 2] OneForAll strategy...\n";
    {
        Runtime rt;
        SupervisorFlags flags;
        flags.strategy = Strategy::OneForAll;
        flags.max_restarts = 10;
        flags.period_ms = 5000;

        auto s1 = std::make_shared<WorkerState>();
        auto s2 = std::make_shared<WorkerState>();
        auto s3 = std::make_shared<WorkerState>();

        std::vector<ChildSpec> specs = {
            {"worker_1", [s1](Process& sup) { return spawn_test_worker(sup, s1); }, Restart::Permanent},
            {"worker_2", [s2](Process& sup) { return spawn_test_worker(sup, s2); }, Restart::Permanent},
            {"worker_3", [s3](Process& sup) { return spawn_test_worker(sup, s3); }, Restart::Permanent},
        };

        Pid sup = start_supervisor(rt, flags, specs);
        rt.run_until_idle();

        CHECK(s1->start_count.load() == 1, "Initial: worker 1 start 1x");
        CHECK(s2->start_count.load() == 1, "Initial: worker 2 start 1x");
        CHECK(s3->start_count.load() == 1, "Initial: worker 3 start 1x");

        // Crash worker 2
        crash_worker(rt, s2->pid);
        rt.run_until_idle();

        // Verifikasi: SEMUA worker restart (OneForAll)
        CHECK(s1->start_count.load() == 2, "Worker 1 restart (OneForAll)");
        CHECK(s2->start_count.load() == 2, "Worker 2 restart (OneForAll)");
        CHECK(s3->start_count.load() == 2, "Worker 3 restart (OneForAll)");
    }
    std::cout << "PASS\n\n";

    // =================================================================
    // Test 3: RestForOne — crash child ke-2, child 2 & 3 restart
    // =================================================================
    std::cout << "[Test 3] RestForOne strategy...\n";
    {
        Runtime rt;
        SupervisorFlags flags;
        flags.strategy = Strategy::RestForOne;
        flags.max_restarts = 10;
        flags.period_ms = 5000;

        auto s1 = std::make_shared<WorkerState>();
        auto s2 = std::make_shared<WorkerState>();
        auto s3 = std::make_shared<WorkerState>();

        std::vector<ChildSpec> specs = {
            {"worker_1", [s1](Process& sup) { return spawn_test_worker(sup, s1); }, Restart::Permanent},
            {"worker_2", [s2](Process& sup) { return spawn_test_worker(sup, s2); }, Restart::Permanent},
            {"worker_3", [s3](Process& sup) { return spawn_test_worker(sup, s3); }, Restart::Permanent},
        };

        Pid sup = start_supervisor(rt, flags, specs);
        rt.run_until_idle();

        CHECK(s1->start_count.load() == 1, "Initial: worker 1 start 1x");
        CHECK(s2->start_count.load() == 1, "Initial: worker 2 start 1x");
        CHECK(s3->start_count.load() == 1, "Initial: worker 3 start 1x");

        // Crash worker 2 (index 1)
        crash_worker(rt, s2->pid);
        rt.run_until_idle();

        // Verifikasi: worker 1 TETAP, worker 2 & 3 restart
        CHECK(s1->start_count.load() == 1, "Worker 1 TIDAK restart (RestForOne)");
        CHECK(s2->start_count.load() == 2, "Worker 2 restart (RestForOne)");
        CHECK(s3->start_count.load() == 2, "Worker 3 restart (RestForOne)");
    }
    std::cout << "PASS\n\n";

    // =================================================================
    // Test 4: MaxRestarts — crash terus-menerus, supervisor mati
    // =================================================================
    std::cout << "[Test 4] MaxRestarts...\n";
    {
        Runtime rt;
        SupervisorFlags flags;
        flags.strategy = Strategy::OneForOne;
        flags.max_restarts = 3;  // hanya boleh 3 restart dalam 5 detik
        flags.period_ms = 5000;

        auto s1 = std::make_shared<WorkerState>();

        std::vector<ChildSpec> specs = {
            {"worker_1", [s1](Process& sup) { return spawn_test_worker(sup, s1); }, Restart::Permanent},
        };

        Pid sup = start_supervisor(rt, flags, specs);
        rt.run_until_idle();
        CHECK(s1->start_count.load() == 1, "Initial: worker start 1x");

        // Crash 4 kali (melebihi max_restarts=3)
        for (int i = 0; i < 4; ++i) {
            crash_worker(rt, s1->pid);
            rt.run_until_idle();
        }

        // Verifikasi: supervisor berhenti setelah max_restarts (worker start <= 4x)
        CHECK(s1->start_count.load() <= 4, "Supervisor berhenti setelah max_restarts (worker start <= 4x)");
    }
    std::cout << "PASS\n\n";

    // =================================================================
    // Test 5: Restart types — Permanent, Transient, Temporary
    // =================================================================
    std::cout << "[Test 5] Restart types (Permanent/Transient/Temporary)...\n";
    {
        Runtime rt;
        SupervisorFlags flags;
        flags.strategy = Strategy::OneForOne;
        flags.max_restarts = 10;
        flags.period_ms = 5000;

        auto s_perm = std::make_shared<WorkerState>();
        auto s_trans = std::make_shared<WorkerState>();
        auto s_temp = std::make_shared<WorkerState>();

        std::vector<ChildSpec> specs = {
            {"permanent_worker", [s_perm](Process& sup) { return spawn_test_worker(sup, s_perm); }, Restart::Permanent},
            {"transient_worker", [s_trans](Process& sup) { return spawn_test_worker(sup, s_trans); }, Restart::Transient},
            {"temporary_worker", [s_temp](Process& sup) { return spawn_test_worker(sup, s_temp); }, Restart::Temporary},
        };

        Pid sup = start_supervisor(rt, flags, specs);
        rt.run_until_idle();

        CHECK(s_perm->start_count.load() == 1, "Permanent start 1x");
        CHECK(s_trans->start_count.load() == 1, "Transient start 1x");
        CHECK(s_temp->start_count.load() == 1, "Temporary start 1x");

        // Crash temporary worker — tidak boleh restart
        crash_worker(rt, s_temp->pid);
        rt.run_until_idle();
        CHECK(s_temp->start_count.load() == 1, "Temporary TIDAK restart saat crash");

        // Crash transient worker (abnormal) — HARUS restart
        crash_worker(rt, s_trans->pid);
        rt.run_until_idle();
        CHECK(s_trans->start_count.load() == 2, "Transient RESTART saat abnormal crash");

        // Crash permanent worker — HARUS restart
        crash_worker(rt, s_perm->pid);
        rt.run_until_idle();
        CHECK(s_perm->start_count.load() == 2, "Permanent RESTART saat crash");
    }
    std::cout << "PASS\n\n";

    std::cout << "========================================\n";
    std::cout << "FASE G1.2 - SUPERVISOR SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
