#pragma once
// Garuda Scripting & Embedding Engine (`script_engine.hpp`)
#include "../../compiler/lexer.hpp"
#include "../../compiler/parser.hpp"
#include "../../compiler/codegen.hpp"
#include "../core/runtime.hpp"
#include "../gvm/interpreter.hpp"
#include "../io/filesystem.hpp"

#include <algorithm>
#include <array>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace garuda::beam {

class ScriptEngine {
public:
    ScriptEngine() = default;

    std::optional<Term> eval(const std::string& source) {
        if (source.empty()) return std::nullopt;

        try {
            std::string code = source;

            // If source doesn't contain module/function keywords, wrap as 'fungsi utama() -> <expression>\nrampung'
            if (code.find("fungsi ") == std::string::npos && code.find("fun ") == std::string::npos) {
                code = "modul ScriptEval\nfungsi utama() -> " + code + "\nrampung";
            } else if (code.find("modul ") == std::string::npos) {
                code = "modul ScriptEval\n" + code;
            }

            compiler::Lexer lexer(code);
            auto tokens = lexer.tokenize();
            if (tokens.empty()) return std::nullopt;

            compiler::Parser parser(tokens);
            auto ast = parser.parse_program();
            if (!ast) return std::nullopt;

            compiler::CodeGenerator cg;
            compiler::CodeGeneratorOutput cg_out = cg.generate(ast);

            std::size_t start_ip = 0;
            auto it = cg_out.function_table.find("utama");
            if (it != cg_out.function_table.end()) {
                start_ip = it->second;
            }

            return execute_instructions(cg_out, start_ip);
        } catch (...) {
            return std::nullopt;
        }
    }

