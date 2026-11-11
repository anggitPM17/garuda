#include <iostream>
#include <fstream>
#include <cassert>
#include <string>

#include "garuda/beam/interop/embedding_api.hpp"

using namespace garuda::beam::interop;
using namespace garuda::beam;

void test_garuda_game_audio3d() {
    std::string audio_file = "packages/game/src/audio3d.grd";

    std::ifstream ifs(audio_file);
    if (!ifs.is_open()) {
        ifs.open("garuda_v67_0_production/garuda_v67.0.0/" + audio_file);
    }
    assert(ifs.is_open() && "Cannot open packages/game/src/audio3d.grd");

    std::string audio_code((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    ifs.close();

    GvmEmbeddingHost host;
    bool load_ok = host.load_source(audio_code);
    assert(load_ok && "Failed to load packages/game/src/audio3d.grd via GvmEmbeddingHost");
    (void)load_ok;

    assert(host.has_function("buat_pendengar") && "GameAudio3DGaruda should have 'buat_pendengar' function");
    assert(host.has_function("buat_sumber_suara") && "GameAudio3DGaruda should have 'buat_sumber_suara' function");
    assert(host.has_function("hitung_peredaman_jarak") && "GameAudio3DGaruda should have 'hitung_peredaman_jarak' function");
    assert(host.has_function("hitung_sumber") && "GameAudio3DGaruda should have 'hitung_sumber' function");

    auto count_res = host.call_function("hitung_sumber");
    assert(count_res.has_value() && "hitung_sumber should return a value");
    assert(std::get<int64_t>(*count_res) == 1 && "GameAudio3DGaruda audio source count should be 1");

    std::cout << "[PASS] test_garuda_game_audio3d" << std::endl;
}

int main() {
    std::cout << "--- RUNNING GARUDA PURE 3D SPATIAL AUDIO TESTS ---" << std::endl;
    test_garuda_game_audio3d();
    std::cout << "--- ALL GARUDA PURE 3D SPATIAL AUDIO TESTS PASSED 100% ---" << std::endl;
    return 0;
}
