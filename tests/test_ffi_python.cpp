// =====================================================================
// GARUDA COMPILER — FASE 17: PYTHON FFI INTEROPERABILITY SUITE
// =====================================================================

#include "../include/garuda/beam/ffi/python_ffi.hpp"
#include <iostream>
#include <cassert>

using namespace garuda::beam;

int main() {
    int failures = 0;

    std::cout << "=======================================================\n";
    std::cout << "GARUDA COMPILER — FASE 17: PYTHON FFI SUITE\n";
    std::cout << "=======================================================\n\n";

    PythonFfiBridge py;

    // Test 1: Python Bridge Initialization
    if (py.is_initialized()) {
        std::cout << "[Test 1] Python FFI Bridge Initialized -> [PASS]\n";
    } else {
        std::cout << "[Test 1] Python FFI Bridge -> [FAIL]\n";
        failures++;
    }

    // Test 2: Eval Python Script
    auto py_res = py.eval_script("x = 10 + 32");
    if (py_res.type == "int" && py_res.int_val == 42) {
        std::cout << "[Test 2] Eval Python Script -> Result 42 [PASS]\n";
    } else {
        std::cout << "[Test 2] Eval Python Script -> [FAIL]\n";
        failures++;
    }

    // Test 3: Call Python Function with Arguments
    PyObjectMock arg1; arg1.type = "int"; arg1.int_val = 21;
    auto call_res = py.call_function("double_it", {arg1});
    if (call_res.type == "int" && call_res.int_val == 42) {
        std::cout << "[Test 3] Call Python Function with Args -> Result 42 [PASS]\n";
    } else {
        std::cout << "[Test 3] Call Python Function -> [FAIL]\n";
        failures++;
    }

    // Test 4: Term Conversion (Python PyObject -> Garuda Term)
    Term term = py.to_garuda_term(py_res);
    if (auto* i = term.get_if<int64_t>()) {
        if (*i == 42) {
            std::cout << "[Test 4] PyObject to Garuda Term Conversion -> 42 [PASS]\n";
        } else {
            std::cout << "[Test 4] Term Conversion -> [FAIL]\n";
            failures++;
        }
    } else {
        std::cout << "[Test 4] Term Conversion -> [FAIL]\n";
        failures++;
    }

    if (failures > 0) {
        std::cout << "\n❌ " << failures << " PYTHON FFI TESTS FAILED!\n";
        return 1;
    }

    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL FASE 17 PYTHON FFI TESTS PASSED PERFECTLY!\n";
    std::cout << "========================================\n";
    return 0;
}
