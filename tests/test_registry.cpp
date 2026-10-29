// =====================================================================
// FASE D.2.2 - GARUDA C REGISTRY TESTS
// Memverifikasi registry.hpp
// =====================================================================
#include "../include/garuda/ffi/registry.hpp"
#include <iostream>
#include <string>
#include <cstdlib>

using namespace garuda::ffi;

#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

int main() {
    std::cout << "========================================\n";
    std::cout << "FASE D.2.2 - C REGISTRY TESTS\n";
    std::cout << "========================================\n\n";

    CRegistry reg;

#if defined(_WIN32)
    CFunctionSpec spec{"kernel32.dll", "GetCurrentProcessId", "int", {}};
#else
    CFunctionSpec spec{"libc.so.6", "getpid", "int", {}};
#endif

    // Test 1: Register C Function
    std::cout << "[Test 1] Register C Function...\n";
    {
        bool registered = reg.register_function("get_pid", spec);
        CHECK(registered, "register_function harus return true");
        CHECK(reg.function_count() == 1, "function_count harus 1");
    }
    std::cout << "PASS\n\n";

    // Test 2: Get Registered Function
    std::cout << "[Test 2] Get Registered Function...\n";
    {
        auto fn_ref = reg.get("get_pid");
        CHECK(fn_ref.has_value(), "get('get_pid') harus return reference valid");
        CHECK(fn_ref->get().is_loaded(), "is_loaded() harus true");
    }
    std::cout << "PASS\n\n";

    // Test 3: Call Int Function
    std::cout << "[Test 3] Call Int Function...\n";
    {
        auto res = reg.call_int("get_pid");
        CHECK(res.has_value(), "call_int harus return value valid");
        CHECK(*res > 0, "PID result harus > 0");
    }
    std::cout << "PASS\n\n";

    // Test 4: Register Invalid Library
    std::cout << "[Test 4] Register Invalid Library...\n";
    {
        CFunctionSpec bad_spec{"invalid_lib_99.dll", "bad_fn", "int", {}};
        bool ok = reg.register_function("bad_fn", bad_spec);
        CHECK(!ok, "Register invalid lib harus return false");
    }
    std::cout << "PASS\n\n";

    // Test 5: List Functions
    std::cout << "[Test 5] List Functions...\n";
    {
        auto list = reg.list_functions();
        CHECK(list.size() == 1, "list_functions size harus 1");
        CHECK(list[0] == "get_pid", "Name harus get_pid");
    }
    std::cout << "PASS\n\n";

    std::cout << "========================================\n";
    std::cout << "FASE D.2.2 - C REGISTRY SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
