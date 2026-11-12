#include <iostream>
#include <fstream>
#include <cassert>
#include <string>

#include "garuda/beam/interop/embedding_api.hpp"

using namespace garuda::beam::interop;
using namespace garuda::beam;

void test_garuda_game_physics3d() {
    std::string physics_file = "packages/game/src/physics3d.grd";

    std::ifstream ifs(physics_file);
    if (!ifs.is_open()) {
        ifs.open("garuda_v67_0_production/garuda_v67.0.0/" + physics_file);
    }
    assert(ifs.is_open() && "Cannot open packages/game/src/physics3d.grd");

    std::string physics_code((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    ifs.close();

    GvmEmbeddingHost host;
    bool load_ok = host.load_source(physics_code);
    assert(load_ok && "Failed to load packages/game/src/physics3d.grd via GvmEmbeddingHost");
    (void)load_ok;

    assert(host.has_function("buat_dunia_fisika") && "Physics3DGaruda should have 'buat_dunia_fisika' function");
    assert(host.has_function("deteksi_tabrakan_aabb") && "Physics3DGaruda should have 'deteksi_tabrakan_aabb' function");
    assert(host.has_function("hitung_benda_kaku") && "Physics3DGaruda should have 'hitung_benda_kaku' function");

    auto count_res = host.call_function("hitung_benda_kaku");
    assert(count_res.has_value() && "hitung_benda_kaku should return a value");
    assert(std::get<int64_t>(*count_res) == 0 && "Physics3DGaruda initial rigid body count should be 0");

    std::cout << "[PASS] test_garuda_game_physics3d" << std::endl;
}

int main() {
    std::cout << "--- RUNNING GARUDA PURE 3D PHYSICS TESTS ---" << std::endl;
    test_garuda_game_physics3d();
    std::cout << "--- ALL GARUDA PURE 3D PHYSICS TESTS PASSED 100% ---" << std::endl;
    return 0;
}
