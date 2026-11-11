#ifndef GARUDA_BEAM_INTEROP_EMBEDDING_API_HPP
#define GARUDA_BEAM_INTEROP_EMBEDDING_API_HPP

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <memory>
#include <optional>
#include <iostream>

#include "garuda/beam/gvm/actor_runtime.hpp"
#include "garuda/compiler/lexer.hpp"
#include "garuda/compiler/parser.hpp"
#include "garuda/compiler/codegen.hpp"

namespace garuda {
namespace beam {
namespace interop {

class GvmEmbeddingHost {
private:
    std::shared_ptr<GvmActorRuntime> runtime_;
    std::vector<Instruction> loaded_instructions_;
    std::unordered_map<std::string, size_t> loaded_function_table_;
    std::map<std::string, Term> global_terms_;

public:
    GvmEmbeddingHost() = default;

    bool load_source(const std::string& source_code) {
        using namespace garuda::compiler;
        try {
            Lexer lexer(source_code);
            Parser parser(lexer.tokenize());
            auto ast_module = parser.parse_program();

            CodeGenerator codegen;
            auto output = codegen.generate(ast_module);

            loaded_instructions_ = output.all_instructions;
            loaded_function_table_ = output.function_table;

            runtime_ = std::make_shared<GvmActorRuntime>(loaded_instructions_, loaded_function_table_);
            return true;
        } catch (const std::exception& e) {
            std::cerr << "[GvmEmbeddingHost] Error compiling source: " << e.what() << std::endl;
            return false;
        }
    }

    bool load_bytecode(const std::vector<Instruction>& instructions,
                      const std::unordered_map<std::string, size_t>& function_table) {
        loaded_instructions_ = instructions;
        loaded_function_table_ = function_table;
        runtime_ = std::make_shared<GvmActorRuntime>(loaded_instructions_, loaded_function_table_);
        return true;
    }

    void set_global(const std::string& name, const Term& value) {
        global_terms_[name] = value;
    }

    std::optional<Term> get_global(const std::string& name) const {
        auto it = global_terms_.find(name);
        if (it != global_terms_.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    std::optional<Term> call_function(const std::string& func_name) {
        if (!runtime_) {
            std::cerr << "[GvmEmbeddingHost] Error: Runtime not initialized. Call load_source() or load_bytecode() first." << std::endl;
            return std::nullopt;
        }

        if (loaded_function_table_.find(func_name) == loaded_function_table_.end()) {
            std::cerr << "[GvmEmbeddingHost] Error: Function '" << func_name << "' not found in loaded module." << std::endl;
            return std::nullopt;
        }

        return runtime_->run_until_done(func_name);
    }

    bool has_function(const std::string& func_name) const {
        return loaded_function_table_.find(func_name) != loaded_function_table_.end();
    }

    size_t get_instruction_count() const {
        return loaded_instructions_.size();
    }
};

} // namespace interop
} // namespace beam
} // namespace garuda

#endif // GARUDA_BEAM_INTEROP_EMBEDDING_API_HPP
