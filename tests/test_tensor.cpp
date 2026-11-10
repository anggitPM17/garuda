// =====================================================================
// GARUDA COMPILER — FASE 19 MACHINE LEARNING TENSOR SUITE (5 TESTS)
// =====================================================================

#include "../include/garuda/beam/ml/ml_primitives.hpp"
#include <cassert>
#include <cmath>
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
    std::cout << "GARUDA COMPILER — FASE 19 MACHINE LEARNING TENSOR SUITE\n";
    std::cout << "=======================================================\n\n";

    // === Test 1: Tensor Instantiation & Reshape ===
    std::cout << "[TEST 1/5] Running Tensor Instantiation & Reshape test...\n";
    Tensor t1({2, 3}, {1.0, 2.0, 3.0, 4.0, 5.0, 6.0});
    CHECK(t1.rows() == 2 && t1.cols() == 3, "Tensor 1 shape must be 2x3");
    CHECK(t1.at(1, 0) == 4.0, "Element at (1,0) must be 4.0");

    Tensor t1_reshaped = t1.reshape({3, 2});
    CHECK(t1_reshaped.rows() == 3 && t1_reshaped.cols() == 2, "Reshaped tensor shape must be 3x2");
    std::cout << "  ✓ Test 1: Tensor Instantiation & Reshape PASS\n";

    // === Test 2: Matrix Multiplication (Matmul) & Transpose ===
    std::cout << "[TEST 2/5] Running Matrix Multiplication & Transpose test...\n";
    Tensor A({2, 3}, {1.0, 2.0, 3.0, 4.0, 5.0, 6.0}); // 2x3
    Tensor B({3, 2}, {7.0, 8.0, 9.0, 1.0, 2.0, 3.0}); // 3x2

    Tensor C = A.matmul(B); // Result 2x2
    CHECK(C.rows() == 2 && C.cols() == 2, "Matmul result shape must be 2x2");
    // Row 0: 1*7 + 2*9 + 3*2 = 7 + 18 + 6 = 31.0
    CHECK(C.at(0, 0) == 31.0, "Matmul C(0,0) must be 31.0");

    Tensor A_t = A.transpose();
    CHECK(A_t.rows() == 3 && A_t.cols() == 2, "Transposed A must be 3x2");
    CHECK(A_t.at(0, 1) == 4.0, "Transposed A(0,1) must be 4.0");
    std::cout << "  ✓ Test 2: Matrix Multiplication & Transpose PASS\n";

    // === Test 3: Activation Functions (ReLU & Sigmoid) ===
    std::cout << "[TEST 3/5] Running Activation Functions test...\n";
    Tensor raw({1, 4}, {-2.0, 0.0, 1.0, 3.0});

    Tensor relu_out = raw.relu();
    CHECK(relu_out.get(0) == 0.0, "ReLU(-2) must be 0.0");
    CHECK(relu_out.get(3) == 3.0, "ReLU(3) must be 3.0");

    Tensor sig_out = raw.sigmoid();
    CHECK(std::abs(sig_out.get(1) - 0.5) < 0.001, "Sigmoid(0) must be 0.5");
    std::cout << "  ✓ Test 3: Activation Functions PASS\n";

    // === Test 4: Softmax Normalization ===
    std::cout << "[TEST 4/5] Running Softmax Normalization test...\n";
    Tensor logits({1, 3}, {1.0, 2.0, 3.0});
    Tensor probs = logits.softmax();

    double sum = 0.0;
    for (std::size_t i = 0; i < probs.size(); ++i) {
        sum += probs.get(i);
    }
    CHECK(std::abs(sum - 1.0) < 0.0001, "Softmax probabilities sum must equal 1.0");
    CHECK(probs.get(2) > probs.get(1) && probs.get(1) > probs.get(0), "Softmax output must preserve rank order");
    std::cout << "  ✓ Test 4: Softmax Normalization PASS\n";

    // === Test 5: Neural Network Training Step (SGD) ===
    std::cout << "[TEST 5/5] Running Neural Network Training Step test...\n";
    NeuralNetworkEngine nn(2, 1);
    Tensor x({1, 2}, {1.0, 2.0});
    Tensor y_true({1, 1}, {1.0});

    double initial_loss = nn.train_step(x, y_true, 0.05);
    double loss_after = initial_loss;
    for (int step = 0; step < 10; ++step) {
        loss_after = nn.train_step(x, y_true, 0.05);
    }

    CHECK(loss_after <= initial_loss, "Training loss must decrease after SGD steps");
    std::cout << "  ✓ Test 5: Neural Network Training Step PASS\n";

    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL 5 MACHINE LEARNING TENSOR TESTS PASSED PERFECTLY!\n";
    std::cout << "========================================\n";
    return 0;
}
