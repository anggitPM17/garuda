#include "../include/garuda/beam/core/runtime.hpp"
#include <chrono>
#include <cstdio>
#include <iostream>

using namespace garuda::beam;
using Clock = std::chrono::steady_clock;

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA ACTOR SPAWN THROUGHPUT BENCHMARK\n";
    std::cout << "=======================================================\n\n";

    Runtime rt;
    auto start = Clock::now();
    for (int b = 0; b < 100; ++b) {
        for (int i = 0; i < 10000; ++i) {
            rt.spawn([](Process&) { return Status::Done; });
        }
        rt.run_until_idle();
    }
    double elapsed_sec = std::chrono::duration<double>(Clock::now() - start).count();
    double throughput = 1000000.0 / (elapsed_sec > 0 ? elapsed_sec : 0.001);

    std::printf("spawn+exit 1.000.000 proses : %.2f dtk  (%.0f processes/sec)\n", elapsed_sec, throughput);
    std::printf("Spawn Throughput: %.0f processes/sec\n", throughput);

    return 0;
}
