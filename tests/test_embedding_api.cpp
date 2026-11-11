#include <iostream>
#include <cassert>
#include <string>

#include "garuda/beam/interop/embedding_api.hpp"

using namespace garuda::beam::interop;
using namespace garuda::beam;

void test_load_source_and_call() {
    GvmEmbeddingHost host;

    std::string grd_source = R"(
        modul EmbedDemo
        fungsi utama() ->
            100
        rampung
    )";

    bool load_ok = host.load_source(grd_source);
    assert(load_ok && "GvmEmbeddingHost::load_source failed");
    (void)load_ok;

    assert(host.has_function("utama") && "GvmEmbeddingHost::has_function('utama') should be true");
    assert(!host.has_function("non_existent") && "has_function should return false for unknown function");

    auto result = host.call_function("utama");
    assert(result.has_value() && "call_function('utama') should return a value");
    assert(std::get<int64_t>(*result) == 100 && "utama() should return 100");

    std::cout << "[PASS] test_load_source_and_call" << std::endl;
}

void test_set_get_global() {
    GvmEmbeddingHost host;

    host.set_global("version", std::string("67.0.0"));
    host.set_global("max_connections", int64_t(10000));

    auto ver = host.get_global("version");
    assert(ver.has_value() && std::get<std::string>(*ver) == "67.0.0");

    auto max_conn = host.get_global("max_connections");
    assert(max_conn.has_value() && std::get<int64_t>(*max_conn) == 10000);

    auto unknown = host.get_global("unknown_key");
    assert(!unknown.has_value() && "get_global should return nullopt for unknown key");

    std::cout << "[PASS] test_set_get_global" << std::endl;
}

int main() {
    std::cout << "--- RUNNING GVM INTEROP EMBEDDING API TESTS ---" << std::endl;
    test_load_source_and_call();
    test_set_get_global();
    std::cout << "--- ALL GVM INTEROP EMBEDDING API TESTS PASSED 100% ---" << std::endl;
    return 0;
}
