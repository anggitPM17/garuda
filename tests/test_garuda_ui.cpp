#include <iostream>
#include <fstream>
#include <cassert>
#include <string>

#include "garuda/beam/interop/embedding_api.hpp"

using namespace garuda::beam::interop;
using namespace garuda::beam;

void test_garuda_ui_framework() {
    std::string gui_file = "packages/gui/src/garuda_ui.grd";

    std::ifstream ifs(gui_file);
    if (!ifs.is_open()) {
        ifs.open("garuda_v67_0_production/garuda_v67.0.0/" + gui_file);
    }
    assert(ifs.is_open() && "Cannot open packages/gui/src/garuda_ui.grd");

    std::string gui_code((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    ifs.close();

    GvmEmbeddingHost host;
    bool load_ok = host.load_source(gui_code);
    assert(load_ok && "Failed to load packages/gui/src/garuda_ui.grd via GvmEmbeddingHost");
    (void)load_ok;

    assert(host.has_function("buat_jendela") && "GarudaUI should have 'buat_jendela' function");
    assert(host.has_function("tambah_tombol") && "GarudaUI should have 'tambah_tombol' function");
    assert(host.has_function("tambah_teks_input") && "GarudaUI should have 'tambah_teks_input' function");
    assert(host.has_function("tambah_kotak_pesan") && "GarudaUI should have 'tambah_kotak_pesan' function");
    assert(host.has_function("hitung_komponen") && "GarudaUI should have 'hitung_komponen' function");

    auto count_res = host.call_function("hitung_komponen");
    assert(count_res.has_value() && "hitung_komponen should return a value");
    assert(std::get<int64_t>(*count_res) == 1 && "GarudaUI component count should be 1");

    std::cout << "[PASS] test_garuda_ui_framework" << std::endl;
}

int main() {
    std::cout << "--- RUNNING GARUDA DESKTOP NATIVE GUI FRAMEWORK TESTS ---" << std::endl;
    test_garuda_ui_framework();
    std::cout << "--- ALL GARUDA DESKTOP NATIVE GUI FRAMEWORK TESTS PASSED 100% ---" << std::endl;
    return 0;
}
