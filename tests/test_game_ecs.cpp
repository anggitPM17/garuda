#include <iostream>
#include <fstream>
#include <cassert>
#include <string>

#include "garuda/beam/interop/embedding_api.hpp"

using namespace garuda::beam::interop;
using namespace garuda::beam;

void test_garuda_game_ecs() {
    std::string ecs_file = "packages/game/src/ecs.grd";

    std::ifstream ifs(ecs_file);
    if (!ifs.is_open()) {
        ifs.open("garuda_v67_0_production/garuda_v67.0.0/" + ecs_file);
    }
    assert(ifs.is_open() && "Cannot open packages/game/src/ecs.grd");

    std::string ecs_code((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    ifs.close();

    GvmEmbeddingHost host;
    bool load_ok = host.load_source(ecs_code);
    assert(load_ok && "Failed to load packages/game/src/ecs.grd via GvmEmbeddingHost");
    (void)load_ok;

    assert(host.has_function("buat_dunia") && "GameEcsGaruda should have 'buat_dunia' function");
    assert(host.has_function("sistem_gerak_update") && "GameEcsGaruda should have 'sistem_gerak_update' function");
    assert(host.has_function("hitung_entitas") && "GameEcsGaruda should have 'hitung_entitas' function");

    auto count_res = host.call_function("hitung_entitas");
    assert(count_res.has_value() && "hitung_entitas should return a value");
    assert(std::get<int64_t>(*count_res) == 1 && "GameEcsGaruda active entity count should be 1");

    std::cout << "[PASS] test_garuda_game_ecs" << std::endl;
}

int main() {
    std::cout << "--- RUNNING GARUDA PURE ECS TESTS ---" << std::endl;
    test_garuda_game_ecs();
    std::cout << "--- ALL GARUDA PURE ECS TESTS PASSED 100% ---" << std::endl;
    return 0;
}
