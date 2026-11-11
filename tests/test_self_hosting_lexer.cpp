#include <iostream>
#include <fstream>
#include <cassert>
#include <string>

#include "garuda/beam/interop/embedding_api.hpp"

using namespace garuda::beam::interop;
using namespace garuda::beam;

void test_self_hosting_lexer() {
    std::string lexer_file = "packages/compiler/src/lexer.grd";

    std::ifstream ifs(lexer_file);
    if (!ifs.is_open()) {
        ifs.open("garuda_v67_0_production/garuda_v67.0.0/" + lexer_file);
    }
    assert(ifs.is_open() && "Cannot open packages/compiler/src/lexer.grd");

    std::string lexer_code((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    ifs.close();

    GvmEmbeddingHost host;
    bool load_ok = host.load_source(lexer_code);
    assert(load_ok && "Failed to load packages/compiler/src/lexer.grd via GvmEmbeddingHost");
    (void)load_ok;

    assert(host.has_function("tokenisasi") && "LexerGaruda should have 'tokenisasi' function");
    assert(host.has_function("hitung_token") && "LexerGaruda should have 'hitung_token' function");

    auto count_res = host.call_function("hitung_token");
    assert(count_res.has_value() && "hitung_token should return a value");
    assert(std::get<int64_t>(*count_res) == 7 && "LexerGaruda token count should be 7");

    std::cout << "[PASS] test_self_hosting_lexer" << std::endl;
}

int main() {
    std::cout << "--- RUNNING GARUDA SELF-HOSTING LEXER TESTS ---" << std::endl;
    test_self_hosting_lexer();
    std::cout << "--- ALL GARUDA SELF-HOSTING LEXER TESTS PASSED 100% ---" << std::endl;
    return 0;
}
