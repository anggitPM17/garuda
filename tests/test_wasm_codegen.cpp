// =====================================================================
// GARUDA COMPILER — FASE 18 WEBASSEMBLY (WASM/WASI) AOT SUITE (4 TESTS)
// =====================================================================

#include "../include/garuda/compiler/lexer.hpp"
#include "../include/garuda/compiler/parser.hpp"
#include "../include/garuda/compiler/wasm_codegen.hpp"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

using namespace garuda::compiler;
namespace fs = std::filesystem;

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        std::cerr << "[FAIL] " << msg << "\n"; \
        return 1; \
    } \
} while(0)

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA COMPILER — FASE 18 WEBASSEMBLY (WASM/WASI) AOT SUITE\n";
    std::cout << "=======================================================\n\n";

    fs::path tmp_dir = fs::temp_directory_path() / "garuda_wasm_test";
    fs::create_directories(tmp_dir);

    // === Test 1: WASM Header & Magic Number Validation ===
    std::cout << "[TEST 1/4] Running WASM Header Validation test...\n";
    CodeGeneratorOutput gvm_out;
    gvm_out.module_name = "TesWasm";
    gvm_out.all_instructions = {
        Instruction{Opcode::OpMove, {Operand{RegType::Literal, 0, Term(42LL)}}, 0, 1},
        Instruction{Opcode::OpReturn, {}, 0, 2}
    };
    gvm_out.functions = {{"utama", 0, 0, {}}};

    WasmCodeGenerator wasm_cg;
    auto wasm_out = wasm_cg.generate_wasm(gvm_out);

    CHECK(wasm_out.wasm_bytes.size() > 8, "WASM bytes length must be > 8 bytes");
    CHECK(wasm_out.wasm_bytes[0] == 0x00 && wasm_out.wasm_bytes[1] == 0x61 &&
          wasm_out.wasm_bytes[2] == 0x73 && wasm_out.wasm_bytes[3] == 0x6d, "Magic bytes must be \\0asm");
    CHECK(wasm_out.wasm_bytes[4] == 0x01 && wasm_out.wasm_bytes[5] == 0x00 &&
          wasm_out.wasm_bytes[6] == 0x00 && wasm_out.wasm_bytes[7] == 0x00, "Version bytes must be 1.0.0.0");
    std::cout << "  ✓ Test 1: WASM Header Validation PASS\n";

    // === Test 2: WASI System Call Import Verification ===
    std::cout << "[TEST 2/4] Running WASI Import Verification test...\n";
    CHECK(wasm_out.has_wasi_imports, "WASM module must indicate WASI imports presence");
    CHECK(wasm_out.exported_function_count == 3, "WASM module must export 3 items (_start, utama, memory)");
    std::cout << "  ✓ Test 2: WASI Import Verification PASS\n";

    // === Test 3: Binary .wasm File I/O ===
    std::cout << "[TEST 3/4] Running Binary .wasm File I/O test...\n";
    fs::path wasm_file = tmp_dir / "tes_wasm.wasm";
    bool file_ok = WasmCodeGenerator::write_wasm_file(wasm_out, wasm_file.string());

    CHECK(file_ok, "write_wasm_file must return true");
    CHECK(fs::exists(wasm_file), "Generated .wasm file must exist on disk");
    CHECK(fs::file_size(wasm_file) == wasm_out.wasm_bytes.size(), "File size on disk must match bytecode vector size");
    std::cout << "  ✓ Test 3: Binary .wasm File I/O PASS\n";

    // === Test 4: End-to-End Compiler -> WASM Translation ===
    std::cout << "[TEST 4/4] Running End-to-End Compiler -> WASM Translation test...\n";
    std::string src = R"(
        modul WasmApp
        fungsi tambah(a, b) ->
            a + b
        rampung
        fungsi utama() ->
            tambah(10, 32)
        rampung
    )";

    Lexer lexer(src);
    Parser parser(lexer.tokenize());
    CodeGenerator cg;
    auto orig_gvm = cg.generate(parser.parse_program());

    auto full_wasm = wasm_cg.generate_wasm(orig_gvm);
    fs::path app_wasm_file = tmp_dir / "wasm_app.wasm";
    bool app_ok = WasmCodeGenerator::write_wasm_file(full_wasm, app_wasm_file.string());

    CHECK(app_ok && fs::exists(app_wasm_file), "Full WASM compilation & disk write must pass");
    std::cout << "  ✓ Test 4: End-to-End Compiler -> WASM Translation PASS\n";

    // Cleanup
    fs::remove_all(tmp_dir);

    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL 4 WEBASSEMBLY AOT & WASI TESTS PASSED PERFECTLY!\n";
    std::cout << "========================================\n";
    return 0;
}
