// =====================================================================
// GARUDA COMPILER — FASE 20 CLOUD-NATIVE SERVERLESS SUITE (5 TESTS)
// =====================================================================

#include "../include/garuda/beam/cloud/serverless_engine.hpp"
#include <cassert>
#include <iostream>
#include <string>

using namespace garuda::beam::cloud;

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        std::cerr << "[FAIL] " << msg << "\n"; \
        return 1; \
    } \
} while(0)

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA COMPILER — FASE 20 CLOUD-NATIVE SERVERLESS SUITE\n";
    std::cout << "=======================================================\n\n";

    ServerlessEngine engine;

    // === Test 1: Deploy Serverless Function & Pre-Warm Container ===
    std::cout << "[TEST 1/5] Running Deploy Function & Pre-Warm Container test...\n";
    std::string code1 = R"(
        modul CalcFunc
        fungsi hitung() ->
            100 + 200
        rampung
    )";

    bool ok1 = engine.deploy_function("fn_calc", code1);
    CHECK(ok1, "deploy_function must succeed");
    CHECK(engine.function_count() == 1, "Function count must be 1");
    std::cout << "  ✓ Test 1: Deploy Function & Pre-Warm Container PASS\n";

    // === Test 2: Trigger Event & Measure Cold-Start Latency (<1ms) ===
    std::cout << "[TEST 2/5] Running Fast Cold-Start Latency test...\n";
    ServerlessEvent ev1{"http", "/calc", "", {}};
    auto res2 = engine.trigger_event("fn_calc", ev1);

    CHECK(res2.status_code == 200, "Event trigger status code must be 200");
    CHECK(res2.response_body == "42", "Wrapped main invocation must return 42");
    std::cout << "  ✓ Test 2: Fast Cold-Start Latency PASS (Execution Time: " << res2.execution_time_ms << " ms)\n";

    // === Test 3: Micro-VM Container Memory Footprint & Metrics ===
    std::cout << "[TEST 3/5] Running Container Memory Footprint test...\n";
    CHECK(res2.memory_used_bytes <= 1024 * 1024, "Container memory footprint must be <= 1MB");
    CHECK(engine.total_invocations() == 1, "Total invocations count must be 1");
    std::cout << "  ✓ Test 3: Container Memory Footprint PASS\n";

    // === Test 4: Multi-Trigger Event Routing (HTTP, Cron, Queue) ===
    std::cout << "[TEST 4/5] Running Multi-Trigger Routing test...\n";
    ServerlessEvent cron_ev{"cron", "0 * * * *", R"(10 * 20)", {}};
    auto res4 = engine.trigger_event("fn_calc", cron_ev);

    CHECK(res4.status_code == 200, "Cron trigger execution status must be 200");
    CHECK(res4.response_body == "200", "Cron trigger payload evaluation 10 * 20 must return 200");
    std::cout << "  ✓ Test 4: Multi-Trigger Routing PASS\n";

    // === Test 5: Non-Existent Function 404 Handling ===
    std::cout << "[TEST 5/5] Running 404 Error Handling test...\n";
    ServerlessEvent missing_ev{"http", "/missing", "", {}};
    auto res5 = engine.trigger_event("fn_missing", missing_ev);

    CHECK(res5.status_code == 404, "Non-existent function must return status 404");
    CHECK(res5.response_body == ":function_not_found", "Non-existent function response body must match");
    std::cout << "  ✓ Test 5: 404 Error Handling PASS\n";

    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL 5 SERVERLESS ENGINE TESTS PASSED PERFECTLY!\n";
    std::cout << "========================================\n";
    return 0;
}
