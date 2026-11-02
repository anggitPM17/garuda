#pragma once
// Garuda Language Compiler: Code Generator (`codegen.hpp`)
// Menelusuri AST, melakukan alokasi register X (256 standard), dan menghasilkan instruksi Opcode GVM.
#include "ast.hpp"
#include "../beam/gvm/interpreter.hpp"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace garuda::compiler {

using namespace garuda::beam;

struct CompiledFunctionInfo {
    std::string name;
    std::uint8_t arity = 0;
    std::size_t start_ip = 0;
    std::vector<Instruction> instructions;
};

struct CodeGeneratorOutput {
    std::string module_name = "Main";
    std::vector<CompiledFunctionInfo> functions;
    std::vector<Instruction> all_instructions;
    std::unordered_map<std::string, std::size_t> function_table;
};

class CodegenError : public std::runtime_error {
public:
    explicit CodegenError(const std::string& msg) : std::runtime_error(msg) {}
};

class CodeGenerator {
public:
    CodeGenerator() = default;

    CodeGeneratorOutput generate(std::shared_ptr<ModuleNode> module) {
        CodeGeneratorOutput output;
        if (!module) return output;

        module_name_ = module->name();
        output.module_name = module->name();

        for (const auto& fn : module->functions()) {
            auto state = std::make_shared<FuncState>();
            state->name = fn->name();
            state->base_global_ip = global_instructions_.size();
            current_func_ = state;

            gen_function(fn, state);

            CompiledFunctionInfo fn_info;
            fn_info.name = fn->name();
            fn_info.arity = static_cast<std::uint8_t>(fn->params().size());
            fn_info.start_ip = state->base_global_ip;
            fn_info.instructions = state->instructions;
            output.functions.push_back(fn_info);

            std::string qualified_name = output.module_name + "." + fn->name();
            output.function_table[qualified_name] = state->base_global_ip;
            output.function_table[qualified_name + "/" + std::to_string(fn_info.arity)] = state->base_global_ip;
            output.function_table[fn->name()] = state->base_global_ip;
            output.function_table[fn->name() + "/" + std::to_string(fn_info.arity)] = state->base_global_ip;
            function_ip_map_[fn->name()] = state->base_global_ip;

            for (const auto& inst : state->instructions) {
                global_instructions_.push_back(inst);
            }
        }

        // Process pending anonymous / spawned lambda functions
        while (!pending_functions_.empty()) {
            auto fn = pending_functions_.back();
            pending_functions_.pop_back();

            auto state = std::make_shared<FuncState>();
            state->name = fn->name();
            state->base_global_ip = global_instructions_.size();
            current_func_ = state;

            gen_function(fn, state);

            CompiledFunctionInfo fn_info;
            fn_info.name = fn->name();
            fn_info.arity = static_cast<std::uint8_t>(fn->params().size());
            fn_info.start_ip = state->base_global_ip;
            fn_info.instructions = state->instructions;
            output.functions.push_back(fn_info);

            std::string qualified_name = output.module_name + "." + fn->name();
            output.function_table[qualified_name] = state->base_global_ip;
            output.function_table[qualified_name + "/" + std::to_string(fn_info.arity)] = state->base_global_ip;
            output.function_table[fn->name()] = state->base_global_ip;
            output.function_table[fn->name() + "/" + std::to_string(fn_info.arity)] = state->base_global_ip;
            function_ip_map_[fn->name()] = state->base_global_ip;

            for (const auto& inst : state->instructions) {
                global_instructions_.push_back(inst);
            }
        }

        // Patch global call/spawn jump addresses
        auto is_bif_name = [](const std::string& name) {
            return name == "List.tambah" || name == "List.append" ||
                   name == "List.panjang" || name == "List.length" ||
                   name == "List.kepala"  || name == "List.head" ||
                   name == "List.ekor"    || name == "List.tail" ||
                   name == "Map.baru"     || name == "Map.new" ||
                   name == "Map.taruh"    || name == "Map.put" ||
                   name == "Map.ambil"    || name == "Map.get" ||
                   name == "Map.hapus"    || name == "Map.remove" ||
                   name == "Map.kunci"    || name == "Map.keys" ||
                   name == "Map.nilai"    || name == "Map.values" ||
                   name == "Json.encode_int" || name == "Json.decode_int" ||
                   name == "Json.encode_string" || name == "Json.decode_string" ||
                   name == "Json.encode_bool" || name == "Json.decode_bool" ||
                   name == "Json.encode_list" || name == "Json.decode_list" ||
                   name == "Json.encode_map" || name == "Json.decode_map" ||
                   name == "String.panjang" || name == "String.length" ||
                   name == "String.potong" || name == "String.pisah" ||
                   name == "String.rapikan" || name == "String.karakter_ke_int" ||
                   name == "String.int_ke_karakter";
        };

        for (std::size_t idx : global_call_patches_) {
            if (idx < global_instructions_.size()) {
                auto& inst = global_instructions_[idx];
                if (!inst.args.empty()) {
                    if (auto* atom_val = inst.args[0].literal.get_if<Atom>()) {
                        if (!is_bif_name(atom_val->name)) {
                            auto it = output.function_table.find(atom_val->name);
                            if (it != output.function_table.end()) {
                                inst.args[0] = Operand{RegType::Literal, 0, Term(static_cast<std::int64_t>(it->second))};
                            }
                        }
                    }
                }
            }
        }

        output.all_instructions = std::move(global_instructions_);
        return output;
    }

private:
    struct FuncState {
        std::string name;
        std::vector<std::string> param_names;
        std::size_t base_global_ip = 0;
        std::uint16_t next_reg = 0;
        std::vector<Instruction> instructions;
        std::unordered_map<std::string, std::uint16_t> var_map;
    };

