// =====================================================================
// FASE C.5.1 - GARUDA RATE LIMITER TESTS
// Memverifikasi rate_limiter.hpp
// =====================================================================
#include "../include/garuda/security/rate_limiter.hpp"
#include <iostream>
#include <string>
#include <cstdlib>
#include <thread>

using namespace garuda::security;

#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

int main() {
    std::cout << "========================================\n";
    std::cout << "FASE C.5.1 - RATE LIMITER TESTS\n";
    std::cout << "========================================\n\n";

    RateLimitConfig cfg{5, std::chrono::seconds(1)};
    RateLimiter limiter(cfg);

    // Test 1: Allow Under Limit
    std::cout << "[Test 1] Allow Under Limit...\n";
    {
        for (int i = 0; i < 5; ++i) {
            CHECK(limiter.allow("192.168.1.1"), "Request ke-" + std::to_string(i+1) + " harus di-allow");
        }
        CHECK(limiter.request_count("192.168.1.1") == 5, "request_count harus 5");
    }
    std::cout << "PASS\n\n";

    // Test 2: Block Over Limit
    std::cout << "[Test 2] Block Over Limit...\n";
    {
        bool allowed_6th = limiter.allow("192.168.1.1");
        CHECK(!allowed_6th, "Request ke-6 harus di-block (return false)");
        CHECK(limiter.request_count("192.168.1.1") == 5, "request_count tetap 5");
    }
    std::cout << "PASS\n\n";

    // Test 3: Reset Key
    std::cout << "[Test 3] Reset Key...\n";
    {
        limiter.reset("192.168.1.1");
        CHECK(limiter.request_count("192.168.1.1") == 0, "request_count = 0 setelah reset");

        bool allowed_after_reset = limiter.allow("192.168.1.1");
        CHECK(allowed_after_reset, "Request setelah reset harus di-allow");
    }
    std::cout << "PASS\n\n";

    // Test 4: Multiple IP Isolation
    std::cout << "[Test 4] Multiple IP Isolation...\n";
    {
        for (int i = 0; i < 5; ++i) {
            limiter.allow("10.0.0.1");
        }
        CHECK(!limiter.allow("10.0.0.1"), "IP 10.0.0.1 reached limit");

        // Different IP 10.0.0.2 should still be allowed
        CHECK(limiter.allow("10.0.0.2"), "IP 10.0.0.2 harus tetap di-allow");
        CHECK(limiter.bucket_count() >= 2, "bucket_count harus >= 2");
    }
    std::cout << "PASS\n\n";

    // Test 5: Window Expiry
    std::cout << "[Test 5] Window Expiry...\n";
    {
        RateLimitConfig short_cfg{2, std::chrono::seconds(1)};
        RateLimiter short_limiter(short_cfg);

        short_limiter.allow("127.0.0.1");
        short_limiter.allow("127.0.0.1");
        CHECK(!short_limiter.allow("127.0.0.1"), "Blocked at limit 2");

        std::this_thread::sleep_for(std::chrono::milliseconds(1100)); // Sleep > 1s window

        CHECK(short_limiter.allow("127.0.0.1"), "Allowed again after window expired");
    }
    std::cout << "PASS\n\n";

    std::cout << "========================================\n";
    std::cout << "FASE C.5.1 - RATE LIMITER SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
