// =====================================================================
// FASE D.2.1 - GARUDA C-ABI FFI TESTS
// Memverifikasi ffi.hpp
// =====================================================================
#include "../include/garuda/ffi/ffi.hpp"
#include <iostream>
#include <string>
#include <cstdlib>

using namespace garuda::ffi;

#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

int main() {
    std::cout << "========================================\n";
    std::cout << "FASE D.2.1 - C-ABI FFI TESTS\n";
    std::cout << "========================================\n\n";

#if defined(_WIN32)
    std::string lib_path = "kernel32.dll";
    std::string sym_name = "GetCurrentProcessId";
#else
    std::string lib_path = "libc.so.6";
    std::string sym_name = "getpid";
#endif

    // Test 1: Load C Library & Function Symbol
    std::cout << "[Test 1] Load C Library & Function Symbol...\n";
    {
        auto fn_opt = CFunction::load(lib_path, sym_name);
        CHECK(fn_opt.has_value(), "CFunction::load harus return value untuk " + sym_name);
        CHECK(fn_opt->is_loaded(), "is_loaded() harus true");
        CHECK(fn_opt->name() == sym_name, "Function name harus match");
    }
    std::cout << "PASS\n\n";

    // Test 2: Call C Function
    std::cout << "[Test 2] Call C Function...\n";
    {
        auto fn_opt = CFunction::load(lib_path, sym_name);
        CHECK(fn_opt.has_value(), "Load " + sym_name + " OK");

#if defined(_WIN32)
        using PidType = unsigned long;
        PidType pid = fn_opt->call<PidType>();
#else
        using PidType = int;
        PidType pid = fn_opt->call<PidType>();
#endif
        CHECK(pid > 0, "PID result harus > 0");
        std::cout << "  Process PID obtained via FFI: " << pid << "\n";
    }
    std::cout << "PASS\n\n";

    // Test 3: Load Invalid Library Rejection
    std::cout << "[Test 3] Load Invalid Library Rejection...\n";
    {
        auto fn_opt = CFunction::load("non_existent_library_12345.dll", "some_func");
        CHECK(!fn_opt.has_value(), "Load invalid library harus return nullopt");
    }
    std::cout << "PASS\n\n";

    // Test 4: Load Invalid Symbol Rejection
    std::cout << "[Test 4] Load Invalid Symbol Rejection...\n";
    {
        auto fn_opt = CFunction::load(lib_path, "invalid_symbol_name_99999");
        CHECK(!fn_opt.has_value(), "Load invalid symbol harus return nullopt");
    }
    std::cout << "PASS\n\n";

    // Test 5: Move CFunction Instance
    std::cout << "[Test 5] Move CFunction Instance...\n";
    {
        auto fn_opt = CFunction::load(lib_path, sym_name);
        CFunction fn = std::move(*fn_opt);
        CHECK(fn.is_loaded(), "Moved CFunction instance harus tetap valid & loaded");
    }
    std::cout << "PASS\n\n";

    std::cout << "========================================\n";
    std::cout << "FASE D.2.1 - C-ABI FFI SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