    std::uint16_t alloc_reg() {
        if (current_func_->next_reg >= 256) {
            throw CodegenError("Register overflow: function '" + current_func_->name + "' needs more than 256 registers.");
        }
        return current_func_->next_reg++;
    }

    void emit(Opcode op, std::vector<Operand> args) {
        current_func_->instructions.push_back(Instruction{op, std::move(args), 0, 0});
    }

    std::size_t current_global_ip() const {
        return current_func_->base_global_ip + current_func_->instructions.size();
    }

    void gen_function(std::shared_ptr<FunctionDeclNode> fn, std::shared_ptr<FuncState> state) {
        state->param_names = fn->params();
        for (std::size_t i = 0; i < fn->params().size(); ++i) {
            std::uint16_t reg = alloc_reg(); // Param i -> X[i]
            state->var_map[fn->params()[i]] = reg;
        }

        const auto& stmts = fn->body();
        for (std::size_t i = 0; i < stmts.size(); ++i) {
            bool is_tail = (i == stmts.size() - 1);
            std::uint16_t last_reg = gen_expr(stmts[i], is_tail);

            if (is_tail && stmts[i]->type() != AstNodeType::CallExpr) {
                emit(Opcode::OpMove, {Operand{RegType::X, last_reg, Term()}, Operand{RegType::X, 0, Term()}});
                emit(Opcode::OpReturn, {});
            }
        }

        if (stmts.empty()) {
            std::uint16_t reg = alloc_reg();
            emit(Opcode::OpMove, {Operand{RegType::Literal, 0, Term(Atom{"ok"})}, Operand{RegType::X, reg, Term()}});
            emit(Opcode::OpMove, {Operand{RegType::X, reg, Term()}, Operand{RegType::X, 0, Term()}});
            emit(Opcode::OpReturn, {});
        }
    }

    void protect_param_regs(std::size_t arity) {
        std::vector<std::pair<std::string, std::uint16_t>> to_move;
        for (const auto& [vname, vreg] : current_func_->var_map) {
            if (vreg < arity) {
                to_move.emplace_back(vname, vreg);
            }
        }
        for (const auto& [vname, vreg] : to_move) {
            std::uint16_t new_reg = alloc_reg();
            emit(Opcode::OpMove, {Operand{RegType::X, vreg, Term()}, Operand{RegType::X, new_reg, Term()}});
            current_func_->var_map[vname] = new_reg;
        }
    }

    void gen_tail_call(std::shared_ptr<CallExprNode> call) {
        std::size_t arity = call->args().size();
        protect_param_regs(arity);

        std::vector<std::uint16_t> arg_regs;
        arg_regs.reserve(arity);
        for (std::size_t i = 0; i < arity; ++i) {
            arg_regs.push_back(gen_expr(call->args()[i], false));
        }
        std::vector<std::uint16_t> temp_regs;
        temp_regs.reserve(arg_regs.size());
        for (std::size_t i = 0; i < arg_regs.size(); ++i) {
            std::uint16_t t_reg = alloc_reg();
            emit(Opcode::OpMove, {Operand{RegType::X, arg_regs[i], Term()}, Operand{RegType::X, t_reg, Term()}});
            temp_regs.push_back(t_reg);
        }
        for (std::size_t i = 0; i < temp_regs.size(); ++i) {
            emit(Opcode::OpMove, {Operand{RegType::X, temp_regs[i], Term()}, Operand{RegType::X, static_cast<std::uint16_t>(i), Term()}});
        }

        // Reset formal parameter mappings back to canonical registers (0..N-1) for loop iterations
        for (std::size_t i = 0; i < current_func_->param_names.size(); ++i) {
            current_func_->var_map[current_func_->param_names[i]] = static_cast<std::uint16_t>(i);
        }
        std::size_t global_idx = current_global_ip();
        if (current_func_->var_map.count(call->name())) {
            std::uint16_t fn_reg = current_func_->var_map[call->name()];
            emit(Opcode::OpTailCall, {Operand{RegType::X, fn_reg, Term()}});
        } else {
            emit(Opcode::OpTailCall, {Operand{RegType::Literal, 0, Term(Atom{call->name()})}});
            global_call_patches_.push_back(global_idx);
        }
    }

