#define GARUDA_GRDMIX_NO_MAIN
#include "../src/grdmix.cpp"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>

using namespace garuda::cli;
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
        std::exit(1);
    }
}

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA CLI — GRDMIX END-TO-END SUITE\n";
    std::cout << "=======================================================\n\n";

    fs::path tmp_dir = fs::temp_directory_path() / "temp_grdmix_test";
    fs::remove_all(tmp_dir);

    try {
        // === Test 1: grdmix init in temporary folder ===
        run_test_with_timeout("Test 1: grdmix init in temporary directory", [&tmp_dir]() {
            fs::create_directories(tmp_dir);
            int res = Grdmix::init_project(tmp_dir);
            GARUDA_CHECK(res == 0, "grdmix init must return 0");
            GARUDA_CHECK(fs::exists(tmp_dir / "grdmix.toml"), "grdmix.toml must be created");
            std::cout << "  ✓ Test 1: grdmix init PASS\n";
        }, 5);

        // === Test 2: Custom .grd source file creation ===
        run_test_with_timeout("Test 2: Custom .grd source file creation", [&tmp_dir]() {
            fs::path main_src = tmp_dir / "src" / "utama.grd";
            std::ofstream src_ofs(main_src);
            src_ofs << "modul HaloProyek\n\n";
            src_ofs << "fungsi utama() ->\n";
            src_ofs << "    cetak(\"Halo saking grdmix e2e!\")\n";
            src_ofs << "    100\n";
            src_ofs << "rampung\n";
            src_ofs.close();

            GARUDA_CHECK(fs::exists(main_src), "src/utama.grd must exist");
            std::cout << "  ✓ Test 2: Custom .grd source file creation PASS\n";
        }, 5);

        // === Test 3: grdmix compile / build ===
        run_test_with_timeout("Test 3: grdmix compile / build", [&tmp_dir]() {
            int res = Grdmix::compile(tmp_dir);
            GARUDA_CHECK(res == 0, "grdmix compile must return 0");
            GARUDA_CHECK(fs::exists(tmp_dir / "src" / "utama.gvm"), "Bytecode src/utama.gvm must be created");
            std::cout << "  ✓ Test 3: grdmix compile / build PASS\n";
        }, 5);

        // === Test 4: grdmix run execution ===
        run_test_with_timeout("Test 4: grdmix run execution", [&tmp_dir]() {
            int res = Grdmix::run(tmp_dir);
            GARUDA_CHECK(res == 0, "grdmix run must return 0");
            std::cout << "  ✓ Test 4: grdmix run execution PASS\n";
        }, 5);

        // Cleanup
        fs::remove_all(tmp_dir);
    } catch (...) {
        fs::remove_all(tmp_dir);
        throw;
    }

    std::cout << "\n🎉 ALL GRDMIX END-TO-END TESTS PASSED PERFECTLY!\n";
    return 0;
}