    std::optional<Term> eval_file(const std::string& path) {
        if (!Fs::exists(path)) return std::nullopt;
        std::string src = Fs::read_file(path);
        if (src.empty()) return std::nullopt;
        return eval(src);
    }

private:
    std::optional<Term> execute_instructions(const compiler::CodeGeneratorOutput& cg_out, std::size_t start_ip) {
        if (cg_out.all_instructions.empty() || start_ip >= cg_out.all_instructions.size()) return std::nullopt;

        std::array<Term, 256> x_regs;
        std::vector<std::size_t> call_stack;
        std::vector<std::array<Term, 256>> frame_stack;
        std::size_t ip = start_ip;
        Term last_result;

        auto handle_builtin = [&](const std::string& fn_name) -> bool {
            auto is_fn = [](const std::string& name, const std::string& base) {
                return name == base || name == ("String." + base) ||
                       name.rfind(base + "/", 0) == 0 ||
                       name.rfind("String." + base + "/", 0) == 0;
            };

            auto clean_str = [](const Term& t) {
                std::string s;
                if (auto* str = t.get_if<std::string>()) s = *str;
                else if (auto* a = t.get_if<Atom>()) s = a->name;
                else if (auto* i = t.get_if<std::int64_t>()) s = std::to_string(*i);
                if (s.length() >= 2 && s.front() == '"' && s.back() == '"') s = s.substr(1, s.length() - 2);
                return s;
            };

            if (is_fn(fn_name, "panjang") || fn_name == "string_length") {
                std::string s = clean_str(x_regs[0]);
                Term res(static_cast<std::int64_t>(s.length()));
                x_regs[0] = res;
                last_result = res;
                return true;
            }
            if (is_fn(fn_name, "besar")) {
                std::string s = clean_str(x_regs[0]);
                std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::toupper(c); });
                Term res(s);
                x_regs[0] = res;
                last_result = res;
                return true;
            }
            if (is_fn(fn_name, "kecil")) {
                std::string s = clean_str(x_regs[0]);
                std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
                Term res(s);
                x_regs[0] = res;
                last_result = res;
                return true;
            }
            if (is_fn(fn_name, "potong")) {
                std::string s = clean_str(x_regs[0]);
                int64_t start = x_regs[1].get_if<std::int64_t>() ? *x_regs[1].get_if<std::int64_t>() : 0;
                int64_t len = x_regs[2].get_if<std::int64_t>() ? *x_regs[2].get_if<std::int64_t>() : static_cast<int64_t>(s.length());
                if (start >= 0 && static_cast<std::size_t>(start) < s.length()) {
                    s = s.substr(static_cast<std::size_t>(start), static_cast<std::size_t>(len));
                } else {
                    s = "";
                }
                Term res(s);
                x_regs[0] = res;
                last_result = res;
                return true;
            }
            if (is_fn(fn_name, "gabung")) {
                std::string s1 = clean_str(x_regs[0]);
                std::string s2 = clean_str(x_regs[1]);
                Term res(s1 + s2);
                x_regs[0] = res;
                last_result = res;
                return true;
            }
            return false;
        };

        while (ip < cg_out.all_instructions.size()) {
            const auto& inst = cg_out.all_instructions[ip];

            switch (inst.op) {
                case Opcode::OpMove: {
                    if (inst.args.size() >= 2) {
                        const auto& src = inst.args[0];
                        const auto& dst = inst.args[1];
                        Term val = (src.type == RegType::Literal) ? src.literal : x_regs[src.index % 256];
                        x_regs[dst.index % 256] = val;
                        last_result = val;
                    }
                    break;
                }
                case Opcode::OpAdd: {
                    if (inst.args.size() >= 3) {
                        const auto& a = inst.args[0];
                        const auto& b = inst.args[1];
                        const auto& dst = inst.args[2];
                        Term val_a = (a.type == RegType::Literal) ? a.literal : x_regs[a.index % 256];
                        Term val_b = (b.type == RegType::Literal) ? b.literal : x_regs[b.index % 256];

                        if (val_a.get_if<std::int64_t>() && val_b.get_if<std::int64_t>()) {
                            Term res(*val_a.get_if<std::int64_t>() + *val_b.get_if<std::int64_t>());
                            x_regs[dst.index % 256] = res;
                            last_result = res;
                        } else if (val_a.get_if<double>() || val_b.get_if<double>()) {
                            double da = val_a.get_if<double>() ? *val_a.get_if<double>() : static_cast<double>(*val_a.get_if<std::int64_t>());
                            double db = val_b.get_if<double>() ? *val_b.get_if<double>() : static_cast<double>(*val_b.get_if<std::int64_t>());
                            Term res(da + db);
                            x_regs[dst.index % 256] = res;
                            last_result = res;
                        }
                    }
                    break;
                }
                case Opcode::OpSub: {
                    if (inst.args.size() >= 3) {
                        Term val_a = (inst.args[0].type == RegType::Literal) ? inst.args[0].literal : x_regs[inst.args[0].index % 256];
                        Term val_b = (inst.args[1].type == RegType::Literal) ? inst.args[1].literal : x_regs[inst.args[1].index % 256];
                        if (val_a.get_if<std::int64_t>() && val_b.get_if<std::int64_t>()) {
                            Term res(*val_a.get_if<std::int64_t>() - *val_b.get_if<std::int64_t>());
                            x_regs[inst.args[2].index % 256] = res;
                            last_result = res;
                        }
                    }
                    break;
                }
                case Opcode::OpMul: {
                    if (inst.args.size() >= 3) {
                        Term val_a = (inst.args[0].type == RegType::Literal) ? inst.args[0].literal : x_regs[inst.args[0].index % 256];
                        Term val_b = (inst.args[1].type == RegType::Literal) ? inst.args[1].literal : x_regs[inst.args[1].index % 256];
                        if (val_a.get_if<std::int64_t>() && val_b.get_if<std::int64_t>()) {
                            Term res(*val_a.get_if<std::int64_t>() * *val_b.get_if<std::int64_t>());
                            x_regs[inst.args[2].index % 256] = res;
                            last_result = res;
                        }
                    }
                    break;
                }
                case Opcode::OpDiv: {
                    if (inst.args.size() >= 3) {
                        Term val_a = (inst.args[0].type == RegType::Literal) ? inst.args[0].literal : x_regs[inst.args[0].index % 256];
                        Term val_b = (inst.args[1].type == RegType::Literal) ? inst.args[1].literal : x_regs[inst.args[1].index % 256];
                        if (val_a.get_if<std::int64_t>() && val_b.get_if<std::int64_t>() && *val_b.get_if<std::int64_t>() != 0) {
                            Term res(*val_a.get_if<std::int64_t>() / *val_b.get_if<std::int64_t>());
                            x_regs[inst.args[2].index % 256] = res;
                            last_result = res;
                        }
                    }
                    break;
                }
                case Opcode::OpConcat: {
                    if (inst.args.size() >= 3) {
                        Term val_a = (inst.args[0].type == RegType::Literal) ? inst.args[0].literal : x_regs[inst.args[0].index % 256];
                        Term val_b = (inst.args[1].type == RegType::Literal) ? inst.args[1].literal : x_regs[inst.args[1].index % 256];
                        auto clean_str = [](const Term& t) {
                            std::string s;
                            if (auto* str = t.get_if<std::string>()) s = *str;
                            else if (auto* a = t.get_if<Atom>()) s = a->name;
                            else if (auto* i = t.get_if<std::int64_t>()) s = std::to_string(*i);
                            if (s.length() >= 2 && s.front() == '"' && s.back() == '"') s = s.substr(1, s.length() - 2);
                            return s;
                        };
                        std::string sa = clean_str(val_a);
                        std::string sb = clean_str(val_b);
                        Term res(sa + sb);
                        x_regs[inst.args[2].index % 256] = res;
                        last_result = res;
                    }
                    break;
                }
                case Opcode::OpCall: {
                    if (!inst.args.empty()) {
                        std::string fn_name;
                        if (auto* a = inst.args[0].literal.get_if<Atom>()) fn_name = a->name;
                        else if (auto* s = inst.args[0].literal.get_if<std::string>()) fn_name = *s;

                        if (handle_builtin(fn_name)) break;

                        std::size_t target_ip = 0;
                        bool found = false;

                        if (auto* i = inst.args[0].literal.get_if<std::int64_t>()) {
                            target_ip = static_cast<std::size_t>(*i);
                            found = true;
                        } else {
                            auto fit = cg_out.function_table.find(fn_name);
                            if (fit == cg_out.function_table.end()) {
                                for (const auto& [k, v] : cg_out.function_table) {
                                    if (k == fn_name || k.rfind(fn_name + "/", 0) == 0) {
                                        fit = cg_out.function_table.find(k);
                                        break;
                                    }
                                }
                            }

                            if (fit != cg_out.function_table.end()) {
                                target_ip = fit->second;
                                found = true;
                            }
                        }

                        if (found) {
                            call_stack.push_back(ip + 1);
                            frame_stack.push_back(x_regs);
                            ip = target_ip - 1;
                        }
                    }
                    break;
                }
                case Opcode::OpTailCall: {
                    if (!inst.args.empty()) {
                        std::string fn_name;
                        if (auto* a = inst.args[0].literal.get_if<Atom>()) fn_name = a->name;
                        else if (auto* s = inst.args[0].literal.get_if<std::string>()) fn_name = *s;

                        if (handle_builtin(fn_name)) break;

                        std::size_t target_ip = 0;
                        bool found = false;

                        if (auto* i = inst.args[0].literal.get_if<std::int64_t>()) {
                            target_ip = static_cast<std::size_t>(*i);
                            found = true;
                        } else {
                            auto fit = cg_out.function_table.find(fn_name);
                            if (fit == cg_out.function_table.end()) {
                                for (const auto& [k, v] : cg_out.function_table) {
                                    if (k == fn_name || k.rfind(fn_name + "/", 0) == 0) {
                                        fit = cg_out.function_table.find(k);
                                        break;
                                    }
                                }
                            }

                            if (fit != cg_out.function_table.end()) {
                                target_ip = fit->second;
                                found = true;
                            }
                        }

                        if (found) {
                            ip = target_ip - 1;
                        }
                    }
                    break;
                }
                case Opcode::OpReturn: {
                    Term ret_val = !last_result.is<std::monostate>() ? last_result : x_regs[0];
                    if (!call_stack.empty()) {
                        ip = call_stack.back() - 1;
                        call_stack.pop_back();
                        if (!frame_stack.empty()) {
                            x_regs = frame_stack.back();
                            frame_stack.pop_back();
                        }
                        x_regs[0] = ret_val;
                        last_result = ret_val;
                    } else {
                        return ret_val;
                    }
                    break;
                }
                case Opcode::OpHalt: {
                    Term ret_val = !x_regs[0].is<std::monostate>() ? x_regs[0] : last_result;
                    return ret_val;
                }
                default:
                    break;
            }

            ip++;
        }

        if (!x_regs[0].is<std::monostate>()) return x_regs[0];
        return last_result;
    }
};

} // namespace garuda::beam
