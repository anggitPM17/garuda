// =====================================================================
// GARUDA COMPILER — FASE 17: JAVASCRIPT & WASM FFI SUITE
// =====================================================================

#include "../include/garuda/beam/ffi/js_wasm_ffi.hpp"
#include <iostream>
#include <cassert>

using namespace garuda::beam;

int main() {
    int failures = 0;

    std::cout << "=======================================================\n";
    std::cout << "GARUDA COMPILER — FASE 17: JAVASCRIPT & WASM FFI SUITE\n";
    std::cout << "=======================================================\n\n";

    JsWasmFfiBridge js;

    // Test 1: JS FFI Active
    if (js.is_active()) {
        std::cout << "[Test 1] JS/WASM FFI Active -> [PASS]\n";
    } else {
        std::cout << "[Test 1] JS FFI -> [FAIL]\n";
        failures++;
    }

    // Test 2: Eval JS Code
    auto js_val = js.eval_js("Math.sqrt(10000)");
    if (js_val.type == "number" && js_val.number_value == 100.0) {
        std::cout << "[Test 2] Eval JS Code -> Result 100.0 [PASS]\n";
    } else {
        std::cout << "[Test 2] Eval JS Code -> [FAIL]\n";
        failures++;
    }

    // Test 3: Call JS Function with Arguments
    JsValueMock arg; arg.type = "number"; arg.number_value = 14.0;
    auto call_res = js.call_js_func("triple_val", {arg});
    if (call_res.type == "number" && call_res.number_value == 42.0) {
        std::cout << "[Test 3] Call JS Function with Args -> Result 42.0 [PASS]\n";
    } else {
        std::cout << "[Test 3] Call JS Function -> [FAIL]\n";
        failures++;
    }

    // Test 4: WebAssembly Invoke
    auto wasm_res = js.wasm_invoke("math_module", "add_nums", {10.0, 31.0});
    if (wasm_res.type == "number" && wasm_res.number_value == 42.0) {
        std::cout << "[Test 4] WebAssembly Exported Function Invoke -> Result 42.0 [PASS]\n";
    } else {
        std::cout << "[Test 4] WASM Invoke -> [FAIL]\n";
        failures++;
    }

    if (failures > 0) {
        std::cout << "\n❌ " << failures << " JS/WASM FFI TESTS FAILED!\n";
        return 1;
    }

    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL FASE 17 JS/WASM FFI TESTS PASSED PERFECTLY!\n";
    std::cout << "========================================\n";
    return 0;
}
