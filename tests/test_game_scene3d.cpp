#include <iostream>
#include <fstream>
#include <cassert>
#include <string>

#include "garuda/beam/interop/embedding_api.hpp"

using namespace garuda::beam::interop;
using namespace garuda::beam;

void test_garuda_game_scene3d() {
    std::string scene_file = "packages/game/src/scene3d.grd";

    std::ifstream ifs(scene_file);
    if (!ifs.is_open()) {
        ifs.open("garuda_v67_0_production/garuda_v67.0.0/" + scene_file);
    }
    assert(ifs.is_open() && "Cannot open packages/game/src/scene3d.grd");

    std::string scene_code((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    ifs.close();

    GvmEmbeddingHost host;
    bool load_ok = host.load_source(scene_code);
    assert(load_ok && "Failed to load packages/game/src/scene3d.grd via GvmEmbeddingHost");
    (void)load_ok;

    assert(host.has_function("buat_node_3d") && "GameScene3DGaruda should have 'buat_node_3d' function");
    assert(host.has_function("hitung_transformasi_dunia") && "GameScene3DGaruda should have 'hitung_transformasi_dunia' function");
    assert(host.has_function("buat_material_pbr") && "GameScene3DGaruda should have 'buat_material_pbr' function");
    assert(host.has_function("hitung_total_node") && "GameScene3DGaruda should have 'hitung_total_node' function");

    auto count_res = host.call_function("hitung_total_node");
    assert(count_res.has_value() && "hitung_total_node should return a value");
    assert(std::get<int64_t>(*count_res) == 2 && "GameScene3DGaruda node count should be 2");

    std::cout << "[PASS] test_garuda_game_scene3d" << std::endl;
}

int main() {
    std::cout << "--- RUNNING GARUDA PURE 3D SCENE GRAPH TESTS ---" << std::endl;
    test_garuda_game_scene3d();
    std::cout << "--- ALL GARUDA PURE 3D SCENE GRAPH TESTS PASSED 100% ---" << std::endl;
    return 0;
}
