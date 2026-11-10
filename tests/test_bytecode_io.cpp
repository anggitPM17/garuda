#include "../include/garuda/compiler/lexer.hpp"
#include "../include/garuda/compiler/parser.hpp"
#include "../include/garuda/compiler/codegen.hpp"
#include "../include/garuda/compiler/bytecode_writer.hpp"
#include "../include/garuda/compiler/bytecode_reader.hpp"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>

using namespace garuda::compiler;
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
    std::cout << "GARUDA COMPILER — BYTECODE I/O & CHECKSUM SUITE\n";
    std::cout << "=======================================================\n\n";

    fs::path tmp_dir = fs::temp_directory_path() / "garuda_bytecode_io_test";
    fs::create_directories(tmp_dir);

    // Prepare valid source bytecode
    std::string src = R"(
        modul TesBytecode
        fungsi tambah(A, B) ->
            A + B
        rampung
        fungsi utama() ->
            tambah(10, 20)
        rampung
    )";

    Lexer lexer(src);
    Parser parser(lexer.tokenize());
    auto ast = parser.parse_program();
    CodeGenerator cg;
    auto orig_output = cg.generate(ast);

    // === Test 1: Write & Read Roundtrip ===
    run_test_with_timeout("Test 1: Write & Read Roundtrip", [&]() {
        fs::path gvm_file = tmp_dir / "valid.gvm";
        bool write_ok = BytecodeWriter::write(orig_output, gvm_file.string());
        GARUDA_CHECK(write_ok, "BytecodeWriter::write must succeed");
        GARUDA_CHECK(fs::exists(gvm_file), "GVM file must exist on disk");

        auto read_opt = BytecodeReader::read(gvm_file.string());
        GARUDA_CHECK(read_opt.has_value(), "BytecodeReader::read must succeed for valid file");

        auto& read_output = *read_opt;
        GARUDA_CHECK(read_output.module_name == orig_output.module_name, "Module names must match");
        GARUDA_CHECK(read_output.all_instructions.size() == orig_output.all_instructions.size(), "Instruction counts must match");
        GARUDA_CHECK(read_output.functions.size() == orig_output.functions.size(), "Function counts must match");

        std::cout << "  ✓ Test 1: Write & Read Roundtrip PASS\n";
    }, 5);

    // === Test 2: Checksum Validation (Corrupted Byte Detection) ===
    run_test_with_timeout("Test 2: Checksum Validation (Corrupted File)", [&]() {
        fs::path gvm_file = tmp_dir / "valid.gvm";
        fs::path corrupt_file = tmp_dir / "corrupted.gvm";

        std::ifstream ifs(gvm_file.string(), std::ios::binary);
        std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        ifs.close();

        // Corrupt 1 byte in the body section (offset 35)
        if (content.length() > 35) {
            content[35] = static_cast<char>(content[35] ^ 0xFF);
        }

        std::ofstream ofs(corrupt_file.string(), std::ios::binary);
        ofs.write(content.data(), content.length());
        ofs.close();

        auto read_opt = BytecodeReader::read(corrupt_file.string());
        GARUDA_CHECK(!read_opt.has_value(), "BytecodeReader::read must fail on checksum mismatch");

        std::cout << "  ✓ Test 2: Checksum Validation (Corrupted File) PASS\n";
    }, 5);

    // === Test 3: Magic Number Validation ===
    run_test_with_timeout("Test 3: Magic Number Validation", [&]() {
        fs::path invalid_magic_file = tmp_dir / "bad_magic.gvm";

        std::string bad_header = "BADM0000000000000000000000000000"; // 32 bytes with invalid magic 'BADM'
        std::ofstream ofs(invalid_magic_file.string(), std::ios::binary);
        ofs.write(bad_header.data(), bad_header.length());
        ofs.close();

        auto read_opt = BytecodeReader::read(invalid_magic_file.string());
        GARUDA_CHECK(!read_opt.has_value(), "BytecodeReader::read must fail on invalid magic number");

        std::cout << "  ✓ Test 3: Magic Number Validation PASS\n";
    }, 5);

    // === Test 4: Truncated File Handling ===
    run_test_with_timeout("Test 4: Truncated File Handling", [&]() {
        fs::path gvm_file = tmp_dir / "valid.gvm";
        fs::path trunc_file = tmp_dir / "truncated.gvm";

        std::ifstream ifs(gvm_file.string(), std::ios::binary);
        std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        ifs.close();

        // Truncate file in half
        std::size_t half_len = content.length() / 2;
        std::ofstream ofs(trunc_file.string(), std::ios::binary);
        ofs.write(content.data(), half_len);
        ofs.close();

        auto read_opt = BytecodeReader::read(trunc_file.string());
        GARUDA_CHECK(!read_opt.has_value(), "BytecodeReader::read must fail gracefully on truncated file");

        std::cout << "  ✓ Test 4: Truncated File Handling PASS\n";
    }, 5);

    // Cleanup
    fs::remove_all(tmp_dir);

    std::cout << "\n🎉 ALL BYTECODE I/O & CHECKSUM TESTS PASSED PERFECTLY!\n";
    return 0;
}
