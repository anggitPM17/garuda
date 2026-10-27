#include "../include/garuda/beam/stdlib/jwt.hpp"
#include <chrono>
#include <future>
#include <iostream>

using namespace garuda::beam;
using namespace garuda::beam::stdlib;

#define GARUDA_CHECK(cond, msg) \
    if (!(cond)) { \
        std::cerr << "  ❌ ASSERTION FAILED: " << msg << " (" << #cond << ")\n"; \
        std::exit(1); \
    }

void run_test_with_timeout(const std::string& test_name, std::function<void()> test_fn, int timeout_seconds = 5) {
    auto future = std::async(std::launch::async, test_fn);
    if (future.wait_for(std::chrono::seconds(timeout_seconds)) == std::future_status::timeout) {
        std::cerr << "  ❌ " << test_name << " FAILED (TIMEOUT > " << timeout_seconds << "s)\n";
        std::exit(1);
    }
}

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA JWT HS256 AUTHENTICATION SUITE\n";
    std::cout << "=======================================================\n\n";

    // === Test 1: JWT Encode & Decode Roundtrip ===
    run_test_with_timeout("Test 1: JWT Encode & Decode Roundtrip", []() {
        std::map<std::string, Term> payload;
        payload["user"] = Term("budi");
        payload["role"] = Term("admin");

        std::string secret = "rahasia_garuda";
        std::string token = Jwt::encode(payload, secret);

        GARUDA_CHECK(!token.empty(), "Generated JWT token must not be empty");
        GARUDA_CHECK(std::count(token.begin(), token.end(), '.') == 2, "JWT token must contain exactly 2 periods '.'");

        // Decode with correct secret
        auto decoded_opt = Jwt::decode(token, secret);
        GARUDA_CHECK(decoded_opt.has_value(), "Jwt::decode with correct secret must succeed");

        auto decoded_map = *decoded_opt;
        auto u_it = decoded_map.find("user");
        auto r_it = decoded_map.find("role");

        GARUDA_CHECK(u_it != decoded_map.end() && to_string(u_it->second) == "\"budi\"", "User claim must be 'budi'");
        GARUDA_CHECK(r_it != decoded_map.end() && to_string(r_it->second) == "\"admin\"", "Role claim must be 'admin'");

        std::cout << "  ✓ Test 1: JWT Encode & Decode Roundtrip PASS (Token: " << token.substr(0, 25) << "...)\n";
    }, 5);

    // === Test 2: Invalid Secret Token Rejection ===
    run_test_with_timeout("Test 2: Invalid Secret Token Rejection", []() {
        std::map<std::string, Term> payload;
        payload["user"] = Term("budi");
        payload["role"] = Term("admin");

        std::string secret = "rahasia_garuda";
        std::string token = Jwt::encode(payload, secret);

        // Decode with wrong secret
        auto bad_decode = Jwt::decode(token, "salah_secret");
        GARUDA_CHECK(!bad_decode.has_value(), "Jwt::decode with invalid secret must return nullopt");

        std::cout << "  ✓ Test 2: Invalid Secret Token Rejection PASS\n";
    }, 5);

    std::cout << "\n🎉 ALL JWT HS256 AUTHENTICATION TESTS PASSED PERFECTLY!\n";
    return 0;
}
