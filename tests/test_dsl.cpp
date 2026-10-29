// =====================================================================
// FASE D.2.4 - GARUDA FFI DSL TESTS
// Memverifikasi dsl.hpp
// =====================================================================
#include "../include/garuda/ffi/dsl.hpp"
#include <iostream>
#include <string>
#include <cstdlib>

using namespace garuda::ffi;

#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

int main() {
    std::cout << "========================================\n";
    std::cout << "FASE D.2.4 - FFI DSL TESTS\n";
    std::cout << "========================================\n\n";

    auto reg = std::make_shared<CRegistry>();

#if defined(_WIN32)
    CFunctionSpec spec{"kernel32.dll", "GetCurrentProcessId", "int", {}};
#else
    CFunctionSpec spec{"libc.so.6", "getpid", "int", {}};
#endif

    reg->register_function("get_pid", spec);
    Dsl dsl(reg);

    // Test 1: Execute 'panggil_c get_pid'
    std::cout << "[Test 1] Execute 'panggil_c get_pid'...\n";
    {
        auto res = dsl.execute("panggil_c get_pid");
        CHECK(res.has_value(), "execute('panggil_c get_pid') harus return result valid");
        CHECK(std::stoi(*res) > 0, "Result PID string harus > 0");
    }
    std::cout << "PASS\n\n";

    // Test 2: Execute Non-panggil_c Keyword Rejection
    std::cout << "[Test 2] Execute Non-panggil_c Keyword Rejection...\n";
    {
        auto res = dsl.execute("cetak hello");
        CHECK(!res.has_value(), "Keyword bukan panggil_c harus return nullopt");
    }
    std::cout << "PASS\n\n";

    // Test 3: Execute Unknown Alias Rejection
    std::cout << "[Test 3] Execute Unknown Alias Rejection...\n";
    {
        auto res = dsl.execute("panggil_c unknown_fn");
        CHECK(!res.has_value(), "Alias tidak terdaftar harus return nullopt");
    }
    std::cout << "PASS\n\n";

    // Test 4: Execute Script Multiline
    std::cout << "[Test 4] Execute Script Multiline...\n";
    {
        std::string script = "panggil_c get_pid\n# komentar\npanggil_c get_pid\n";
        auto results = dsl.execute_script(script);
        CHECK(results.size() == 2, "execute_script() harus menghasilkan 2 hasil eksekusi");
    }
    std::cout << "PASS\n\n";

    // Test 5: Empty Line Rejection
    std::cout << "[Test 5] Empty Line Rejection...\n";
    {
        auto res = dsl.execute("");
        CHECK(!res.has_value(), "Baris kosong harus return nullopt");
    }
    std::cout << "PASS\n\n";

    std::cout << "========================================\n";
    std::cout << "FASE D.2.4 - FFI DSL SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