    void gen_pattern_bind(std::shared_ptr<AstNode> pattern, std::uint16_t src_reg) {
        if (!pattern) return;
        if (pattern->type() == AstNodeType::PatternVar) {
            auto* pvar = static_cast<PatternVarNode*>(pattern.get());
            if (pvar->name() != "_") {
                std::uint16_t reg = alloc_reg();
                emit(Opcode::OpMove, {Operand{RegType::X, src_reg, Term()}, Operand{RegType::X, reg, Term()}});
                current_func_->var_map[pvar->name()] = reg;
            }
        } else if (pattern->type() == AstNodeType::VariableExpr) {
            auto* pvar = static_cast<VariableNode*>(pattern.get());
            if (pvar->name() != "_") {
                std::uint16_t reg = alloc_reg();
                emit(Opcode::OpMove, {Operand{RegType::X, src_reg, Term()}, Operand{RegType::X, reg, Term()}});
                current_func_->var_map[pvar->name()] = reg;
            }
        } else if (pattern->type() == AstNodeType::PatternTuple || pattern->type() == AstNodeType::TupleExpr) {
            auto* ptup = static_cast<PatternTupleNode*>(pattern.get());
            for (std::size_t i = 0; i < ptup->items().size(); ++i) {
                std::uint16_t elem_reg = alloc_reg();
                emit(Opcode::OpGetTupleElem, {
                    Operand{RegType::X, src_reg, Term()},
                    Operand{RegType::Literal, 0, Term(static_cast<std::int64_t>(i))},
                    Operand{RegType::X, elem_reg, Term()}
                });
                gen_pattern_bind(ptup->items()[i], elem_reg);
            }
        } else if (pattern->type() == AstNodeType::PatternListHeadTail) {
            auto* pht = static_cast<PatternListHeadTailNode*>(pattern.get());
            std::uint16_t head_reg = alloc_reg();
            std::uint16_t tail_reg = alloc_reg();

            emit(Opcode::OpGetListElem, {
                Operand{RegType::X, src_reg, Term()},
                Operand{RegType::Literal, 0, Term(0LL)},
                Operand{RegType::X, head_reg, Term()}
            });
            emit(Opcode::OpGetListTail, {
                Operand{RegType::X, src_reg, Term()},
                Operand{RegType::X, tail_reg, Term()}
            });

            gen_pattern_bind(pht->head_ptr(), head_reg);
            gen_pattern_bind(pht->tail_ptr(), tail_reg);
        } else if (pattern->type() == AstNodeType::PatternGuard) {
            auto* pg = static_cast<PatternGuardNode*>(pattern.get());
            gen_pattern_bind(pg->pattern_ptr(), src_reg);
            gen_expr(pg->guard_ptr(), false);
        }
    }

