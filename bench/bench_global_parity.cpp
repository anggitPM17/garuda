#include "../include/garuda/beam/global/phoenix_framework.hpp"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <iostream>
#include <vector>

using namespace garuda::beam::global;

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA FASE H — GLOBAL PARITY & ECOSYSTEM SCALE BENCHMARK\n";
    std::cout << "=======================================================\n\n";

    // Benchmark 1: Hex Package Resolution Throughput
    HexRegistry hex;
    std::size_t sample_hex = 1000;
    std::cout << "[BENCH 1] Executing " << sample_hex << " Hex Package Search Operations...\n";

    for (std::size_t i = 0; i < sample_hex; ++i) {
        hex.publish({"pkg_" + std::to_string(i), "1.0.0", "Package " + std::to_string(i)});
    }

    auto start_hex = std::chrono::high_resolution_clock::now();
    for (std::size_t i = 0; i < sample_hex; ++i) {
        hex.search("pkg_" + std::to_string(i), "1.0.0");
    }
    auto elapsed_hex_ms = std::chrono::duration<double, std::milli>(
        std::chrono::high_resolution_clock::now() - start_hex).count();

    double hex_throughput = (sample_hex / (elapsed_hex_ms / 1000.0));
    std::cout << "  ⏱️ Hex Resolution Throughput: " << hex_throughput << " pkg/sec (Target >= 1000 pkg/sec!)\n\n";

    // Benchmark 2: Mix Build Tool Latency
    MixBuildTool mix;
    std::cout << "[BENCH 2] Executing Mix Compile & Test Cycle...\n";
    auto start_mix = std::chrono::high_resolution_clock::now();
    mix.deps_get();
    mix.compile();
    mix.test();
    auto elapsed_mix_ms = std::chrono::duration<double, std::milli>(
        std::chrono::high_resolution_clock::now() - start_mix).count();

    std::cout << "  ⏱️ Mix Build Latency: " << elapsed_mix_ms << " ms (Target <= 100 ms!)\n\n";

    // Benchmark 3: IEx REPL Evaluation Latency
    IExRepl iex;
    std::size_t sample_iex = 1000;
    std::cout << "[BENCH 3] Executing " << sample_iex << " IEx REPL Evaluations...\n";

    std::vector<double> iex_latencies_us;
    iex_latencies_us.reserve(sample_iex);

    for (std::size_t i = 0; i < sample_iex; ++i) {
        auto t0 = std::chrono::high_resolution_clock::now();
        iex.eval("1 + 2");
        auto t1 = std::chrono::high_resolution_clock::now();
        iex_latencies_us.push_back(std::chrono::duration<double, std::micro>(t1 - t0).count());
    }

    std::sort(iex_latencies_us.begin(), iex_latencies_us.end());
    double iex_p50 = iex_latencies_us[static_cast<std::size_t>(iex_latencies_us.size() * 0.50)];
    std::cout << "  ⏱️ Measured IEx REPL Eval Latency p50: " << (iex_p50 / 1000.0) << " ms (Target <= 10 ms!)\n\n";

    // Benchmark 4: ExDoc HTML Documentation Generation Speed
    ExDocGenerator exdoc;
    std::cout << "[BENCH 4] Executing 100 Module HTML Documentation Generation...\n";
    auto start_exdoc = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 100; ++i) {
        exdoc.generate_html("Garuda.Module" + std::to_string(i), "Documentation string for module " + std::to_string(i));
    }
    auto elapsed_exdoc_sec = std::chrono::duration<double>(
        std::chrono::high_resolution_clock::now() - start_exdoc).count();

    std::cout << "  ⏱️ ExDoc HTML Generation Time: " << elapsed_exdoc_sec << " sec (Target <= 5.0 sec!)\n\n";

    std::cout << "🎉 ALL FASE H GLOBAL PARITY BENCHMARKS EXECUTED SUCCESSFULLY!\n";
    return 0;
}
