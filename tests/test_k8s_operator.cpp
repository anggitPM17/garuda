// =====================================================================
// GARUDA COMPILER — FASE 20 KUBERNETES OPERATOR & CRDT SUITE (5 TESTS)
// =====================================================================

#include "../include/garuda/beam/cloud/k8s_operator.hpp"
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
    std::cout << "GARUDA COMPILER — FASE 20 KUBERNETES OPERATOR & CRDT SUITE\n";
    std::cout << "=======================================================\n\n";

    // === Test 1: CRD Spec Parsing & Initialization ===
    std::cout << "[TEST 1/5] Running CRD Spec Initialization test...\n";
    GarudaClusterSpec spec{"garuda-prod-asia", 3, 2, 10, 70.0, 500};
    CHECK(spec.cluster_name == "garuda-prod-asia", "Cluster name must match spec");
    CHECK(spec.replicas == 3, "Initial replicas must be 3");
    std::cout << "  ✓ Test 1: CRD Spec Initialization PASS\n";

    // === Test 2: HPA AutoScaler Scale-Out Evaluation ===
    std::cout << "[TEST 2/5] Running HPA Scale-Out test...\n";
    HpaAutoScaler scaler(spec);
    std::vector<NodeMetrics> high_load_metrics = {
        {"node-1", 85.0, 1024.0, 600},
        {"node-2", 90.0, 2048.0, 700},
        {"node-3", 80.0, 1536.0, 550}
    };

    int new_replicas = scaler.evaluate_scale(high_load_metrics);
    CHECK(new_replicas > 3, "Scale-out must increase replicas from 3 to 5");
    CHECK(scaler.spec().replicas == 5, "Scaler spec replicas must update to 5");
    std::cout << "  ✓ Test 2: HPA Scale-Out PASS (New Replicas: " << new_replicas << ")\n";

    // === Test 3: HPA AutoScaler Scale-In Evaluation ===
    std::cout << "[TEST 3/5] Running HPA Scale-In test...\n";
    std::vector<NodeMetrics> low_load_metrics = {
        {"node-1", 10.0, 256.0, 20},
        {"node-2", 15.0, 512.0, 30},
        {"node-3", 12.0, 384.0, 25},
        {"node-4", 18.0, 400.0, 15},
        {"node-5", 14.0, 300.0, 10}
    };

    int scale_in_replicas = scaler.evaluate_scale(low_load_metrics);
    CHECK(scale_in_replicas < 5, "Scale-in must decrease replicas from 5 to 4");
    std::cout << "  ✓ Test 3: HPA Scale-In PASS (New Replicas: " << scale_in_replicas << ")\n";

    // === Test 4: Multi-Region CRDT G-Counter & PN-Counter Merge ===
    std::cout << "[TEST 4/5] Running Multi-Region CRDT Counter test...\n";
    PNCounter node_jakarta("node-cgk");
    PNCounter node_singapore("node-sin");

    node_jakarta.increment(10);
    node_jakarta.decrement(2); // Net = +8

    node_singapore.increment(15);
    node_singapore.decrement(3); // Net = +12

    node_jakarta.merge(node_singapore);
    CHECK(node_jakarta.value() == 20, "Merged PN-Counter value must equal 20 (8 + 12)");
    std::cout << "  ✓ Test 4: Multi-Region CRDT Counter PASS (Merged Value: " << node_jakarta.value() << ")\n";

    // === Test 5: Multi-Region CRDT LWW-Register Resolution ===
    std::cout << "[TEST 5/5] Running CRDT LWW-Register Resolution test...\n";
    LWWRegister<std::string> reg1("Old State - Version 1", 1000);
    LWWRegister<std::string> reg2("New State - Version 2", 2000);

    reg1.merge(reg2);
    CHECK(reg1.value() == "New State - Version 2", "LWW-Register must resolve to highest timestamp value");
    CHECK(reg1.timestamp() == 2000, "LWW-Register timestamp must update to 2000");
    std::cout << "  ✓ Test 5: CRDT LWW-Register Resolution PASS\n";

    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL 5 KUBERNETES OPERATOR & CRDT TESTS PASSED PERFECTLY!\n";
    std::cout << "========================================\n";
    return 0;
}
