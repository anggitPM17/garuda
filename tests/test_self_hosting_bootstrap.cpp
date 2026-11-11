#include <iostream>
#include <fstream>
#include <cassert>
#include <string>

#include "garuda/beam/interop/embedding_api.hpp"

using namespace garuda::beam::interop;
using namespace garuda::beam;

void test_self_hosting_bootstrap() {
    std::string grdc_file = "examples/compiler/grdc.grd";

    std::ifstream ifs(grdc_file);
    if (!ifs.is_open()) {
        ifs.open("garuda_v67_0_production/garuda_v67.0.0/" + grdc_file);
    }
    assert(ifs.is_open() && "Cannot open examples/compiler/grdc.grd");

    std::string grdc_code((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    ifs.close();

    // Stage 0 Bootstrapping (C++ Host Compiles Pure Garuda Compiler grdc.grd)
    GvmEmbeddingHost stage1_compiler;
    bool stage1_ok = stage1_compiler.load_source(grdc_code);
    assert(stage1_ok && "Stage 0 C++ Host failed to compile Stage 1 Pure Garuda Compiler grdc.grd");
    (void)stage1_ok;

    assert(stage1_compiler.has_function("utama") && "grdc.grd must have 'utama' function");
    assert(stage1_compiler.has_function("kompilasi_mandiri") && "grdc.grd must have 'kompilasi_mandiri' function");

    // Execute Stage 1 Self-Hosting Compiler
    auto stage1_res = stage1_compiler.call_function("utama");
    assert(stage1_res.has_value() && "Stage 1 Compiler should return a result");
    assert(std::get<int64_t>(*stage1_res) == 200 && "Stage 1 Compiler result parity check passed");

    std::cout << "[PASS] test_self_hosting_bootstrap" << std::endl;
}

int main() {
    std::cout << "--- RUNNING GARUDA SELF-HOSTING BOOTSTRAP TESTS ---" << std::endl;
    test_self_hosting_bootstrap();
    std::cout << "--- ALL GARUDA SELF-HOSTING BOOTSTRAP TESTS PASSED 100% ---" << std::endl;
    return 0;
}
