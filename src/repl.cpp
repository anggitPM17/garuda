#pragma once
// Garuda Language CLI: Interactive REPL Engine (`repl.cpp`)
// Evaluasi real-time ekspresi Garuda, deteksi multi-line, dan penanganan galat aman.
#include "../include/garuda/compiler/lexer.hpp"
#include "../include/garuda/compiler/parser.hpp"
#include "../include/garuda/compiler/codegen.hpp"
#include "../include/garuda/beam/gvm/actor_runtime.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace garuda::cli {

using namespace garuda::compiler;
using namespace garuda::beam;

class Repl {
public:
    Repl() = default;

    void run() {
        std::cout << "Garuda Interactive REPL Shell v0.4.0 🦅\n";
        std::cout << "GVM BEAM-Core Engine. Ketik :help untuk bantuan, :quit atau :q untuk keluar.\n\n";

        std::string buffer;
        while (true) {
            print_prompt(!buffer.empty());
            std::string line;
            if (!std::getline(std::cin, line)) break;

            if (buffer.empty()) {
                if (line == ":quit" || line == ":q" || line == "keluar" || line == "exit") break;
                if (line == ":help") { cmd_help(); continue; }
                if (line == ":reset") { cmd_reset(); continue; }
                if (line.empty()) continue;
            }

            buffer += line + "\n";

            if (is_incomplete(buffer)) continue;

            eval_input(buffer);
            buffer.clear();
        }
    }

    void eval_input(const std::string& input) {
        try {
            std::string code = input;
            // Trim
            std::size_t first = code.find_first_not_of(" \t\n\r");
            if (first != std::string::npos) code = code.substr(first);

            std::string wrapped;
            if (code.rfind("modul ", 0) == 0) {
                wrapped = code;
            } else if (code.rfind("fungsi ", 0) == 0) {
                wrapped = "modul Repl " + code + " fungsi utama() -> :ok rampung rampung";
            } else {
                wrapped = "modul Repl fungsi utama() -> " + code + " rampung rampung";
            }

            Lexer lexer(wrapped);
            Parser parser(lexer.tokenize());
            auto ast = parser.parse_program();

            CodeGenerator codegen;
            auto output = codegen.generate(ast);

            GvmActorRuntime runtime(output.all_instructions, output.function_table);
            runtime.set_verbose(false);
            auto result = runtime.run_until_done("utama", 50000);

            if (result.has_value()) {
                std::cout << "=> " << to_string(*result) << "\n";
            }
        } catch (const std::exception& e) {
            std::cout << "❌ Galat REPL: " << e.what() << "\n";
        }
    }

    bool is_incomplete(const std::string& buffer) const {
        std::size_t open_count = 0;
        std::size_t close_count = 0;

        std::istringstream ss(buffer);
        std::string token;
        while (ss >> token) {
            if (token == "fungsi" || token == "def" ||
                token == "yen" || token == "if" ||
                token == "tinjau" || token == "case" ||
                token == "tampa" || token == "receive" ||
                token == "modul" || token == "defmodule") {
                open_count++;
            } else if (token == "rampung" || token == "end") {
                close_count++;
            }
        }

        return open_count > close_count;
    }

    void print_prompt(bool continuation) const {
        if (continuation) {
            std::cout << "... ";
        } else {
            std::cout << "garuda> ";
        }
        std::cout.flush();
    }

    void cmd_help() const {
        std::cout << "Garuda REPL Commands:\n";
        std::cout << "  :help   - Tampilkan bantuan ini\n";
        std::cout << "  :reset  - Reset buffer REPL\n";
        std::cout << "  :quit   - Keluar dari REPL (:q)\n\n";
        std::cout << "Contoh:\n";
        std::cout << "  garuda> 1 + 2\n";
        std::cout << "  => 3\n";
        std::cout << "  garuda> \"halo\" ++ \" dunia\"\n";
        std::cout << "  => \"halo dunia\"\n\n";
    }

    void cmd_reset() {
        accumulated_source_.clear();
        std::cout << "✅ REPL state di-reset.\n";
    }

private:
    std::string accumulated_source_;
};

inline void run_repl() {
    Repl r;
    r.run();
}

} // namespace garuda::cli
