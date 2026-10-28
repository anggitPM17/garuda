#include "../include/garuda/beam/tools/script_engine.hpp"
#include <iostream>

using namespace garuda::beam;

int main() {
    std::cout << "=======================================================\n";
    std::cout << "🚀 GARUDA GVM EMBEDDING DEMO\n";
    std::cout << "=======================================================\n\n";

    ScriptEngine engine;

    // 1. Evaluate Arithmetic Expression
    std::string code1 = "10 + 20 * 2";
    auto res1 = engine.eval(code1);
    if (res1.has_value() && res1->get_if<std::int64_t>()) {
        std::cout << "  ✓ eval('" << code1 << "') = " << *res1->get_if<std::int64_t>() << std::endl;
    }

    // 2. Evaluate Garuda Function
    std::string code2 = R"(
        fungsi tambah(a, b) -> a + b
        fungsi utama() -> tambah(15, 25)
    )";
    auto res2 = engine.eval(code2);
    if (res2.has_value() && res2->get_if<std::int64_t>()) {
        std::cout << "  ✓ eval(function call) = " << *res2->get_if<std::int64_t>() << std::endl;
    }

    std::cout << "\n🎉 GVM Embedded successfully into C++ application!" << std::endl;
    return 0;
}
