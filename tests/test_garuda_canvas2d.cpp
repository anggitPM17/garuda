#include <iostream>
#include <fstream>
#include <cassert>
#include <string>

#include "garuda/beam/interop/embedding_api.hpp"

using namespace garuda::beam::interop;
using namespace garuda::beam;

void test_garuda_canvas2d() {
    std::string canvas_file = "packages/gui/src/canvas2d.grd";

    std::ifstream ifs(canvas_file);
    if (!ifs.is_open()) {
        ifs.open("garuda_v67_0_production/garuda_v67.0.0/" + canvas_file);
    }
    assert(ifs.is_open() && "Cannot open packages/gui/src/canvas2d.grd");

    std::string canvas_code((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    ifs.close();

    GvmEmbeddingHost host;
    bool load_ok = host.load_source(canvas_code);
    assert(load_ok && "Failed to load packages/gui/src/canvas2d.grd via GvmEmbeddingHost");
    (void)load_ok;

    assert(host.has_function("buat_kanvas") && "GarudaCanvas2D should have 'buat_kanvas' function");
    assert(host.has_function("gambar_garis") && "GarudaCanvas2D should have 'gambar_garis' function");
    assert(host.has_function("gambar_lingkaran") && "GarudaCanvas2D should have 'gambar_lingkaran' function");
    assert(host.has_function("gambar_persegi") && "GarudaCanvas2D should have 'gambar_persegi' function");
    assert(host.has_function("hitung_elemen_gambar") && "GarudaCanvas2D should have 'hitung_elemen_gambar' function");

    auto count_res = host.call_function("hitung_elemen_gambar");
    assert(count_res.has_value() && "hitung_elemen_gambar should return a value");
    assert(std::get<int64_t>(*count_res) == 1 && "GarudaCanvas2D vector element count should be 1");

    std::cout << "[PASS] test_garuda_canvas2d" << std::endl;
}

int main() {
    std::cout << "--- RUNNING GARUDA PURE CANVAS 2D VECTOR DRAWING TESTS ---" << std::endl;
    test_garuda_canvas2d();
    std::cout << "--- ALL GARUDA PURE CANVAS 2D VECTOR DRAWING TESTS PASSED 100% ---" << std::endl;
    return 0;
}
