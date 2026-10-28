#include "../include/garuda/beam/tools/script_engine.hpp"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>

using namespace garuda::beam;
namespace fs = std::filesystem;

#define GARUDA_CHECK(cond, msg) \
    if (!(cond)) { \
        std::cerr << "  ❌ ASSERTION FAILED: " << msg << " (" << #cond << ")\n"; \
        std::exit(1); \
    }

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA SCRIPT ENGINE & EMBEDDING SUITE\n";
    std::cout << "=======================================================\n\n";

    ScriptEngine engine;

    // === Test 1: Eval Expression (1 + 2) ===
    {
        std::cout << "Running Test 1: Eval Expression..." << std::endl;
        auto res = engine.eval("1 + 2");
        GARUDA_CHECK(res.has_value(), "eval('1 + 2') must return a value");
        GARUDA_CHECK(res->get_if<std::int64_t>() && *res->get_if<std::int64_t>() == 3, "eval('1 + 2') must equal 3");

        std::cout << "  ✓ Test 1: Eval Expression PASS (1 + 2 = " << *res->get_if<std::int64_t>() << ")\n";
    }

    // === Test 2: Eval Function Call (tambah(3, 4)) ===
    {
        std::cout << "Running Test 2: Eval Function Call..." << std::endl;
        std::string code = R"(
            fungsi tambah(a, b) -> a + b
            rampung
            fungsi utama() -> tambah(3, 4)
            rampung
        )";
        auto res = engine.eval(code);
        if (res.has_value()) {
            if (res->get_if<std::int64_t>()) {
                std::cout << "  Got int: " << *res->get_if<std::int64_t>() << std::endl;
            } else if (res->get_if<double>()) {
                std::cout << "  Got double: " << *res->get_if<double>() << std::endl;
            } else if (res->get_if<Atom>()) {
                std::cout << "  Got atom: " << res->get_if<Atom>()->name << std::endl;
            } else {
                std::cout << "  Got other type" << std::endl;
            }
        } else {
            std::cout << "  Got nullopt!" << std::endl;
        }
        GARUDA_CHECK(res.has_value(), "eval(tambah(3, 4)) must return a value");
        GARUDA_CHECK(res->get_if<std::int64_t>() && *res->get_if<std::int64_t>() == 7, "tambah(3, 4) must equal 7");

        std::cout << "  ✓ Test 2: Eval Function Call PASS (3 + 4 = " << *res->get_if<std::int64_t>() << ")\n";
    }

    // === Test 3: Eval File ===
    {
        std::cout << "Running Test 3: Eval File..." << std::endl;
        fs::path tmp_file = fs::temp_directory_path() / "test_eval.grd";
        {
            std::ofstream ofs(tmp_file, std::ios::trunc);
            ofs << "modul TestFile\nfungsi utama() -> 42\nrampung\n";
            ofs.close();
        }

        auto res = engine.eval_file(tmp_file.string());
        GARUDA_CHECK(res.has_value(), "eval_file must return a value");
        GARUDA_CHECK(res->get_if<std::int64_t>() && *res->get_if<std::int64_t>() == 42, "eval_file result must equal 42");

        fs::remove(tmp_file);

        std::cout << "  ✓ Test 3: Eval File PASS (Result: " << *res->get_if<std::int64_t>() << ")\n";
    }

    // === Test 4: Syntax Error Handling (No Crash) ===
    {
        std::cout << "Running Test 4: Syntax Error Handling..." << std::endl;
        auto res = engine.eval("1 +");
        GARUDA_CHECK(!res.has_value(), "eval('1 +') must return std::nullopt without crashing");

        std::cout << "  ✓ Test 4: Syntax Error Handling PASS (Returned std::nullopt cleanly)\n";
    }

    // === Test 5: Nested Non-Tail Call (frame_stack regression) ===
    {
        std::cout << "Running Test 5: Nested Non-Tail Call..." << std::endl;
        std::string code = R"(
            fungsi helper(x) -> x * 2
            rampung
            fungsi utama() -> helper(5) + 1
            rampung
        )";
        auto res = engine.eval(code);
        GARUDA_CHECK(res.has_value(), "nested call must return a value");
        GARUDA_CHECK(res->get_if<std::int64_t>() && *res->get_if<std::int64_t>() == 11,
                     "helper(5) + 1 must equal 11 (caller frame must survive callee)");
        std::cout << "  ✓ Test 5: Nested Non-Tail Call PASS (" << *res->get_if<std::int64_t>() << ")\n";
    }

    // === Test 6: Sequential Calls Preserve Caller Locals ===
    {
        std::cout << "Running Test 6: Sequential Calls..." << std::endl;
        std::string code = R"(
            fungsi tambah(a, b) -> a + b
            rampung
            fungsi utama() -> tambah(10, 20) + tambah(3, 4)
            rampung
        )";
        auto res = engine.eval(code);
        GARUDA_CHECK(res.has_value(), "sequential calls must return a value");
        GARUDA_CHECK(res->get_if<std::int64_t>() && *res->get_if<std::int64_t>() == 37,
                     "tambah(10,20) + tambah(3,4) must equal 37");
        std::cout << "  ✓ Test 6: Sequential Calls PASS (" << *res->get_if<std::int64_t>() << ")\n";
    }

    std::cout << "\n🎉 ALL SCRIPT ENGINE & EMBEDDING TESTS PASSED PERFECTLY!\n";
    return 0;
}
