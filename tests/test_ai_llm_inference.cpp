#include <iostream>
#include <fstream>
#include <cassert>
#include <string>

#include "garuda/beam/interop/embedding_api.hpp"

using namespace garuda::beam::interop;
using namespace garuda::beam;

void test_garuda_ai_llm_inference() {
    std::string ai_file = "packages/ai/src/llm_inference.grd";

    std::ifstream ifs(ai_file);
    if (!ifs.is_open()) {
        ifs.open("garuda_v67_0_production/garuda_v67.0.0/" + ai_file);
    }
    assert(ifs.is_open() && "Cannot open packages/ai/src/llm_inference.grd");

    std::string ai_code((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    ifs.close();

    GvmEmbeddingHost host;
    bool load_ok = host.load_source(ai_code);
    assert(load_ok && "Failed to load packages/ai/src/llm_inference.grd via GvmEmbeddingHost");
    (void)load_ok;

    assert(host.has_function("muat_model_lokal") && "LlmInferenceGaruda should have 'muat_model_lokal' function");
    assert(host.has_function("tokenisasi_prompt") && "LlmInferenceGaruda should have 'tokenisasi_prompt' function");
    assert(host.has_function("generasi_teks") && "LlmInferenceGaruda should have 'generasi_teks' function");
    assert(host.has_function("hitung_ukuran_model") && "LlmInferenceGaruda should have 'hitung_ukuran_model' function");

    auto size_res = host.call_function("hitung_ukuran_model");
    assert(size_res.has_value() && "hitung_ukuran_model should return a value");
    assert(std::get<int64_t>(*size_res) == 256 && "LlmInferenceGaruda model memory size should be 256 MB");

    std::cout << "[PASS] test_garuda_ai_llm_inference" << std::endl;
}

int main() {
    std::cout << "--- RUNNING GARUDA PURE ON-DEVICE AI LLM INFERENCE TESTS ---" << std::endl;
    test_garuda_ai_llm_inference();
    std::cout << "--- ALL GARUDA PURE ON-DEVICE AI LLM INFERENCE TESTS PASSED 100% ---" << std::endl;
    return 0;
}
