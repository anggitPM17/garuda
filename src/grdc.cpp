#pragma once
// Garuda Language CLI Compiler & Runner (`grdc.cpp`)
// Menawarkan perintah CLI resmi: grdc compile <file.grd>, grdc run <file.grd|file.gvm>, grdc fmt <file.grd>, grdc lint <file.grd>, grdc repl, grdc version.
#include "../include/garuda/beam/beam.hpp"
#include "../include/garuda/compiler/lexer.hpp"
#include "../include/garuda/compiler/parser.hpp"
#include "../include/garuda/compiler/codegen.hpp"
#include "../include/garuda/compiler/bytecode_writer.hpp"
#include "../include/garuda/compiler/bytecode_reader.hpp"
#include "../include/garuda/compiler/formatter.hpp"
#include "../include/garuda/compiler/linter.hpp"
#include "../include/garuda/beam/gvm/actor_runtime.hpp"
#include "../include/garuda/beam/micro/gvm_micro.hpp"
#include "repl.cpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace fs = std::filesystem;

using namespace garuda::compiler;
using namespace garuda::beam;

void print_version() {
    std::cout << "Garuda Language Compiler (grdc) v1.0.0\n";
    std::cout << "GVM BEAM-Core Runtime v67.0.0-production (C++20)\n";
    std::cout << "Copyright (C) 2026 Garuda Language Project. All rights reserved.\n";
}

void print_help() {
    std::cout << "Penggunaan CLI Garuda:\n";
    std::cout << "  grdc compile <file.grd>      Kompilasi kode sumber ke berkas biner .gvm\n";
    std::cout << "  grdc run <file.grd|file.gvm> Kompilasi (bila .grd) dan eksekusi langsung\n";
    std::cout << "  grdc fmt <file.grd>          Auto-format berkas kode sumber Garuda\n";
    std::cout << "  grdc lint <file.grd>         Jalankan analisis linter gaya penulisan kode\n";
    std::cout << "  grdc repl                   Jalankan Interactive REPL Shell\n";
    std::cout << "  grdc version                Tampilkan versi compiler Garuda\n";
}