    std::uint16_t gen_expr(std::shared_ptr<AstNode> node, bool is_tail = false) {
        if (!node) return alloc_reg();

        switch (node->type()) {
            case AstNodeType::LiteralExpr: {
                auto* lit = static_cast<LiteralNode*>(node.get());
                std::uint16_t reg = alloc_reg();
                emit(Opcode::OpMove, {Operand{RegType::Literal, 0, lit->value()}, Operand{RegType::X, reg, Term()}});
                return reg;
            }
            case AstNodeType::PatternLiteral: {
                auto* pat_lit = static_cast<PatternLiteralNode*>(node.get());
                std::uint16_t reg = alloc_reg();
                emit(Opcode::OpMove, {Operand{RegType::Literal, 0, pat_lit->value()}, Operand{RegType::X, reg, Term()}});
                return reg;
            }
            case AstNodeType::AtomExpr: {
                auto* atom_node = static_cast<AtomExprNode*>(node.get());
                std::uint16_t reg = alloc_reg();
                emit(Opcode::OpMove, {Operand{RegType::Literal, 0, Term(Atom{atom_node->name()})}, Operand{RegType::X, reg, Term()}});
                return reg;
            }
            case AstNodeType::BoolExpr: {
                auto* bool_node = static_cast<BoolExprNode*>(node.get());
                std::uint16_t reg = alloc_reg();
                emit(Opcode::OpMove, {Operand{RegType::Literal, 0, Term(Atom{bool_node->value() ? "true" : "false"})}, Operand{RegType::X, reg, Term()}});
                return reg;
            }
            case AstNodeType::NilExpr: {
                std::uint16_t reg = alloc_reg();
                emit(Opcode::OpMove, {Operand{RegType::Literal, 0, Term(Atom{"nil"})}, Operand{RegType::X, reg, Term()}});
                return reg;
            }
            case AstNodeType::VariableExpr: {
                auto* var = static_cast<VariableNode*>(node.get());
                auto it = current_func_->var_map.find(var->name());
                if (it != current_func_->var_map.end()) {
                    return it->second;
                }
                std::uint16_t reg = alloc_reg();
                current_func_->var_map[var->name()] = reg;
                emit(Opcode::OpMove, {
                    Operand{RegType::Literal, 0, Term(Atom{var->name()})},
                    Operand{RegType::X, reg, Term()}
                });
                return reg;
            }
            case AstNodeType::BinaryExpr: {
                auto* bin = static_cast<BinaryExprNode*>(node.get());
                if (bin->op() == "|>") {
                    if (bin->right_ptr()->type() == AstNodeType::CallExpr) {
                        auto* call_node = static_cast<CallExprNode*>(bin->right_ptr().get());
                        std::vector<std::shared_ptr<AstNode>> new_args;
                        new_args.reserve(call_node->args().size() + 1);
                        new_args.push_back(bin->left_ptr());
                        for (const auto& arg : call_node->args()) {
                            new_args.push_back(arg);
                        }
                        auto desugared_call = std::make_shared<CallExprNode>(call_node->name(), std::move(new_args));
                        return gen_expr(desugared_call, is_tail);
                    } else if (bin->right_ptr()->type() == AstNodeType::VariableExpr) {
                        auto* var_node = static_cast<VariableNode*>(bin->right_ptr().get());
                        std::vector<std::shared_ptr<AstNode>> new_args = {bin->left_ptr()};
                        auto desugared_call = std::make_shared<CallExprNode>(var_node->name(), std::move(new_args));
                        return gen_expr(desugared_call, is_tail);
                    } else {
                        std::uint16_t fn_val_reg = gen_expr(bin->right_ptr(), false);
                        std::uint16_t arg_val_reg = gen_expr(bin->left_ptr(), false);
                        std::uint16_t t_arg = alloc_reg();
                        emit(Opcode::OpMove, {Operand{RegType::X, arg_val_reg, Term()}, Operand{RegType::X, t_arg, Term()}});
                        emit(Opcode::OpMove, {Operand{RegType::X, t_arg, Term()}, Operand{RegType::X, 0, Term()}});
                        emit(Opcode::OpCall, {Operand{RegType::X, fn_val_reg, Term()}});
                        std::uint16_t ret_reg = alloc_reg();
                        emit(Opcode::OpMove, {Operand{RegType::X, 0, Term()}, Operand{RegType::X, ret_reg, Term()}});
                        return ret_reg;
                    }
                }

                std::uint16_t left_reg = gen_expr(bin->left_ptr(), false);
                std::uint16_t save_left_reg = alloc_reg();
                emit(Opcode::OpMove, {Operand{RegType::X, left_reg, Term()}, Operand{RegType::X, save_left_reg, Term()}});

                std::uint16_t right_reg = gen_expr(bin->right_ptr(), false);
                std::uint16_t out_reg = alloc_reg();

                Opcode op = Opcode::OpAdd;
                if (bin->op() == "+") op = Opcode::OpAdd;
                else if (bin->op() == "-") op = Opcode::OpSub;
                else if (bin->op() == "*") op = Opcode::OpMul;
                else if (bin->op() == "/") op = Opcode::OpDiv;
                else if (bin->op() == "==") op = Opcode::OpEq;
                else if (bin->op() == "!=") op = Opcode::OpNeq;
                else if (bin->op() == "<") op = Opcode::OpLt;
                else if (bin->op() == ">") op = Opcode::OpGt;
                else if (bin->op() == "<=") op = Opcode::OpLte;
                else if (bin->op() == ">=") op = Opcode::OpGte;
                else if (bin->op() == "++") op = Opcode::OpConcat;

                emit(op, {Operand{RegType::X, save_left_reg, Term()}, Operand{RegType::X, right_reg, Term()}, Operand{RegType::X, out_reg, Term()}});
                return out_reg;
            }
            case AstNodeType::UnaryExpr: {
                auto* un = static_cast<UnaryExprNode*>(node.get());
                std::uint16_t operand_reg = gen_expr(un->operand_ptr(), false);
                std::uint16_t out_reg = alloc_reg();
                if (un->op() == "-") {
                    emit(Opcode::OpSub, {Operand{RegType::Literal, 0, Term(0LL)}, Operand{RegType::X, operand_reg, Term()}, Operand{RegType::X, out_reg, Term()}});
                } else {
                    emit(Opcode::OpMove, {Operand{RegType::X, operand_reg, Term()}, Operand{RegType::X, out_reg, Term()}});
                }
                return out_reg;
            }
            case AstNodeType::AssignmentExpr: {
                auto* assign = static_cast<AssignmentExprNode*>(node.get());
                std::uint16_t val_reg = gen_expr(assign->value_ptr(), false);
                gen_pattern_bind(assign->target_ptr(), val_reg);
                return val_reg;
            }
            case AstNodeType::TupleExpr: {
                auto* tup = static_cast<TupleExprNode*>(node.get());
                std::vector<Operand> args;
                std::uint16_t out_reg = alloc_reg();
                args.push_back(Operand{RegType::X, out_reg, Term()});

                for (const auto& item : tup->items()) {
                    std::uint16_t item_reg = gen_expr(item, false);
                    args.push_back(Operand{RegType::X, item_reg, Term()});
                }
                emit(Opcode::OpMakeTuple, std::move(args));
                return out_reg;
            }
            case AstNodeType::ListExpr: {
                auto* lst = static_cast<ListExprNode*>(node.get());
                std::vector<Operand> args;
                std::uint16_t out_reg = alloc_reg();
                args.push_back(Operand{RegType::X, out_reg, Term()});

                for (const auto& item : lst->items()) {
                    std::uint16_t item_reg = gen_expr(item, false);
                    args.push_back(Operand{RegType::X, item_reg, Term()});
                }
                emit(Opcode::OpMakeList, std::move(args));
                return out_reg;
            }
            case AstNodeType::ListConsExpr: {
                auto* cons = static_cast<const ListConsExprNode*>(node.get());
                std::vector<Operand> head_args;
                std::uint16_t head_list_reg = alloc_reg();
                head_args.push_back(Operand{RegType::X, head_list_reg, Term()});

                for (const auto& item : cons->head_items()) {
                    std::uint16_t item_reg = gen_expr(item, false);
                    head_args.push_back(Operand{RegType::X, item_reg, Term()});
                }
                emit(Opcode::OpMakeList, std::move(head_args));

                std::uint16_t tail_reg = gen_expr(cons->tail(), false);
                std::uint16_t out_reg = alloc_reg();
                emit(Opcode::OpConcat, {Operand{RegType::X, head_list_reg, Term()}, Operand{RegType::X, tail_reg, Term()}, Operand{RegType::X, out_reg, Term()}});
                return out_reg;
            }
            case AstNodeType::CallExpr: {
                auto* call = static_cast<CallExprNode*>(node.get());
                if (is_tail) {
                    gen_tail_call(std::shared_ptr<CallExprNode>(node, call));
                    return 0;
                }

                std::size_t arity = call->args().size();
                protect_param_regs(arity);

                std::uint16_t fn_reg = 0;
                bool is_var_fn = current_func_->var_map.count(call->name()) > 0;
                if (is_var_fn) {
                    std::uint16_t orig_fn_reg = current_func_->var_map[call->name()];
                    fn_reg = alloc_reg();
                    emit(Opcode::OpMove, {Operand{RegType::X, orig_fn_reg, Term()}, Operand{RegType::X, fn_reg, Term()}});
                }

                std::vector<std::uint16_t> arg_regs;
                arg_regs.reserve(arity);
                for (std::size_t i = 0; i < arity; ++i) {
                    arg_regs.push_back(gen_expr(call->args()[i], false));
                }

                std::vector<std::uint16_t> temp_regs;
                temp_regs.reserve(arg_regs.size());
                for (std::size_t i = 0; i < arg_regs.size(); ++i) {
                    std::uint16_t t_reg = alloc_reg();
                    emit(Opcode::OpMove, {Operand{RegType::X, arg_regs[i], Term()}, Operand{RegType::X, t_reg, Term()}});
                    temp_regs.push_back(t_reg);
                }

                for (std::size_t i = 0; i < temp_regs.size(); ++i) {
                    emit(Opcode::OpMove, {Operand{RegType::X, temp_regs[i], Term()}, Operand{RegType::X, static_cast<std::uint16_t>(i), Term()}});
                }

                std::size_t global_idx = current_global_ip();
                if (is_var_fn) {
                    emit(Opcode::OpCall, {Operand{RegType::X, fn_reg, Term()}});
                } else {
                    emit(Opcode::OpCall, {Operand{RegType::Literal, 0, Term(Atom{call->name()})}});
                    global_call_patches_.push_back(global_idx);
                }

                std::uint16_t ret_reg = alloc_reg();
                emit(Opcode::OpMove, {Operand{RegType::X, 0, Term()}, Operand{RegType::X, ret_reg, Term()}});
                return ret_reg;
            }
            case AstNodeType::TryCatchExpr: {
                auto* tc = static_cast<TryCatchNode*>(node.get());

                std::size_t local_try_begin = current_func_->instructions.size();
                emit(Opcode::OpTryBegin, {Operand{RegType::Literal, 0, Term(0LL)}});

                std::uint16_t try_res_reg = 0;
                for (const auto& stmt : tc->try_body()) {
                    try_res_reg = gen_expr(stmt, false);
                }

                emit(Opcode::OpTryEnd, {});

                std::size_t local_jump_end = current_func_->instructions.size();
                emit(Opcode::OpJump, {Operand{RegType::Literal, 0, Term(0LL)}});

                std::size_t catch_start_ip = current_global_ip();
                current_func_->instructions[local_try_begin].args[0] =
                    Operand{RegType::Literal, 0, Term(static_cast<std::int64_t>(catch_start_ip))};

                gen_pattern_bind(tc->catch_pattern_ptr(), 0);

                std::uint16_t catch_res_reg = 0;
                for (const auto& stmt : tc->catch_body()) {
                    catch_res_reg = gen_expr(stmt, false);
                }

                std::size_t end_ip = current_global_ip();
                current_func_->instructions[local_jump_end].args[0] =
                    Operand{RegType::Literal, 0, Term(static_cast<std::int64_t>(end_ip))};

                std::uint16_t out_reg = alloc_reg();
                emit(Opcode::OpMove, {Operand{RegType::X, catch_res_reg != 0 ? catch_res_reg : try_res_reg, Term()}, Operand{RegType::X, out_reg, Term()}});
                return out_reg;
            }
            case AstNodeType::TryOperatorExpr: {
                auto* try_op = static_cast<TryOperatorNode*>(node.get());
                std::uint16_t val_reg = gen_expr(try_op->expr_ptr(), false);

                std::uint16_t unwrap_reg = alloc_reg();
                emit(Opcode::OpGetTupleElem, {
                    Operand{RegType::X, val_reg, Term()},
                    Operand{RegType::Literal, 0, Term(1LL)},
                    Operand{RegType::X, unwrap_reg, Term()}
                });

                return unwrap_reg;
            }
            case AstNodeType::CaseExpr: {
                auto* case_node = static_cast<CaseExprNode*>(node.get());
                std::uint16_t orig_subj = gen_expr(case_node->subject_ptr(), false);
                std::uint16_t subj_reg = alloc_reg();
                emit(Opcode::OpMove, {Operand{RegType::X, orig_subj, Term()}, Operand{RegType::X, subj_reg, Term()}});

                std::uint16_t result_reg = alloc_reg();
                std::vector<std::size_t> end_jumps;

                for (const auto& clause : case_node->clauses()) {
                    std::uint16_t match_reg = alloc_reg();
                    if (clause.pattern->type() == AstNodeType::PatternWildcard) {
                        emit(Opcode::OpMove, {Operand{RegType::Literal, 0, Term(1LL)}, Operand{RegType::X, match_reg, Term()}});
                    } else if (clause.pattern->type() == AstNodeType::PatternLiteral) {
                        auto* pat_lit = static_cast<PatternLiteralNode*>(clause.pattern.get());
                        std::uint16_t pat_reg = alloc_reg();
                        emit(Opcode::OpMove, {Operand{RegType::Literal, 0, pat_lit->value()}, Operand{RegType::X, pat_reg, Term()}});
                        emit(Opcode::OpEq, {Operand{RegType::X, subj_reg, Term()}, Operand{RegType::X, pat_reg, Term()}, Operand{RegType::X, match_reg, Term()}});
                    } else if (clause.pattern->type() == AstNodeType::ListExpr) {
                        std::uint16_t len_reg = alloc_reg();
                        emit(Opcode::OpGetListLen, {
                            Operand{RegType::X, subj_reg, Term()},
                            Operand{RegType::X, len_reg, Term()}
                        });
                        std::uint16_t zero_reg = alloc_reg();
                        emit(Opcode::OpMove, {Operand{RegType::Literal, 0, Term(0LL)}, Operand{RegType::X, zero_reg, Term()}});
                        emit(Opcode::OpEq, {Operand{RegType::X, len_reg, Term()}, Operand{RegType::X, zero_reg, Term()}, Operand{RegType::X, match_reg, Term()}});
                    } else if (clause.pattern->type() == AstNodeType::PatternListHeadTail) {
                        std::uint16_t len_reg = alloc_reg();
                        emit(Opcode::OpGetListLen, {
                            Operand{RegType::X, subj_reg, Term()},
                            Operand{RegType::X, len_reg, Term()}
                        });
                        std::uint16_t zero_reg = alloc_reg();
                        emit(Opcode::OpMove, {Operand{RegType::Literal, 0, Term(0LL)}, Operand{RegType::X, zero_reg, Term()}});
                        emit(Opcode::OpGt, {Operand{RegType::X, len_reg, Term()}, Operand{RegType::X, zero_reg, Term()}, Operand{RegType::X, match_reg, Term()}});
                    } else if (clause.pattern->type() == AstNodeType::PatternTuple ||
                               clause.pattern->type() == AstNodeType::TupleExpr) {
                        auto* ptup = static_cast<PatternTupleNode*>(clause.pattern.get());
                        if (!ptup->items().empty() && (ptup->items()[0]->type() == AstNodeType::AtomExpr || ptup->items()[0]->type() == AstNodeType::PatternLiteral)) {
                            std::uint16_t elem0_reg = alloc_reg();
                            emit(Opcode::OpGetTupleElem, {
                                Operand{RegType::X, subj_reg, Term()},
                                Operand{RegType::Literal, 0, Term(0LL)},
                                Operand{RegType::X, elem0_reg, Term()}
                            });
                            std::uint16_t tag_reg = gen_expr(ptup->items()[0], false);
                            emit(Opcode::OpEq, {Operand{RegType::X, elem0_reg, Term()}, Operand{RegType::X, tag_reg, Term()}, Operand{RegType::X, match_reg, Term()}});
                        } else {
                            emit(Opcode::OpMove, {Operand{RegType::Literal, 0, Term(1LL)}, Operand{RegType::X, match_reg, Term()}});
                        }
                    } else if (clause.pattern->type() == AstNodeType::LiteralExpr ||
                               clause.pattern->type() == AstNodeType::AtomExpr ||
                               clause.pattern->type() == AstNodeType::BoolExpr) {
                        std::uint16_t pat_reg = gen_expr(clause.pattern, false);
                        emit(Opcode::OpEq, {Operand{RegType::X, subj_reg, Term()}, Operand{RegType::X, pat_reg, Term()}, Operand{RegType::X, match_reg, Term()}});
                    } else {
                        emit(Opcode::OpMove, {Operand{RegType::Literal, 0, Term(1LL)}, Operand{RegType::X, match_reg, Term()}});
                    }

                    std::size_t jump_if_false_ip = current_func_->instructions.size();
                    emit(Opcode::OpJumpIfFalse, {Operand{RegType::X, match_reg, Term()}, Operand{RegType::Literal, 0, Term(0LL)}});

                    gen_pattern_bind(clause.pattern, subj_reg);

                    std::uint16_t clause_res = 0;
                    for (std::size_t j = 0; j < clause.body.size(); ++j) {
                        bool stmt_is_tail = is_tail && (j == clause.body.size() - 1);
                        clause_res = gen_expr(clause.body[j], stmt_is_tail);
                    }

                    emit(Opcode::OpMove, {Operand{RegType::X, clause_res, Term()}, Operand{RegType::X, result_reg, Term()}});

                    end_jumps.push_back(current_func_->instructions.size());
                    emit(Opcode::OpJump, {Operand{RegType::Literal, 0, Term(0LL)}});

                    std::size_t next_clause_ip = current_global_ip();
                    current_func_->instructions[jump_if_false_ip].args[1] =
                        Operand{RegType::Literal, 0, Term(static_cast<std::int64_t>(next_clause_ip))};
                }

                std::size_t end_ip = current_global_ip();
                for (auto jump_ip : end_jumps) {
                    current_func_->instructions[jump_ip].args[0] =
                        Operand{RegType::Literal, 0, Term(static_cast<std::int64_t>(end_ip))};
                }

                return result_reg;
            }
            case AstNodeType::SendExpr: {
                auto* send_node = static_cast<SendExprNode*>(node.get());
                std::uint16_t target_reg = gen_expr(send_node->target_ptr(), false);
                std::uint16_t msg_reg = gen_expr(send_node->message_ptr(), false);
                emit(Opcode::OpSendMsg, {Operand{RegType::X, target_reg, Term()}, Operand{RegType::X, msg_reg, Term()}});
                return msg_reg;
            }
            case AstNodeType::SpawnExpr: {
                auto* spawn_node = static_cast<SpawnExprNode*>(node.get());
                std::uint16_t out_reg = alloc_reg();
                std::string target_fn_name;

                if (spawn_node->fun().type() == AstNodeType::AnonFunExpr) {
                    auto* anon = static_cast<const AnonFunExprNode*>(&spawn_node->fun());
                    target_fn_name = module_name_ + ".__anon_" + std::to_string(anon_counter_++);
                    auto fn_decl = std::make_shared<FunctionDeclNode>(target_fn_name, anon->params(), anon->body());
                    pending_functions_.push_back(fn_decl);
                } else if (spawn_node->fun().type() == AstNodeType::CallExpr) {
                    auto* call = static_cast<const CallExprNode*>(&spawn_node->fun());
                    target_fn_name = call->name();
                } else if (spawn_node->fun().type() == AstNodeType::VariableExpr) {
                    auto* var = static_cast<const VariableNode*>(&spawn_node->fun());
                    target_fn_name = var->name();
                } else {
                    target_fn_name = "server";
                }

                std::size_t global_idx = current_global_ip();
                emit(Opcode::OpSpawn, {Operand{RegType::Literal, 0, Term(Atom{target_fn_name})}, Operand{RegType::X, out_reg, Term()}});
                global_call_patches_.push_back(global_idx);
                return out_reg;
            }
            case AstNodeType::AnonFunExpr: {
                auto* anon = static_cast<const AnonFunExprNode*>(node.get());
                std::string target_fn_name = module_name_ + ".__anon_" + std::to_string(anon_counter_++);
                auto fn_decl = std::make_shared<FunctionDeclNode>(target_fn_name, anon->params(), anon->body());
                pending_functions_.push_back(fn_decl);

                std::uint16_t out_reg = alloc_reg();
                emit(Opcode::OpMove, {Operand{RegType::Literal, 0, Term(Atom{target_fn_name})}, Operand{RegType::X, out_reg, Term()}});
                return out_reg;
            }
            case AstNodeType::SelfExpr: {
                std::uint16_t out_reg = alloc_reg();
                emit(Opcode::OpSelf, {Operand{RegType::X, out_reg, Term()}});
                return out_reg;
            }
            case AstNodeType::ReceiveExpr: {
                auto* rec_node = static_cast<ReceiveExprNode*>(node.get());
                std::uint16_t out_reg = alloc_reg();
                std::uint16_t msg_reg = alloc_reg();
                emit(Opcode::OpReceive, {Operand{RegType::X, msg_reg, Term()}});

                if (!rec_node->clauses().empty()) {
                    const auto& clause = rec_node->clauses()[0];
                    gen_pattern_bind(clause.pattern, msg_reg);
                    std::uint16_t body_reg = 0;
                    for (std::size_t j = 0; j < clause.body.size(); ++j) {
                        bool stmt_is_tail = is_tail && (j == clause.body.size() - 1);
                        body_reg = gen_expr(clause.body[j], stmt_is_tail);
                    }
                    if (body_reg != 0) {
                        emit(Opcode::OpMove, {Operand{RegType::X, body_reg, Term()}, Operand{RegType::X, out_reg, Term()}});
                    }
                }
                return out_reg;
            }
            case AstNodeType::IfExpr: {
                auto* if_node = static_cast<IfExprNode*>(node.get());
                std::uint16_t out_reg = alloc_reg();

                if (!if_node->clauses().empty()) {
                    const auto& clause = if_node->clauses()[0];
                    std::uint16_t cond_reg = gen_expr(clause.condition, false);

                    std::size_t local_jump_if_false = current_func_->instructions.size();
                    emit(Opcode::OpJumpIfFalse, {Operand{RegType::X, cond_reg, Term()}, Operand{RegType::Literal, 0, Term(0LL)}});

                    std::uint16_t then_reg = 0;
                    for (std::size_t j = 0; j < clause.body.size(); ++j) {
                        bool stmt_is_tail = is_tail && (j == clause.body.size() - 1);
                        then_reg = gen_expr(clause.body[j], stmt_is_tail);
                    }
                    emit(Opcode::OpMove, {Operand{RegType::X, then_reg, Term()}, Operand{RegType::X, out_reg, Term()}});

                    std::size_t local_jump_then_end = current_func_->instructions.size();
                    emit(Opcode::OpJump, {Operand{RegType::Literal, 0, Term(0LL)}});

                    std::size_t else_start_ip = current_global_ip();
                    current_func_->instructions[local_jump_if_false].args[1] =
                        Operand{RegType::Literal, 0, Term(static_cast<std::int64_t>(else_start_ip))};

                    std::uint16_t else_reg = 0;
                    for (std::size_t j = 0; j < if_node->else_body().size(); ++j) {
                        bool stmt_is_tail = is_tail && (j == if_node->else_body().size() - 1);
                        else_reg = gen_expr(if_node->else_body()[j], stmt_is_tail);
                    }
                    emit(Opcode::OpMove, {Operand{RegType::X, else_reg, Term()}, Operand{RegType::X, out_reg, Term()}});

                    std::size_t end_ip = current_global_ip();
                    current_func_->instructions[local_jump_then_end].args[0] =
                        Operand{RegType::Literal, 0, Term(static_cast<std::int64_t>(end_ip))};
                }

                return out_reg;
            }
            case AstNodeType::CetakExpr: {
                auto* pr = static_cast<CetakExprNode*>(node.get());
                std::uint16_t arg_reg = gen_expr(pr->value_ptr(), false);
                emit(Opcode::OpPrint, {Operand{RegType::X, arg_reg, Term()}});
                return arg_reg;
            }
            default:
                return alloc_reg();
        }
    }

    std::vector<Instruction> global_instructions_;
    std::shared_ptr<FuncState> current_func_;
    std::vector<std::shared_ptr<FunctionDeclNode>> pending_functions_;
    std::vector<std::size_t> global_call_patches_;
    std::unordered_map<std::string, std::size_t> function_ip_map_;
    std::size_t anon_counter_ = 1;
    std::string module_name_ = "Main";
};

} // namespace garuda::compiler
