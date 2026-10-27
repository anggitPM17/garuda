#include "../include/garuda/beam/sys/logger.hpp"
#include "../include/garuda/beam/stdlib/json.hpp"
#include <chrono>
#include <future>
#include <iostream>
#include <sstream>

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
    std::cout << "GARUDA STRUCTURED JSON LOGGER SUITE\n";
    std::cout << "=======================================================\n\n";

    // === Test 1: Structured JSON Log Output & Parsing ===
    run_test_with_timeout("Test 1: Structured JSON Log Formatting & Parsing", []() {
        std::stringstream ss_out;
        StructuredLogger::info("Hello Garuda", "test_mod", ss_out);

        std::string json_str = ss_out.str();
        GARUDA_CHECK(!json_str.empty(), "JSON log output string must not be empty");

        // Verify JSON string contents
        GARUDA_CHECK(json_str.find("\"level\":\"INFO\"") != std::string::npos, "Log must contain '\"level\":\"INFO\"'");
        GARUDA_CHECK(json_str.find("\"msg\":\"Hello Garuda\"") != std::string::npos, "Log must contain '\"msg\":\"Hello Garuda\"'");
        GARUDA_CHECK(json_str.find("\"timestamp\":") != std::string::npos, "Log must contain '\"timestamp\":'");

        // Parse JSON string with Json::decode
        auto decoded_opt = Json::decode(json_str);
        GARUDA_CHECK(decoded_opt.has_value(), "Json::decode must successfully parse the JSON log output");

        std::cout << "  ✓ Test 1: Structured JSON Log Formatting & Parsing PASS\n";
    }, 5);

    std::cout << "\n🎉 ALL STRUCTURED JSON LOGGER TESTS PASSED PERFECTLY!\n";
    return 0;
}
