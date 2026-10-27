#include "../include/garuda/beam/tools/doc_generator.hpp"
#include <chrono>
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

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA AUTOMATED DOC GENERATOR SUITE\n";
    std::cout << "=======================================================\n\n";

    fs::path tmp_dir = fs::temp_directory_path() / "garuda_doc_test";
    fs::remove_all(tmp_dir);
    fs::create_directories(tmp_dir);

    fs::path sample_grd = tmp_dir / "math.grd";
    {
        std::ofstream ofs(sample_grd, std::ios::trunc);
        ofs << "/// Menghitung faktorial dari n.\n"
            << "/// Parameter:\n"
            << "///   n - bilangan bulat positif\n"
            << "fun fakt(n) ->\n"
            << "    yen n <= 1 -> 1\n"
            << "    lain -> n * fakt(n - 1)\n";
        ofs.close();
    }

    // === Test 1: DocComment Extraction & Parsing ===
    run_test_with_timeout("Test 1: DocComment Extraction & Parsing", [&]() {
        DocPage page = DocGenerator::parse_file(sample_grd.string());
        GARUDA_CHECK(page.file_name == "math.grd", "File name must match 'math.grd'");
        GARUDA_CHECK(page.items.size() == 1, "Must extract exactly 1 function doc item");
        GARUDA_CHECK(page.items[0].func_name == "fakt", "Extracted function name must be 'fakt'");
        GARUDA_CHECK(page.items[0].doc_comment.find("Menghitung faktorial dari n.") != std::string::npos, "Doc comment must contain 'Menghitung faktorial dari n.'");

        std::cout << "  ✓ Test 1: DocComment Extraction & Parsing PASS (Extracted fn: " << page.items[0].func_name << ")\n";
    }, 5);

    // === Test 2: Single Page HTML Generation ===
    run_test_with_timeout("Test 2: Single Page HTML Generation", [&]() {
        DocPage page = DocGenerator::parse_file(sample_grd.string());
        std::string html = DocGenerator::generate_html(page);

        GARUDA_CHECK(html.find("<!DOCTYPE html>") != std::string::npos, "HTML must start with DOCTYPE");
        GARUDA_CHECK(html.find("fn fakt") != std::string::npos, "HTML must display 'fn fakt'");
        GARUDA_CHECK(html.find("Menghitung faktorial dari n.") != std::string::npos, "HTML must contain doc comment text");

        std::cout << "  ✓ Test 2: Single Page HTML Generation PASS\n";
    }, 5);

    // === Test 3: Generate All Directory Index ===
    run_test_with_timeout("Test 3: Generate All Directory Index", [&]() {
        std::string all_html = DocGenerator::generate_all(tmp_dir.string());

        GARUDA_CHECK(all_html.find("Garuda API Documentation Index") != std::string::npos, "Combined HTML must contain Index title");
        GARUDA_CHECK(all_html.find("math.grd") != std::string::npos, "Combined HTML must list 'math.grd'");
        GARUDA_CHECK(all_html.find("fn fakt") != std::string::npos, "Combined HTML must list 'fn fakt'");

        std::cout << "  ✓ Test 3: Generate All Directory Index PASS\n";
    }, 5);

    fs::remove_all(tmp_dir);

    std::cout << "\n🎉 ALL AUTOMATED DOC GENERATOR TESTS PASSED PERFECTLY!\n";
    return 0;
}
