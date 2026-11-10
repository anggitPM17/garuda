// =====================================================================
// GARUDA COMPILER — FASE 21 DEEP LEARNING GPU ACCELERATOR SUITE (5 TESTS)
// =====================================================================

#include "../include/garuda/beam/ml/gpu_accelerator.hpp"
#include <cassert>
#include <iostream>

using namespace garuda::beam::ml;

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        std::cerr << "[FAIL] " << msg << "\n"; \
        return 1; \
    } \
} while(0)

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA COMPILER — FASE 21 DEEP LEARNING GPU ACCELERATOR SUITE\n";
    std::cout << "=======================================================\n\n";

    // === Test 1: GPU Device Detection & VRAM Query ===
    std::cout << "[TEST 1/5] Running GPU Device Detection test...\n";
    auto& dev_mgr = GpuDeviceManager::instance();
    CHECK(dev_mgr.is_gpu_available(), "GPU device must be available");

    auto dev_info = dev_mgr.get_device_info();
    CHECK(!dev_info.device_name.empty(), "GPU device name must not be empty");
    CHECK(dev_info.total_memory_bytes >= 1024 * 1024 * 1024, "VRAM must be >= 1GB");
    std::cout << "  ✓ Test 1: GPU Device Detection PASS (Device: " << dev_info.device_name << ")\n";

    // === Test 2: Host to Device GpuTensor Transfer ===
    std::cout << "[TEST 2/5] Running Host to Device GpuTensor Transfer test...\n";
    Tensor cpu_tensor({2, 2}, {1.0, 2.0, 3.0, 4.0});
    GpuTensor gpu_tensor = GpuTensor::from_host(cpu_tensor);

    CHECK(gpu_tensor.rows() == 2 && gpu_tensor.cols() == 2, "GpuTensor shape must match Host Tensor");
    Tensor restored_cpu = gpu_tensor.to_host();
    CHECK(restored_cpu.at(0, 1) == 2.0 && restored_cpu.at(1, 0) == 3.0, "Host-Device-Host transfer must preserve exact values");
    std::cout << "  ✓ Test 2: Host to Device GpuTensor Transfer PASS\n";

    // === Test 3: Parallel GPU Matrix Multiplication Kernel ===
    std::cout << "[TEST 3/5] Running Parallel GPU Matmul Kernel test...\n";
    GpuTensor gpu_a({2, 3}, {1.0, 2.0, 3.0, 4.0, 5.0, 6.0});
    GpuTensor gpu_b({3, 2}, {7.0, 8.0, 9.0, 1.0, 2.0, 3.0});

    GpuTensor gpu_c = gpu_a.gpu_matmul(gpu_b); // 2x2 result
    Tensor host_c = gpu_c.to_host();

    CHECK(host_c.rows() == 2 && host_c.cols() == 2, "GPU Matmul output shape must be 2x2");
    CHECK(host_c.at(0, 0) == 31.0, "GPU Matmul C(0,0) must equal 31.0");
    std::cout << "  ✓ Test 3: Parallel GPU Matmul Kernel PASS\n";

    // === Test 4: Parallel GPU Vector Add Kernel ===
    std::cout << "[TEST 4/5] Running Parallel GPU Add Kernel test...\n";
    GpuTensor gpu_x({1, 4}, {10.0, 20.0, 30.0, 40.0});
    GpuTensor gpu_y({1, 4}, {1.0, 2.0, 3.0, 4.0});

    GpuTensor gpu_z = gpu_x.gpu_add(gpu_y);
    Tensor host_z = gpu_z.to_host();

    CHECK(host_z.get(0) == 11.0 && host_z.get(3) == 44.0, "GPU Add Kernel result must match");
    std::cout << "  ✓ Test 4: Parallel GPU Add Kernel PASS\n";

    // === Test 5: Parallel GPU Activation Kernel (ReLU) ===
    std::cout << "[TEST 5/5] Running Parallel GPU Activation Kernel test...\n";
    GpuTensor gpu_raw({1, 4}, {-5.0, 0.0, 2.5, 10.0});
    GpuTensor gpu_relu_out = gpu_raw.gpu_relu();
    Tensor host_relu = gpu_relu_out.to_host();

    CHECK(host_relu.get(0) == 0.0 && host_relu.get(2) == 2.5, "GPU ReLU Kernel result must match");
    std::cout << "  ✓ Test 5: Parallel GPU Activation Kernel PASS\n";

    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL 5 DEEP LEARNING GPU ACCELERATOR TESTS PASSED PERFECTLY!\n";
    std::cout << "========================================\n";
    return 0;
}
