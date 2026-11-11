#include <iostream>
#include <fstream>
#include <cassert>
#include <string>

#include "garuda/beam/interop/embedding_api.hpp"

using namespace garuda::beam::interop;
using namespace garuda::beam;

void test_self_hosting_codegen() {
    std::string codegen_file = "packages/compiler/src/codegen.grd";

    std::ifstream ifs(codegen_file);
    if (!ifs.is_open()) {
        ifs.open("garuda_v67_0_production/garuda_v67.0.0/" + codegen_file);
    }
    assert(ifs.is_open() && "Cannot open packages/compiler/src/codegen.grd");

    std::string codegen_code((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    ifs.close();

    GvmEmbeddingHost host;
    bool load_ok = host.load_source(codegen_code);
    assert(load_ok && "Failed to load packages/compiler/src/codegen.grd via GvmEmbeddingHost");
    (void)load_ok;

    assert(host.has_function("buat_bytecode") && "CodeGenGaruda should have 'buat_bytecode' function");
    assert(host.has_function("hitung_instruksi") && "CodeGenGaruda should have 'hitung_instruksi' function");

    auto op_res = host.call_function("hitung_instruksi");
    assert(op_res.has_value() && "hitung_instruksi should return a value");
    assert(std::get<int64_t>(*op_res) == 2 && "CodeGenGaruda instruction count should be 2");

    std::cout << "[PASS] test_self_hosting_codegen" << std::endl;
}

int main() {
    std::cout << "--- RUNNING GARUDA SELF-HOSTING CODEGEN TESTS ---" << std::endl;
    test_self_hosting_codegen();
    std::cout << "--- ALL GARUDA SELF-HOSTING CODEGEN TESTS PASSED 100% ---" << std::endl;
    return 0;
}