std::optional<CodeGeneratorOutput> compile_grd_source(const std::string& source_path) {
    fs::path p(source_path);
    if (!fs::exists(p)) {
        fs::path alt1 = fs::current_path() / p;
        if (fs::exists(alt1)) {
            p = alt1;
        } else {
            fs::path alt2 = fs::current_path() / "garuda_v67_0_production" / "garuda_v67.0.0" / p;
            if (fs::exists(alt2)) {
                p = alt2;
            }
        }
    }

    std::ifstream ifs(p);
    if (!ifs.is_open()) {
        std::cerr << "⚠️ Error: Tidak dapat membuka berkas sumber '" << source_path << "' (Attempted path: " << fs::absolute(p).string() << ")\n";
        return std::nullopt;
    }
    std::string source((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    ifs.close();

    Lexer lexer(source);
    Parser parser(lexer.tokenize());
    auto ast_module = parser.parse_program();

    CodeGenerator codegen;
    return codegen.generate(ast_module);
}

#ifndef GARUDA_GRDC_NO_MAIN
int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_help();
        return 0;
    }

    std::string command = argv[1];

    if (command == "version" || command == "--version" || command == "-v") {
        print_version();
        return 0;
    }

    if (command == "fmt") {
        if (argc < 3) {
            std::cerr << "⚠️ Error: Harap tentukan berkas .grd yang ingin diformat.\n";
            return 1;
        }
        std::string filepath = argv[2];
        std::ifstream ifs(filepath);
        if (!ifs.is_open()) return 1;
        std::string src((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        ifs.close();

        Lexer lexer(src);
        Parser parser(lexer.tokenize());
        auto module = parser.parse_program();

        Formatter fmt;
        std::string formatted = fmt.format(module);

        std::ofstream ofs(filepath);
        ofs << formatted;
        ofs.close();
        std::cout << "✨ Berkas '" << filepath << "' berhasil diformat!\n";
        return 0;
    }

    if (command == "lint") {
        if (argc < 3) {
            std::cerr << "⚠️ Error: Harap tentukan berkas .grd yang ingin di-lint.\n";
            return 1;
        }
        std::string filepath = argv[2];
        std::ifstream ifs(filepath);
        if (!ifs.is_open()) return 1;
        std::string src((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        ifs.close();

        Lexer lexer(src);
        Parser parser(lexer.tokenize());
        auto module = parser.parse_program();

        Linter linter;
        auto warnings = linter.lint(module);

        if (warnings.empty()) {
            std::cout << "✨ Kode '" << filepath << "' bersih! 0 peringatan linter.\n";
        } else {
            std::cout << "⚠️ Ditemukan " << warnings.size() << " peringatan linter:\n";
            for (const auto& w : warnings) {
                std::cout << "  - " << filepath << ":" << w.line << ":" << w.col << " - " << w.message << "\n";
            }
        }
        return 0;
    }

    if (command == "micro") {
        if (argc < 3) {
            std::cerr << "⚠️ Error: Harap tentukan berkas .grd atau .gvm untuk GVM Micro.\n";
            return 1;
        }
        std::string filepath = argv[2];
        if (!fs::exists(filepath) && fs::exists(fs::current_path() / filepath)) {
            filepath = (fs::current_path() / filepath).string();
        }

        std::optional<CodeGeneratorOutput> code_output;

        if (filepath.rfind(".gvm") != std::string::npos) {
            code_output = BytecodeReader::read(filepath);
        } else {
            code_output = compile_grd_source(filepath);
        }

        if (!code_output.has_value() || code_output->all_instructions.empty()) {
            std::cerr << "❌ Error: GVM Micro Gagal membaca/mengompilasi bytecode dari '" << filepath << "'\n";
            return 1;
        }

        garuda::beam::micro::GvmMicroEngine micro_engine;
        std::cout << "⚡ [GARUDA MICRO] Executing '" << filepath << "' (" << code_output->all_instructions.size() << " opcodes) via GVM Micro Embedded Runtime..." << std::endl;
        std::int64_t res = micro_engine.execute_bytecode(code_output->all_instructions);
        std::cout << "✅ [GARUDA MICRO] Result: " << res << std::endl;
        return 0;
    }

    if (command == "compile") {
        if (argc < 3) {
            std::cerr << "⚠️ Error: Harap tentukan berkas .grd yang ingin dikompilasi.\n";
            return 1;
        }
        std::string source_path = argv[2];
        auto output = compile_grd_source(source_path);
        if (!output.has_value()) return 1;

        std::string out_path = source_path;
        auto pos = out_path.rfind(".grd");
        if (pos != std::string::npos) {
            out_path.replace(pos, 4, ".gvm");
        } else {
            out_path += ".gvm";
        }

        if (BytecodeWriter::write(*output, out_path)) {
            std::cout << "🎉 Kompilasi sukses! Berkas biner: " << out_path << "\n";
            return 0;
        }
        return 1;
    }

    if (command == "run") {
        if (argc < 3) {
            std::cerr << "⚠️ Error: Harap tentukan berkas .grd atau .gvm yang ingin dijalankan.\n";
            return 1;
        }
        std::string filepath = argv[2];
        std::optional<CodeGeneratorOutput> code_output;

        if (filepath.rfind(".gvm") != std::string::npos) {
            code_output = BytecodeReader::read(filepath);
        } else {
            code_output = compile_grd_source(filepath);
        }

        if (!code_output.has_value()) return 1;

        GvmActorRuntime runtime(code_output->all_instructions, code_output->function_table);
        auto result = runtime.run_until_done("utama");
        if (result.has_value()) {
            std::cout << "\n✅ Hasil Eksekusi Program: " << to_string(*result) << "\n";
        }
        return 0;
    }

    if (command == "repl") {
        garuda::cli::run_repl();
        return 0;
    }

    std::cerr << "⚠️ Error: Perintah '" << command << "' tidak dikenal. Jalankan 'grdc' tanpa argumen untuk bantuan.\n";
    return 1;
}
#endif
