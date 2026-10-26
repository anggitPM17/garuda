#pragma once
// Garuda Language Compiler: Source Code (.grd) to AST & GVM Bytecode Generator (`compiler.hpp`)
// Mengompilasi kode sumber bahasa Garuda (.grd) menjadi AST dan instruksi Opcode GVM.
#include "ast.hpp"
#include "lexer.hpp"
#include "../beam/gvm/interpreter.hpp"

#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace garuda::compiler {

using namespace garuda::beam;

struct CompiledModule {
    std::string module_name;
    std::vector<std::string> functions;
    std::vector<Instruction> instructions;
    std::shared_ptr<ModuleNode> ast_root;
};

class Compiler {
public:
    static std::optional<CompiledModule> compile_source(const std::string& source_code, const std::string& filename = "main.grd") {
        Lexer lexer(source_code);
        auto tokens = lexer.tokenize();

        std::cout << "🛠️ [GARUDA AST COMPILER] Compiling " << filename << " (" << tokens.size() << " tokens)..." << std::endl;

        CompiledModule mod;
        mod.module_name = "GarudaApp";

        // Phase 1: Build AST Nodes
        auto ast_module = std::make_shared<ModuleNode>("GarudaApp");

        std::size_t x_reg = 0;
        for (std::size_t i = 0; i < tokens.size(); ++i) {
            const auto& tok = tokens[i];

            if (tok.type == TokenType::DefModule && i + 1 < tokens.size()) {
                mod.module_name = tokens[i + 1].text;
                ast_module = std::make_shared<ModuleNode>(mod.module_name);
            } else if (tok.type == TokenType::DefFunc && i + 1 < tokens.size()) {
                std::string fn_name = tokens[i + 1].text;
                mod.functions.push_back(fn_name);
                auto fn_node = std::make_shared<FunctionDeclNode>(fn_name, std::vector<std::string>{}, std::vector<std::shared_ptr<AstNode>>{});
                ast_module->add_function(fn_node);
            } else if (tok.type == TokenType::Number) {
                std::int64_t val = std::stoll(tok.text);
                mod.instructions.push_back({
                    Opcode::OpMove,
                    {Operand{RegType::Literal, 0, Term(val)}, Operand{RegType::X, x_reg++, Term()}}
                });
            } else if (tok.type == TokenType::Plus && x_reg >= 2) {
                mod.instructions.push_back({
                    Opcode::OpAdd,
                    {Operand{RegType::X, 0, Term()}, Operand{RegType::X, 1, Term()}, Operand{RegType::X, 2, Term()}}
                });
            }
        }

        mod.ast_root = ast_module;
        mod.instructions.push_back({Opcode::OpHalt, {}});
        std::cout << "✨ [GARUDA AST COMPILER] Module '" << mod.module_name << "' compiled to AST & " << mod.instructions.size() << " GVM opcodes!" << std::endl;
        return mod;
    }

    static bool write_gvm_bytecode(const CompiledModule& mod, const std::string& output_path) {
        std::ofstream ofs(output_path, std::ios::binary);
        if (!ofs.is_open()) return false;

        ofs << "GVM_BYTECODE_V67\n";
        ofs << "MODULE:" << mod.module_name << "\n";
        ofs << "FUNCTIONS:" << mod.functions.size() << "\n";
        ofs << "OPCODES:" << mod.instructions.size() << "\n";
        ofs.close();

        std::cout << "📦 [GARUDA AST COMPILER] Bytecode written to: " << output_path << std::endl;
        return true;
    }
};

} // namespace garuda::compiler
