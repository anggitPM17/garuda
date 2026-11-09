// =====================================================================
// GARUDA COMPILER — FASE 16: ADVANCED CONCURRENCY PRIMITIVES TEST SUITE
// =====================================================================

#include "../include/garuda/beam/core/concurrency_primitives.hpp"
#include <iostream>
#include <cassert>
#include <vector>

using namespace garuda::beam;

int main() {
    int failures = 0;

    std::cout << "=======================================================\n";
    std::cout << "GARUDA COMPILER — FASE 16: CONCURRENCY PRIMITIVES SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test 1: Mutex Lock & TryLock
    {
        Mutex m;
        m.kunci();
        bool try_res = m.coba_kunci(); // Should return false because already locked
        m.buka();
        if (!try_res) {
            std::cout << "[Test 1] Mutex Lock & TryLock -> Correctly Blocked [PASS]\n";
        } else {
            std::cout << "[Test 1] Mutex Lock & TryLock -> [FAIL]\n";
            failures++;
        }
    }

    // Test 2: Semaphore Wait & Release
    {
        Semaphore sem(1);
        sem.tunggu();
        bool signaled = false;
        std::thread t([&]() {
            sem.tunggu();
            signaled = true;
        });
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        sem.lepas();
        if (t.joinable()) t.join();
        if (signaled) {
            std::cout << "[Test 2] Semaphore Wait & Release -> Signaled [PASS]\n";
        } else {
            std::cout << "[Test 2] Semaphore Wait & Release -> [FAIL]\n";
            failures++;
        }
    }

    // Test 3: CSP Channel Send & Receive
    {
        Channel<std::string> ch(5);
        ch.kirim("pesan_1");
        ch.kirim("pesan_2");

        auto r1 = ch.terima();
        auto r2 = ch.terima();

        if (r1.has_value() && *r1 == "pesan_1" && r2.has_value() && *r2 == "pesan_2") {
            std::cout << "[Test 3] CSP Channel Send & Receive FIFO -> [PASS]\n";
        } else {
            std::cout << "[Test 3] CSP Channel Send & Receive -> [FAIL]\n";
            failures++;
        }
    }

    // Test 4: Promise & Future Await
    {
        Promise<std::string> promise;
        std::thread t([&]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            promise.selesai("hasil_future");
        });
        std::string res = promise.tunggu();
        if (t.joinable()) t.join();

        if (res == "hasil_future") {
            std::cout << "[Test 4] Promise & Future Await -> 'hasil_future' [PASS]\n";
        } else {
            std::cout << "[Test 4] Promise & Future Await -> [FAIL]\n";
            failures++;
        }
    }

    // Test 5: Asynchronous Task Spawn & Join
    {
        Task task = Task::spawn([]() -> Term {
            return Term(std::string("task_sukses"));
        });

        Term result = task.join();
        if (auto* str = result.get_if<std::string>()) {
            if (*str == "task_sukses") {
                std::cout << "[Test 5] Task Spawn & Join -> 'task_sukses' [PASS]\n";
            } else {
                std::cout << "[Test 5] Task Spawn & Join -> [FAIL] got: " << *str << "\n";
                failures++;
            }
        } else {
            std::cout << "[Test 5] Task Spawn & Join -> [FAIL]\n";
            failures++;
        }
    }

    if (failures > 0) {
        std::cout << "\n❌ " << failures << " CONCURRENCY TESTS FAILED!\n";
        return 1;
    }

    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL FASE 16 CONCURRENCY TESTS PASSED PERFECTLY!\n";
    std::cout << "========================================\n";
    return 0;
}
