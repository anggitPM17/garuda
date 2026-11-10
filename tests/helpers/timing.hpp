#pragma once
// Garuda Test Helpers: Timing & Robust Polling (`timing.hpp`)

#include <chrono>
#include <thread>

namespace garuda::test::helpers {

template <typename Predicate>
bool wait_until(Predicate pred, int timeout_ms = 5000, int poll_ms = 10) {
    auto start = std::chrono::steady_clock::now();
    while (std::chrono::steady_clock::now() - start < std::chrono::milliseconds(timeout_ms)) {
        if (pred()) return true;
        std::this_thread::sleep_for(std::chrono::milliseconds(poll_ms));
    }
    return pred();
}

} // namespace garuda::test::helpers
