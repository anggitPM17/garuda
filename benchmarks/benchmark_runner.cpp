// Garuda Language Compiler: Performance Benchmark Suite (`benchmark_runner.cpp`)
#include "garuda/beam/tools/script_engine.hpp"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

struct BenchResult {
    std::string name;
    double median_ms;
    double min_ms;
    double max_ms;
};

double run_once(garuda::beam::ScriptEngine& engine, const std::string& code) {
    auto start = std::chrono::high_resolution_clock::now();
    engine.eval(code);
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count();
}

BenchResult bench(const std::string& name, const std::string& code, int iterations = 3) {
    garuda::beam::ScriptEngine engine;
    std::vector<double> times;
    for (int i = 0; i < iterations; ++i) {
        times.push_back(run_once(engine, code));
    }
    std::sort(times.begin(), times.end());
    double median = times[times.size() / 2];
    return {name, median, times.front(), times.back()};
}

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA BENCHMARK SUITE (v67.0.0)\n";
    std::cout << "=======================================================\n\n";

    std::vector<BenchResult> results;

    // Benchmark 1: Startup & Hello World
    results.push_back(bench("Startup (hello world)", R"(
        modul BenchStartup
        fungsi utama() -> "Halo" rampung
    )"));

    // Benchmark 2: List.peta Transformation
    results.push_back(bench("List.peta Transformation", R"(
        modul BenchPeta
        impor "std/list.grd"
        fungsi utama() ->
            List.peta([1, 2, 3, 4, 5, 6, 7, 8, 9, 10], fungsi(X) -> X * 2 rampung)
        rampung
    )"));

    // Benchmark 3: String Operations (17 functions)
    results.push_back(bench("String.ganti & String.balik", R"(
        modul BenchString
        impor "std/string.grd"
        fungsi utama() ->
            String.balik(String.ganti("halo dunia garuda", "dunia", "BEAM"))
        rampung
    )"));

    // Benchmark 4: Map Insertion & Lookup
    results.push_back(bench("Map.taruh & Map.ambil", R"(
        modul BenchMap
        impor "std/map.grd"
        fungsi utama() ->
            M = Map.taruh(Map.taruh(Map.taruh(Map.baru(), "a", 10), "b", 20), "c", 30)
            Map.ambil(M, "c")
        rampung
    )"));

    // Benchmark 5: JSON Encoding & Decoding
    results.push_back(bench("JSON Encode & Decode", R"(
        modul BenchJson
        impor "std/json.grd"
        fungsi utama() ->
            Encoded = Json.encode_list([1, 2, 3, 4, 5])
            Json.decode_list(Encoded)
        rampung
    )"));

    // Benchmark 6: Actor Concurrency Spawn & Send
    results.push_back(bench("Actor Spawn & Send Message", R"(
        modul BenchActor
        fungsi utama() ->
            Pid = cakra fungsi() -> tampa _ -> nil rampung rampung
            kirim(Pid, "pesan")
            :ok
        rampung
    )"));

    // Print Results Table
    std::cout << "| Benchmark | Median (ms) | Min (ms) | Max (ms) |\n";
    std::cout << "| :--- | :---: | :---: | :---: |\n";
    for (const auto& r : results) {
        std::cout << "| " << std::left << std::setw(30) << r.name
                  << " | " << std::fixed << std::setprecision(3) << std::setw(10) << r.median_ms
                  << " | " << std::fixed << std::setprecision(3) << std::setw(8) << r.min_ms
                  << " | " << std::fixed << std::setprecision(3) << std::setw(8) << r.max_ms << " |\n";
    }

    return 0;
}
