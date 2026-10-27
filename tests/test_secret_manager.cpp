#include "../include/garuda/beam/io/secret_manager.hpp"
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>

using namespace garuda::beam;
namespace fs = std::filesystem;

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

void set_env_var(const char* name, const char* value) {
#if defined(_WIN32)
    _putenv_s(name, value);
#else
    setenv(name, value, 1);
#endif
}

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA SECRET MANAGER SUITE\n";
    std::cout << "=======================================================\n\n";

    // === Test 1: Environment Variable Lookup ===
    run_test_with_timeout("Test 1: Environment Variable Lookup", []() {
        set_env_var("GARUDA_TEST_SECRET", "rahasia123");

        auto& sm = SecretManager::instance();
        std::string val = sm.get_secret("GARUDA_TEST_SECRET");
        GARUDA_CHECK(val == "rahasia123", "get_secret must retrieve 'rahasia123' from environment variable");
        GARUDA_CHECK(sm.has_secret("GARUDA_TEST_SECRET"), "has_secret must return true for set env var");

        set_env_var("GARUDA_TEST_SECRET", "");
        std::cout << "  ✓ Test 1: Environment Variable Lookup PASS\n";
    }, 5);

    // === Test 2: Fallback to Default Value ===
    run_test_with_timeout("Test 2: Fallback to Default Value", []() {
        auto& sm = SecretManager::instance();
        std::string val = sm.get_secret("NON_EXISTENT_KEY_999", "default");
        GARUDA_CHECK(val == "default", "get_secret must return fallback default value for non-existent key");

        std::cout << "  ✓ Test 2: Fallback to Default Value PASS\n";
    }, 5);

    // === Test 3: Secret Redaction ===
    run_test_with_timeout("Test 3: Secret Redaction", []() {
        std::string redacted = SecretManager::redact("rahasia123");
        GARUDA_CHECK(redacted == "****", "redact must return '****' for logging safety");

        std::cout << "  ✓ Test 3: Secret Redaction PASS\n";
    }, 5);

    // === Test 4: Load Secrets from .env File ===
    run_test_with_timeout("Test 4: Load Secrets from .env File", []() {
        fs::path tmp_env = fs::temp_directory_path() / "test_garuda.env";

        std::ofstream ofs(tmp_env, std::ios::trunc);
        ofs << "# Comment line\n";
        ofs << "DB_PASS=supersecret\n";
        ofs << "API_KEY=\"token_abc123\"\n";
        ofs.close();

        auto& sm = SecretManager::instance();
        sm.clear_file_secrets();

        bool loaded = sm.load_from_file(tmp_env.string());
        GARUDA_CHECK(loaded, "load_from_file must succeed");

        std::string db_pass = sm.get_secret("DB_PASS");
        GARUDA_CHECK(db_pass == "supersecret", "get_secret must return 'supersecret' from .env file");

        std::string api_key = sm.get_secret("API_KEY");
        GARUDA_CHECK(api_key == "token_abc123", "get_secret must strip quotes and return 'token_abc123'");

        fs::remove(tmp_env);
        sm.clear_file_secrets();

        std::cout << "  ✓ Test 4: Load Secrets from .env File PASS\n";
    }, 5);

    std::cout << "\n🎉 ALL SECRET MANAGER TESTS PASSED PERFECTLY!\n";
    return 0;
}
