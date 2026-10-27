#include "../include/garuda/beam/io/filesystem.hpp"
#include "../include/garuda/beam/io/process.hpp"
#include <chrono>
#include <filesystem>
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

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA FILESYSTEM & PROCESS DESKTOP TOOLCHAIN SUITE\n";
    std::cout << "=======================================================\n\n";

    fs::path tmp_base = fs::temp_directory_path() / "garuda_fs_proc_test";
    fs::remove_all(tmp_base);

    // === Test 1: Filesystem Write & Read ===
    run_test_with_timeout("Test 1: Filesystem Write & Read", [&]() {
        fs::path test_file = tmp_base / "hello.txt";
        bool written = Fs::write_file(test_file.string(), "Halo Garuda");
        GARUDA_CHECK(written, "Fs::write_file must succeed");
        GARUDA_CHECK(Fs::exists(test_file.string()), "File must exist on disk");

        std::string read_content = Fs::read_file(test_file.string());
        GARUDA_CHECK(read_content == "Halo Garuda", "Read content must match 'Halo Garuda'");

        bool removed = Fs::remove_file(test_file.string());
        GARUDA_CHECK(removed, "Fs::remove_file must succeed");
        GARUDA_CHECK(!Fs::exists(test_file.string()), "File must no longer exist after removal");

        std::cout << "  ✓ Test 1: Filesystem Write & Read PASS\n";
    }, 5);

    // === Test 2: Filesystem Directory Operations ===
    run_test_with_timeout("Test 2: Filesystem Directory Operations", [&]() {
        fs::path test_dir = tmp_base / "subdir";
        bool created = Fs::create_dir(test_dir.string());
        GARUDA_CHECK(created, "Fs::create_dir must succeed");
        GARUDA_CHECK(Fs::exists(test_dir.string()), "Directory must exist on disk");

        fs::path file_inside = test_dir / "item.txt";
        Fs::write_file(file_inside.string(), "item data");

        auto list = Fs::list_dir(test_dir.string());
        GARUDA_CHECK(list.size() == 1 && list[0] == "item.txt", "list_dir must contain 'item.txt'");

        Fs::remove_file(tmp_base.string());
        GARUDA_CHECK(!Fs::exists(tmp_base.string()), "Base directory must be cleaned up completely");

        std::cout << "  ✓ Test 2: Filesystem Directory Operations PASS\n";
    }, 5);

    // === Test 3: Process Execution (Echo) ===
    run_test_with_timeout("Test 3: Process Execution (Echo)", []() {
        auto [code, output] = Process::run("echo Hello from Process");
        GARUDA_CHECK(code == 0, "Process exit code must be 0 for echo");
        GARUDA_CHECK(output.find("Hello from Process") != std::string::npos, "Output must contain 'Hello from Process'");

        std::cout << "  ✓ Test 3: Process Execution (Echo) PASS (Output: " << output.substr(0, output.length() - 1) << ")\n";
    }, 5);

    // === Test 4: Process Failure Exit Code ===
    run_test_with_timeout("Test 4: Process Failure Exit Code", []() {
#if defined(_WIN32)
        auto [code, output] = Process::run("cmd /c exit 1");
#else
        auto [code, output] = Process::run("exit 1");
#endif
        GARUDA_CHECK(code != 0, "Process exit code must be non-zero for failed command");

        std::cout << "  ✓ Test 4: Process Failure Exit Code PASS (Exit Code: " << code << ")\n";
    }, 5);

    std::cout << "\n🎉 ALL FILESYSTEM & PROCESS DESKTOP TOOLCHAIN TESTS PASSED PERFECTLY!\n";
    return 0;
}
