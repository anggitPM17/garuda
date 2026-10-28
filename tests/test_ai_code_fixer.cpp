#include "../include/garuda/autopilot/mock_llm_client.hpp"
#include "../include/garuda/autopilot/ai_code_fixer.hpp"
#include <iostream>
#include <cstdlib>

using namespace garuda::autopilot;

#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

int main() {
    std::cout << "========================================\n";
    std::cout << "FASE 5.2 - AI CODE FIXER TESTS\n";
    std::cout << "========================================\n\n";

    // Test 1: Successful patch generation
    std::cout << "[Test 1] SUCCESS behavior...\n";
    {
        MockLLMClient mock(MockLLMClient::Behavior::SUCCESS, "fun fixed() { return 100 }");
        AICodeFixer fixer(mock);
        auto res = fixer.fix("NullPointerException at MainActivity.kt:42", "class MainActivity {}", "MainActivity.kt");

        CHECK(res.success, "Fix harus sukses");
        CHECK(res.patch == "fun fixed() { return 100 }", "Patch harus sesuai mock");
        CHECK(res.file == "MainActivity.kt", "File harus sesuai target");
        CHECK(res.llm_model == "MockLLM", "Model harus MockLLM");
        CHECK(mock.call_count() == 1, "Mock harus dipanggil 1 kali");
    }
    std::cout << "PASS\n\n";

    // Test 2: Empty patch rejection
    std::cout << "[Test 2] EMPTY_PATCH behavior...\n";
    {
        MockLLMClient mock(MockLLMClient::Behavior::EMPTY_PATCH);
        AICodeFixer fixer(mock);
        auto res = fixer.fix("Error log", "Context");

        CHECK(!res.success, "Fix harus gagal jika patch kosong");
        CHECK(res.error == "LLM returned empty patch", "Error message harus sesuai");
    }
    std::cout << "PASS\n\n";

    // Test 3: LLM failure response
    std::cout << "[Test 3] FAILURE behavior...\n";
    {
        MockLLMClient mock(MockLLMClient::Behavior::FAILURE, "", "API error 500");
        AICodeFixer fixer(mock);
        auto res = fixer.fix("Error log", "Context");

        CHECK(!res.success, "Fix harus gagal jika LLM failure");
        CHECK(res.error == "API error 500", "Error message harus dari LLM");
    }
    std::cout << "PASS\n\n";

    // Test 4: Exception handling (THROW behavior)
    std::cout << "[Test 4] THROW behavior...\n";
    {
        MockLLMClient mock(MockLLMClient::Behavior::THROW);
        AICodeFixer fixer(mock);
        auto res = fixer.fix("Error log", "Context");

        CHECK(!res.success, "Fix harus menangkap exception dan return success=false");
        CHECK(res.error.find("LLM threw exception") != std::string::npos, "Error message harus mencatat exception");
    }
    std::cout << "PASS\n\n";

    // Test 5: Empty error log validation
    std::cout << "[Test 5] Empty error_log input...\n";
    {
        MockLLMClient mock(MockLLMClient::Behavior::SUCCESS);
        AICodeFixer fixer(mock);
        auto res = fixer.fix("", "Context");

        CHECK(!res.success, "Error log kosong harus ditolak sebelum memanggil LLM");
        CHECK(res.error == "empty error_log", "Error message harus 'empty error_log'");
        CHECK(mock.call_count() == 0, "LLM tidak boleh dipanggil jika error_log kosong");
    }
    std::cout << "PASS\n\n";

    // Test 6: Oversized patch rejection (>64KB)
    std::cout << "[Test 6] Oversized patch (>64KB)...\n";
    {
        std::string huge_patch(65 * 1024, 'A');
        MockLLMClient mock(MockLLMClient::Behavior::SUCCESS, huge_patch);
        AICodeFixer fixer(mock);
        auto res = fixer.fix("Error log", "Context");

        CHECK(!res.success, "Patch > 64KB harus ditolak");
        CHECK(res.error == "patch too large (>64KB)", "Error message harus 'patch too large (>64KB)'");
    }
    std::cout << "PASS\n\n";

    std::cout << "========================================\n";
    std::cout << "AI CODE FIXER SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
