#include <iostream>
#include <fstream>
#include <cassert>
#include <string>

#include "garuda/beam/interop/embedding_api.hpp"

using namespace garuda::beam::interop;
using namespace garuda::beam;

void test_garuda_wasm_runner() {
    std::string wasm_file = "packages/web/src/wasm_runner.grd";

    std::ifstream ifs(wasm_file);
    if (!ifs.is_open()) {
        ifs.open("garuda_v67_0_production/garuda_v67.0.0/" + wasm_file);
    }
    assert(ifs.is_open() && "Cannot open packages/web/src/wasm_runner.grd");

    std::string wasm_code((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    ifs.close();

    GvmEmbeddingHost host;
    bool load_ok = host.load_source(wasm_code);
    assert(load_ok && "Failed to load packages/web/src/wasm_runner.grd via GvmEmbeddingHost");
    (void)load_ok;

    assert(host.has_function("buat_modul_wasm") && "WasmRunnerGaruda should have 'buat_modul_wasm' function");
    assert(host.has_function("alokasi_memori_linier") && "WasmRunnerGaruda should have 'alokasi_memori_linier' function");
    assert(host.has_function("eksekusi_fungsi_wasm") && "WasmRunnerGaruda should have 'eksekusi_fungsi_wasm' function");
    assert(host.has_function("hitung_halaman_memori") && "WasmRunnerGaruda should have 'hitung_halaman_memori' function");

    auto page_res = host.call_function("hitung_halaman_memori");
    assert(page_res.has_value() && "hitung_halaman_memori should return a value");
    assert(std::get<int64_t>(*page_res) == 4 && "WasmRunnerGaruda memory page count should be 4");

    std::cout << "[PASS] test_garuda_wasm_runner" << std::endl;
}

int main() {
    std::cout << "--- RUNNING GARUDA PURE WASM RUNNER TESTS ---" << std::endl;
    test_garuda_wasm_runner();
    std::cout << "--- ALL GARUDA PURE WASM RUNNER TESTS PASSED 100% ---" << std::endl;
    return 0;
}
