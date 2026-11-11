#include <iostream>
#include <fstream>
#include <cassert>
#include <string>

#include "garuda/beam/interop/embedding_api.hpp"

using namespace garuda::beam::interop;
using namespace garuda::beam;

void test_garuda_studio_ide() {
    std::string studio_file = "packages/ide/src/garuda_studio.grd";

    std::ifstream ifs(studio_file);
    if (!ifs.is_open()) {
        ifs.open("garuda_v67_0_production/garuda_v67.0.0/" + studio_file);
    }
    assert(ifs.is_open() && "Cannot open packages/ide/src/garuda_studio.grd");

    std::string studio_code((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    ifs.close();

    GvmEmbeddingHost host;
    bool load_ok = host.load_source(studio_code);
    assert(load_ok && "Failed to load packages/ide/src/garuda_studio.grd via GvmEmbeddingHost");
    (void)load_ok;

    assert(host.has_function("inisialisasi_studio") && "GarudaStudioIDE should have 'inisialisasi_studio' function");
    assert(host.has_function("buka_berkas") && "GarudaStudioIDE should have 'buka_berkas' function");
    assert(host.has_function("jalankan_skrip") && "GarudaStudioIDE should have 'jalankan_skrip' function");
    assert(host.has_function("muat_modul_pelatihan") && "GarudaStudioIDE should have 'muat_modul_pelatihan' function");
    assert(host.has_function("hitung_modul_pelatihan") && "GarudaStudioIDE should have 'hitung_modul_pelatihan' function");

    auto count_res = host.call_function("hitung_modul_pelatihan");
    assert(count_res.has_value() && "hitung_modul_pelatihan should return a value");
    assert(std::get<int64_t>(*count_res) == 4 && "GarudaStudioIDE training curriculum module count should be 4");

    std::cout << "[PASS] test_garuda_studio_ide" << std::endl;
}

int main() {
    std::cout << "--- RUNNING GARUDA CODING STUDIO IDE TESTS ---" << std::endl;
    test_garuda_studio_ide();
    std::cout << "--- ALL GARUDA CODING STUDIO IDE TESTS PASSED 100% ---" << std::endl;
    return 0;
}
