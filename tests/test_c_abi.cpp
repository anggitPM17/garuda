#include "../include/garuda/beam/ffi/c_abi.hpp"
#include <chrono>
#include <future>
#include <iostream>

using namespace garuda::beam;

#define GARUDA_CHECK(cond, msg) \
    if (!(cond)) { \
        std::cerr << "  ❌ ASSERTION FAILED: " << msg << " (" << #cond << ")\n"; \
        std::exit(1); \
    }

void run_test_with_timeout(const std::string& test_name, std::function<void()> test_fn, int timeout_seconds = 5) {
    auto future = std::async(std::launch::async, test_fn);
    if (future.wait_for(std::chrono::seconds(timeout_seconds)) == std::future_status::timeout) {
        std::cerr << "  ❌ " << test_name << " FAILED (TIMEOUT > " << timeout_seconds << "s)\n";
        std::exit(1); \
    }
}

// C-compatible functions
extern "C" GrdValue c_add(const GrdValue* args, int argc) {
    if (argc < 2) return GrdValue::make_int(0);
    return GrdValue::make_int(args[0].int_val + args[1].int_val);
}

static std::string g_greet_storage;
extern "C" GrdValue c_greet(const GrdValue* args, int argc) {
    if (argc < 1 || !args[0].str_val) return GrdValue::make_string("Hello!");
    g_greet_storage = std::string("Hello, ") + args[0].str_val;
    return GrdValue::make_string(g_greet_storage.c_str());
}

extern "C" GrdValue c_is_even(const GrdValue* args, int argc) {
    if (argc < 1) return GrdValue::make_bool(0);
    return GrdValue::make_bool(args[0].int_val % 2 == 0 ? 1 : 0);
}

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA FFI C ABI & FUNCTION REGISTRY SUITE\n";
    std::cout << "=======================================================\n\n";

    auto& ffi = CFfiRegistry::instance();
    ffi.clear();
    ffi.register_func("add", c_add);
    ffi.register_func("greet", c_greet);
    ffi.register_func("is_even", c_is_even);

    // === Test 1: Integer C Function Call (add) ===
    run_test_with_timeout("Test 1: Integer C Function Call (add)", [&]() {
        auto res = ffi.call_func("add", {Term(10LL), Term(20LL)});
        GARUDA_CHECK(res.has_value(), "call_func('add') must return a value");
        GARUDA_CHECK(res->get_if<std::int64_t>() && *res->get_if<std::int64_t>() == 30, "add(10, 20) must equal 30");

        std::cout << "  ✓ Test 1: Integer C Function Call (add) PASS (10 + 20 = " << *res->get_if<std::int64_t>() << ")\n";
    }, 5);

    // === Test 2: String C Function Call (greet) ===
    run_test_with_timeout("Test 2: String C Function Call (greet)", [&]() {
        auto res = ffi.call_func("greet", {Term("Garuda")});
        GARUDA_CHECK(res.has_value(), "call_func('greet') must return a value");
        GARUDA_CHECK(res->get_if<std::string>() && *res->get_if<std::string>() == "Hello, Garuda", "greet('Garuda') must equal 'Hello, Garuda'");

        std::cout << "  ✓ Test 2: String C Function Call (greet) PASS (Greeting: " << *res->get_if<std::string>() << ")\n";
    }, 5);

    // === Test 3: Boolean C Function Call (is_even) ===
    run_test_with_timeout("Test 3: Boolean C Function Call (is_even)", [&]() {
        auto res_even = ffi.call_func("is_even", {Term(42LL)});
        GARUDA_CHECK(res_even.has_value() && res_even->get_if<Atom>() && res_even->get_if<Atom>()->name == "true", "is_even(42) must return true");

        auto res_odd = ffi.call_func("is_even", {Term(43LL)});
        GARUDA_CHECK(res_odd.has_value() && res_odd->get_if<Atom>() && res_odd->get_if<Atom>()->name == "false", "is_odd(43) must return false");

        std::cout << "  ✓ Test 3: Boolean C Function Call (is_even) PASS\n";
    }, 5);

    // === Test 4: Direct Type Conversion Tests ===
    run_test_with_timeout("Test 4: Direct Type Conversions", []() {
        // Int
        Term t_int(100LL);
        GrdValue g_int = term_to_grd_value(t_int);
        Term t_int_back = grd_value_to_term(g_int);
        GARUDA_CHECK(*t_int_back.get_if<std::int64_t>() == 100, "Int conversion roundtrip must match");

        // String
        Term t_str("Garuda FFI");
        GrdValue g_str = term_to_grd_value(t_str);
        Term t_str_back = grd_value_to_term(g_str);
        GARUDA_CHECK(*t_str_back.get_if<std::string>() == "Garuda FFI", "String conversion roundtrip must match");

        // Bool
        Term t_bool(Atom{"true"});
        GrdValue g_bool = term_to_grd_value(t_bool);
        Term t_bool_back = grd_value_to_term(g_bool);
        GARUDA_CHECK(t_bool_back.get_if<Atom>()->name == "true", "Bool conversion roundtrip must match");

        std::cout << "  ✓ Test 4: Direct Type Conversions PASS\n";
    }, 5);

    ffi.clear();

    std::cout << "\n🎉 ALL FFI C ABI & FUNCTION REGISTRY TESTS PASSED PERFECTLY!\n";
    return 0;
}
