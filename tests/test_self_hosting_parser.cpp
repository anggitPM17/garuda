#include <iostream>
#include <fstream>
#include <cassert>
#include <string>

#include "garuda/beam/interop/embedding_api.hpp"

using namespace garuda::beam::interop;
using namespace garuda::beam;

void test_self_hosting_parser() {
    std::string parser_file = "packages/compiler/src/parser.grd";

    std::ifstream ifs(parser_file);
    if (!ifs.is_open()) {
        ifs.open("garuda_v67_0_production/garuda_v67.0.0/" + parser_file);
    }
    assert(ifs.is_open() && "Cannot open packages/compiler/src/parser.grd");

    std::string parser_code((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    ifs.close();

    GvmEmbeddingHost host;
    bool load_ok = host.load_source(parser_code);
    assert(load_ok && "Failed to load packages/compiler/src/parser.grd via GvmEmbeddingHost");
    (void)load_ok;

    assert(host.has_function("urai") && "ParserGaruda should have 'urai' function");
    assert(host.has_function("hitung_node") && "ParserGaruda should have 'hitung_node' function");

    auto node_res = host.call_function("hitung_node");
    assert(node_res.has_value() && "hitung_node should return a value");
    assert(std::get<int64_t>(*node_res) == 7 && "ParserGaruda AST node count should be 7");

    std::cout << "[PASS] test_self_hosting_parser" << std::endl;
}

int main() {
    std::cout << "--- RUNNING GARUDA SELF-HOSTING PARSER TESTS ---" << std::endl;
    test_self_hosting_parser();
    std::cout << "--- ALL GARUDA SELF-HOSTING PARSER TESTS PASSED 100% ---" << std::endl;
    return 0;
}
