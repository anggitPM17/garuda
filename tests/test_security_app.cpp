#include "../include/garuda/beam/io/rate_limiter.hpp"
#include "../include/garuda/beam/io/validator.hpp"
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

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA RATE LIMITER & INPUT VALIDATION SECURITY SUITE\n";
    std::cout << "=======================================================\n\n";

    // === Test 1: Rate Limiter Throttling ===
    run_test_with_timeout("Test 1: Rate Limiter Throttling", []() {
        RateLimiter limiter;

        // Key 1: IP_1 allows max 3 requests in 60s window
        GARUDA_CHECK(limiter.is_allowed("IP_1", 3, 60), "1st request for IP_1 must be allowed");
        GARUDA_CHECK(limiter.is_allowed("IP_1", 3, 60), "2nd request for IP_1 must be allowed");
        GARUDA_CHECK(limiter.is_allowed("IP_1", 3, 60), "3rd request for IP_1 must be allowed");
        GARUDA_CHECK(!limiter.is_allowed("IP_1", 3, 60), "4th request for IP_1 must be blocked (Too Many Requests)");

        // Key 2: IP_2 remains allowed
        GARUDA_CHECK(limiter.is_allowed("IP_2", 3, 60), "1st request for IP_2 must be allowed independently");

        std::cout << "  ✓ Test 1: Rate Limiter Throttling PASS\n";
    }, 5);

    // === Test 2: Validator Email Format ===
    run_test_with_timeout("Test 2: Validator Email Format", []() {
        GARUDA_CHECK(Validator::is_valid_email("budi@example.com"), "budi@example.com must be valid");
        GARUDA_CHECK(Validator::is_valid_email("user.name+tag@domain.co.id"), "user.name+tag@domain.co.id must be valid");

        GARUDA_CHECK(!Validator::is_valid_email("budi.com"), "budi.com without @ must be invalid");
        GARUDA_CHECK(!Validator::is_valid_email("budi@.com"), "budi@.com with empty domain name must be invalid");
        GARUDA_CHECK(!Validator::is_valid_email("@domain.com"), "@domain.com without username must be invalid");
        GARUDA_CHECK(!Validator::is_valid_email("budi@domain"), "budi@domain without TLD dot must be invalid");

        std::cout << "  ✓ Test 2: Validator Email Format PASS\n";
    }, 5);

    // === Test 3: Validator Safe String Control Characters ===
    run_test_with_timeout("Test 3: Validator Safe String Control Characters", []() {
        GARUDA_CHECK(Validator::is_safe_string("Hello Garuda 123!"), "Normal printable string must be safe");

        std::string newline_str = "Hello\nGaruda";
        std::string cr_str = "Data\r\nHeader: Bad";
        std::string null_str = "Text\0Bad";
        null_str.push_back('\0');

        GARUDA_CHECK(!Validator::is_safe_string(newline_str), "String with newline must be flagged unsafe");
        GARUDA_CHECK(!Validator::is_safe_string(cr_str), "String with CR must be flagged unsafe");
        GARUDA_CHECK(!Validator::is_safe_string(null_str), "String with null byte must be flagged unsafe");

        std::cout << "  ✓ Test 3: Validator Safe String Control Characters PASS\n";
    }, 5);

    // === Test 4: Sanitize HTML XSS Prevention ===
    run_test_with_timeout("Test 4: Sanitize HTML XSS Prevention", []() {
        std::string xss_input = "<script>alert('xss')</script>";
        std::string sanitized = Validator::sanitize_html(xss_input);

        GARUDA_CHECK(sanitized == "&lt;script&gt;alert(&#39;xss&#39;)&lt;/script&gt;", "XSS script tags & quotes must be escaped to HTML entities");
        GARUDA_CHECK(sanitized.find("<script>") == std::string::npos, "Sanitized output must NOT contain unescaped <script>");

        std::cout << "  ✓ Test 4: Sanitize HTML XSS Prevention PASS (Output: " << sanitized << ")\n";
    }, 5);

    std::cout << "\n🎉 ALL RATE LIMITER & INPUT VALIDATION SECURITY TESTS PASSED PERFECTLY!\n";
    return 0;
}
