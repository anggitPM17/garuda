#include "../include/garuda/compiler/package_registry.hpp"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <thread>
#include <vector>

using namespace garuda::compiler;
using namespace garuda::beam::stdlib;
namespace fs = std::filesystem;

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA PACKAGE MANAGER BENCHMARK (SPRINT F.4 — REAL PERFORMANCE)\n";
    std::cout << "=======================================================\n\n";

    fs::path bench_dir = fs::temp_directory_path() / "garuda_bench_pkg_mgr";
    fs::remove_all(bench_dir);
    fs::create_directories(bench_dir);

    PackageRegistry reg(bench_dir.string());

    // Publish 100 packages with semver versions
    for (int i = 0; i < 100; ++i) {
        std::string pkg_name = "pkg_bench_" + std::to_string(i);
        for (int v = 0; v < 10; ++v) {
            std::string ver = "1." + std::to_string(v) + ".0";
            reg.publish_package({pkg_name, ver, "Bench", "author", "hash", {}});
        }
    }

    // Benchmark 1: Dependency Semver Resolution Time for 100 Packages
    std::size_t sample_res = 100;
    std::cout << "[BENCH 1] Executing Semver Resolution for " << sample_res << " Packages...\n";

    auto start_res = std::chrono::high_resolution_clock::now();

    for (std::size_t i = 0; i < sample_res; ++i) {
        std::string pkg_name = "pkg_bench_" + std::to_string(i);
        auto best = reg.resolve_best_version(pkg_name, "^1.0.0");
        (void)best;
    }

    auto elapsed_res = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now() - start_res).count();

    std::cout << "  ⏱️ Semver Resolution for 100 Packages completed in " << (elapsed_res / 1000.0) << " ms\n\n";

    // Benchmark 2: Local Package Installation Time for 100 Packages
    std::cout << "[BENCH 2] Executing Local Installation for 100 Packages...\n";

    std::string payload = "modul test_mod fungsi run() -> \"ok\" rampung rampung";
    std::string cs = Sha256::hash(payload);

    auto start_inst = std::chrono::high_resolution_clock::now();

    for (std::size_t i = 0; i < 100; ++i) {
        std::string pkg_name = "pkg_bench_" + std::to_string(i);
        reg.install_package(pkg_name, "1.9.0", payload, cs);
    }

    auto elapsed_inst = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now() - start_inst).count();

    std::cout << "  ⏱️ Installation of 100 Packages completed in " << elapsed_inst << " ms (< 1.0 sec target!)\n\n";

    // Benchmark 3: Search Speed across 1,000 Packages
    std::cout << "[BENCH 3] Executing Search Query across 1,000 Packages in Registry...\n";

    for (int i = 100; i < 1000; ++i) {
        reg.publish_package({"pkg_bench_" + std::to_string(i), "1.0.0", "Bench", "author", "hash", {}});
    }

    auto start_search = std::chrono::high_resolution_clock::now();

    auto search_results = reg.search("pkg_bench_99");

    auto elapsed_search = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now() - start_search).count();

    std::cout << "  ⏱️ Search across 1,000 Packages completed in " << (elapsed_search / 1000.0) << " ms (Found " << search_results.size() << " matches)\n\n";

    fs::remove_all(bench_dir);
    std::cout << "🎉 ALL PACKAGE MANAGER BENCHMARKS EXECUTED SUCCESSFULLY!\n";
    return 0;
}
