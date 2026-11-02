#pragma once
// Garuda Scripting & Embedding Engine (`script_engine.hpp`)
#include "../../compiler/lexer.hpp"
#include "../../compiler/parser.hpp"
#include "../../compiler/codegen.hpp"
#include "../core/runtime.hpp"
#include "../gvm/interpreter.hpp"
#include "../io/filesystem.hpp"
#include "../stdlib/string.hpp"

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <httplib.h>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <vector>

#include <set>

namespace garuda::beam {

static std::shared_ptr<httplib::Server> g_http_server;
static std::thread g_http_server_thread;
static std::atomic<bool> g_http_server_running{false};

class ScriptEngine {
public:
    ScriptEngine() = default;

    std::optional<compiler::CodeGeneratorOutput> compile_module(const std::string& path, std::set<std::string>& imported_paths) {
        try {
            std::string resolved_path;
            std::string src = "";
            if (Fs::exists(path)) {
                resolved_path = path;
                src = Fs::read_file(path);
            } else if (Fs::exists("../" + path)) {
                resolved_path = "../" + path;
                src = Fs::read_file("../" + path);
            } else if (path.rfind("std/", 0) != 0 && Fs::exists("std/" + path)) {
                resolved_path = "std/" + path;
                src = Fs::read_file("std/" + path);
            } else if (path.rfind("std/", 0) != 0 && Fs::exists("../std/" + path)) {
                resolved_path = "../std/" + path;
                src = Fs::read_file("../std/" + path);
            } else {
                return std::nullopt;
            }

            if (imported_paths.count(resolved_path) > 0) {
                return compiler::CodeGeneratorOutput{};
            }
            imported_paths.insert(resolved_path);

            if (src.empty()) return compiler::CodeGeneratorOutput{};

            compiler::Lexer lexer(src);
            auto tokens = lexer.tokenize();
            if (tokens.empty()) return compiler::CodeGeneratorOutput{};

            compiler::Parser parser(tokens);
            auto ast = parser.parse_program();
            if (!ast) return std::nullopt;

            compiler::CodeGenerator cg;
            compiler::CodeGeneratorOutput main_out = cg.generate(ast);

            compiler::CodeGeneratorOutput combined_out;

            for (const auto& imp : ast->imports()) {
                auto imp_out_opt = compile_module(imp->path(), imported_paths);
                if (imp_out_opt) {
                    merge_output(combined_out, *imp_out_opt);
                }
            }

            merge_output(combined_out, main_out);
            return combined_out;
        } catch (...) {
            return std::nullopt;
        }
    }

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
            compiler::CodeGeneratorOutput main_out = cg.generate(ast);

            compiler::CodeGeneratorOutput combined_out;
            std::set<std::string> imported_paths;

            for (const auto& imp : ast->imports()) {
                auto imp_out_opt = compile_module(imp->path(), imported_paths);
                if (!imp_out_opt.has_value()) {
                    return std::nullopt; // File not found / invalid import
                }
                merge_output(combined_out, *imp_out_opt);
            }

            merge_output(combined_out, main_out);

#pragma warning(push)
#pragma warning(disable: 4996)
            if (std::getenv("GARUDA_DUMP_BYTECODE") != nullptr) {
#pragma warning(pop)
                std::cerr << "=== DUMP function_table ===\n";
                for (const auto& [k, v] : combined_out.function_table) {
                    std::cerr << "  " << k << " -> " << v << "\n";
                }
                std::cerr << "=== DUMP all_instructions (size=" << combined_out.all_instructions.size() << ") ===\n";
                for (std::size_t i = 0; i < combined_out.all_instructions.size(); ++i) {
                    const auto& inst = combined_out.all_instructions[i];
                    std::cerr << "  " << i << ": op=" << static_cast<int>(inst.op);
                    for (std::size_t a = 0; a < inst.args.size(); ++a) {
                        std::cerr << " arg" << a << "={type=" << static_cast<int>(inst.args[a].type)
                                  << ",idx=" << inst.args[a].index
                                  << ",lit=" << to_string(inst.args[a].literal) << "}";
                    }
                    std::cerr << "\n";
                }
                std::cerr << "=== END DUMP ===\n";
            }

            std::size_t start_ip = 0;
            auto it = combined_out.function_table.find("utama");
            if (it == combined_out.function_table.end()) {
                for (const auto& [k, v] : combined_out.function_table) {
                    if (k == "utama" || k.find("utama") != std::string::npos) {
                        it = combined_out.function_table.find(k);
                        break;
                    }
                }
            }

            if (it != combined_out.function_table.end()) {
                start_ip = it->second;
            }

            return execute_instructions(combined_out, start_ip);
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
    void merge_output(compiler::CodeGeneratorOutput& target, const compiler::CodeGeneratorOutput& src) {
        std::size_t offset = target.all_instructions.size();

        for (const auto& [fn_name, fn_ip] : src.function_table) {
            target.function_table[fn_name] = offset + fn_ip;
        }

        for (auto inst : src.all_instructions) {
            if (inst.op == compiler::Opcode::OpJump || inst.op == compiler::Opcode::OpJumpIfFalse) {
                if (!inst.args.empty() && inst.args.back().type == compiler::RegType::Literal) {
                    if (auto* ip_val = inst.args.back().literal.get_if<std::int64_t>()) {
                        inst.args.back().literal = Term(static_cast<std::int64_t>(offset + *ip_val));
                    }
                }
            }
            if (inst.op == compiler::Opcode::OpCall ||
                inst.op == compiler::Opcode::OpTailCall ||
                inst.op == compiler::Opcode::OpSpawn) {
                if (!inst.args.empty() && inst.args[0].type == compiler::RegType::Literal) {
                    if (auto* ip_val = inst.args[0].literal.get_if<std::int64_t>()) {
                        inst.args[0].literal = Term(static_cast<std::int64_t>(offset + *ip_val));
                    }
                }
            }
            target.all_instructions.push_back(inst);
        }
    }

private:
    std::optional<Term> execute_instructions(const compiler::CodeGeneratorOutput& cg_out, std::size_t start_ip) {
        if (cg_out.all_instructions.empty() || start_ip >= cg_out.all_instructions.size()) return std::nullopt;

        std::array<Term, 256> x_regs;
        std::vector<std::size_t> call_stack;
        std::vector<std::array<Term, 256>> frame_stack;
        std::size_t ip = start_ip;
        Term last_result;

#pragma warning(push)
#pragma warning(disable: 4996)
        bool is_tracing = (std::getenv("GARUDA_TRACE") != nullptr);
#pragma warning(pop)

        auto handle_builtin = [&](const std::string& fn_name) -> bool {
            auto is_fn = [](const std::string& name, const std::string& base) {
                return name == base || name == ("String." + base) ||
                       name.rfind(base + "/", 0) == 0 ||
                       name.rfind("String." + base + "/", 0) == 0;
            };

            auto clean_str = [](const Term& t) {
                if (auto* str = t.get_if<std::string>()) return *str;
                if (auto* a = t.get_if<Atom>()) return a->name;
                if (auto* i = t.get_if<std::int64_t>()) return std::to_string(*i);
                return garuda::beam::to_string(t);
            };

            auto is_mod_fn = [](const std::string& name, const std::string& mod, const std::string& base) {
                return name == (mod + "." + base) ||
                       name.rfind(mod + "." + base + "/", 0) == 0;
            };

            auto get_seq_size = [](const Term& t) -> int64_t {
                if (auto* lst = t.get_if<List>()) {
                    return lst->items ? static_cast<int64_t>(lst->items->size()) : 0;
                }
                if (auto* tup = t.get_if<Tuple>()) {
                    return tup->items ? static_cast<int64_t>(tup->items->size()) : 0;
                }
                return 0;
            };

            auto get_seq_elem = [](const Term& t, size_t idx) -> Term {
                if (auto* lst = t.get_if<List>()) {
                    if (lst->items && idx < lst->items->size()) return (*lst->items)[idx];
                }
                if (auto* tup = t.get_if<Tuple>()) {
                    if (tup->items && idx < tup->items->size()) return (*tup->items)[idx];
                }
                return Term();
            };

            // List Built-ins
            if (is_mod_fn(fn_name, "List", "panjang") || is_mod_fn(fn_name, "List", "length")) {
                if (fn_name.rfind("List.", 0) == 0 || x_regs[0].is<List>() || x_regs[0].is<Tuple>()) {
                    int64_t len = get_seq_size(x_regs[0]);
                    Term res(len);
                    x_regs[0] = res;
                    last_result = res;
                    return true;
                }
            }
            if (is_mod_fn(fn_name, "List", "kepala") || is_mod_fn(fn_name, "List", "head")) {
                Term res = get_seq_elem(x_regs[0], 0);
                x_regs[0] = res;
                last_result = res;
                return true;
            }
            if (is_mod_fn(fn_name, "List", "ekor") || is_mod_fn(fn_name, "List", "tail")) {
                std::vector<Term> tail_items;
                int64_t sz = get_seq_size(x_regs[0]);
                for (int64_t i = 1; i < sz; ++i) {
                    tail_items.push_back(get_seq_elem(x_regs[0], static_cast<size_t>(i)));
                }
                Term res(List{std::make_shared<const std::vector<Term>>(std::move(tail_items))});
                x_regs[0] = res;
                last_result = res;
                return true;
            }
            if (is_mod_fn(fn_name, "List", "tambah") || is_mod_fn(fn_name, "List", "append")) {
                std::vector<Term> new_items;
                int64_t sz = get_seq_size(x_regs[0]);
                for (int64_t i = 0; i < sz; ++i) {
                    new_items.push_back(get_seq_elem(x_regs[0], static_cast<size_t>(i)));
                }
                new_items.push_back(x_regs[1]);
                Term res(List{std::make_shared<const std::vector<Term>>(std::move(new_items))});
                x_regs[0] = res;
                last_result = res;
                return true;
            }

            // Http Built-ins
            if (is_mod_fn(fn_name, "Http", "ambil") || is_mod_fn(fn_name, "Http", "get")) {
                std::string url = clean_str(x_regs[0]);
                std::size_t scheme_pos = url.find("://");
                std::string host_port = (scheme_pos != std::string::npos) ? url.substr(0, url.find('/', scheme_pos + 3)) : url;
                std::string path = (scheme_pos != std::string::npos && url.find('/', scheme_pos + 3) != std::string::npos) ? url.substr(url.find('/', scheme_pos + 3)) : "/";
                httplib::Client cli(host_port);
                cli.set_connection_timeout(5, 0);
                cli.set_read_timeout(5, 0);
                auto res = cli.Get(path.c_str());
                if (res && res->status == 200) {
                    Term result(res->body);
                    x_regs[0] = result;
                    last_result = result;
                    return true;
                }
                Term err(Atom{"error_http"});
                x_regs[0] = err;
                last_result = err;
                return true;
            }
            if (is_mod_fn(fn_name, "Http", "kirim_json") || is_mod_fn(fn_name, "Http", "post_json")) {
                std::string url = clean_str(x_regs[0]);
                std::string data = clean_str(x_regs[1]);
                std::size_t scheme_pos = url.find("://");
                std::string host_port = (scheme_pos != std::string::npos) ? url.substr(0, url.find('/', scheme_pos + 3)) : url;
                std::string path = (scheme_pos != std::string::npos && url.find('/', scheme_pos + 3) != std::string::npos) ? url.substr(url.find('/', scheme_pos + 3)) : "/";
                httplib::Client cli(host_port);
                cli.set_connection_timeout(5, 0);
                cli.set_read_timeout(5, 0);
                auto res = cli.Post(path.c_str(), data, "application/json");
                if (res && res->status == 200) {
                    Term result(res->body);
                    x_regs[0] = result;
                    last_result = result;
                    return true;
                }
                Term err(Atom{"error_http"});
                x_regs[0] = err;
                last_result = err;
                return true;
            }

            // HttpServer Built-ins
            if (is_mod_fn(fn_name, "HttpServer", "mulai") || is_mod_fn(fn_name, "HttpServer", "start")) {
                int64_t port = x_regs[0].get_if<int64_t>() ? *x_regs[0].get_if<int64_t>() : 8080;
                if (g_http_server_running) {
                    Term err(Atom{"error_server_running"});
                    x_regs[0] = err;
                    last_result = err;
                    return true;
                }
                g_http_server = std::make_shared<httplib::Server>();
                g_http_server->Get("/hello", [](const httplib::Request&, httplib::Response& res) {
                    res.set_content("Halo dari Garuda Server!", "text/plain");
                });
                g_http_server->Post("/echo", [](const httplib::Request& req, httplib::Response& res) {
                    res.set_content("Echo: " + req.body, "text/plain");
                });
                g_http_server_running = true;
                g_http_server_thread = std::thread([port]() {
                    g_http_server->listen("127.0.0.1", static_cast<int>(port));
                });
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
                Term res(Atom{"ok"});
                x_regs[0] = res;
                last_result = res;
                return true;
            }
            if (is_mod_fn(fn_name, "HttpServer", "stop")) {
                if (g_http_server_running && g_http_server) {
                    g_http_server->stop();
                    if (g_http_server_thread.joinable()) {
                        g_http_server_thread.join();
                    }
                    g_http_server_running = false;
                }
                Term res(Atom{"ok"});
                x_regs[0] = res;
                last_result = res;
                return true;
            }

            // Waktu Built-ins
            if (is_mod_fn(fn_name, "Waktu", "sekarang") || is_mod_fn(fn_name, "Waktu", "now")) {
                auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count();
                Term res(static_cast<int64_t>(now_ms));
                x_regs[0] = res;
                last_result = res;
                return true;
            }
            if (is_mod_fn(fn_name, "Waktu", "tunggu") || is_mod_fn(fn_name, "Waktu", "sleep")) {
                int64_t ms = x_regs[0].get_if<int64_t>() ? *x_regs[0].get_if<int64_t>() : 0;
                if (ms > 0) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
                }
                Term res(Atom{"nil"});
                x_regs[0] = res;
                last_result = res;
                return true;
            }

            // Map Built-ins
            if (is_mod_fn(fn_name, "Map", "baru") || is_mod_fn(fn_name, "Map", "new")) {
                Term res(Tuple{std::make_shared<const std::vector<Term>>()});
                x_regs[0] = res;
                last_result = res;
                return true;
            }
            if (is_mod_fn(fn_name, "Map", "panjang") || is_mod_fn(fn_name, "Map", "size") || is_mod_fn(fn_name, "Map", "length")) {
                int64_t sz = 0;
                if (auto* tup = x_regs[0].get_if<Tuple>()) {
                    if (tup->items) sz = static_cast<int64_t>(tup->items->size());
                }
                Term res(sz);
                x_regs[0] = res;
                last_result = res;
                return true;
            }
            if (is_mod_fn(fn_name, "Map", "taruh") || is_mod_fn(fn_name, "Map", "put")) {
                std::string k = clean_str(x_regs[1]);
                Term v = x_regs[2];
                std::vector<Term> entries;
                if (auto* tup = x_regs[0].get_if<Tuple>()) {
                    if (tup->items) entries = *tup->items;
                }
                bool updated = false;
                for (std::size_t i = 0; i < entries.size(); ++i) {
                    if (auto* entry = entries[i].get_if<Tuple>()) {
                        if (entry->items && entry->items->size() >= 2) {
                            if (clean_str((*entry->items)[0]) == k) {
                                std::vector<Term> pair = {Term(k), v};
                                entries[i] = Term(Tuple{std::make_shared<const std::vector<Term>>(std::move(pair))});
                                updated = true;
                                break;
                            }
                        }
                    }
                }
                if (!updated) {
                    std::vector<Term> pair = {Term(k), v};
                    entries.push_back(Term(Tuple{std::make_shared<const std::vector<Term>>(std::move(pair))}));
                }
                Term res(Tuple{std::make_shared<const std::vector<Term>>(std::move(entries))});
                x_regs[0] = res;
                last_result = res;
                return true;
            }
            if (is_mod_fn(fn_name, "Map", "ambil") || is_mod_fn(fn_name, "Map", "get")) {
                std::string k = clean_str(x_regs[1]);
                Term res;
                if (auto* tup = x_regs[0].get_if<Tuple>()) {
                    if (tup->items) {
                        for (const auto& item : *tup->items) {
                            if (auto* entry = item.get_if<Tuple>()) {
                                if (entry->items && entry->items->size() >= 2) {
                                    if (clean_str((*entry->items)[0]) == k) {
                                        res = (*entry->items)[1];
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
                x_regs[0] = res;
                last_result = res;
                return true;
            }
            if (is_mod_fn(fn_name, "Map", "hapus") || is_mod_fn(fn_name, "Map", "delete")) {
                std::string k = clean_str(x_regs[1]);
                std::vector<Term> entries;
                if (auto* tup = x_regs[0].get_if<Tuple>()) {
                    if (tup->items) {
                        for (const auto& item : *tup->items) {
                            if (auto* entry = item.get_if<Tuple>()) {
                                if (entry->items && entry->items->size() >= 2) {
                                    if (clean_str((*entry->items)[0]) == k) continue;
                                }
                            }
                            entries.push_back(item);
                        }
                    }
                }
                Term res(Tuple{std::make_shared<const std::vector<Term>>(std::move(entries))});
                x_regs[0] = res;
                last_result = res;
                return true;
            }
            if (is_mod_fn(fn_name, "Map", "kunci") || is_mod_fn(fn_name, "Map", "keys")) {
                std::vector<Term> keys;
                if (auto* tup = x_regs[0].get_if<Tuple>()) {
                    if (tup->items) {
                        for (const auto& item : *tup->items) {
                            if (auto* entry = item.get_if<Tuple>()) {
                                if (entry->items && entry->items->size() >= 2) {
                                    keys.push_back((*entry->items)[0]);
                                }
                            }
                        }
                    }
                }
                Term res(List{std::make_shared<const std::vector<Term>>(std::move(keys))});
                x_regs[0] = res;
                last_result = res;
                return true;
            }

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
            if (is_fn(fn_name, "pisah") || is_mod_fn(fn_name, "String", "pisah")) {
                std::string s = clean_str(x_regs[0]);
                std::string delim = clean_str(x_regs[1]);
                std::vector<Term> list_items;
                if (!delim.empty()) {
                    std::size_t start = 0, end = 0;
                    while ((end = s.find(delim, start)) != std::string::npos) {
                        list_items.push_back(Term(s.substr(start, end - start)));
                        start = end + delim.length();
                    }
                    list_items.push_back(Term(s.substr(start)));
                } else {
                    for (char c : s) list_items.push_back(Term(std::string(1, c)));
                }
                Term res(List{std::make_shared<const std::vector<Term>>(std::move(list_items))});
                x_regs[0] = res;
                last_result = res;
                return true;
            }
            if (is_fn(fn_name, "mengandung") || is_mod_fn(fn_name, "String", "mengandung")) {
                std::string s = clean_str(x_regs[0]);
                std::string sub = clean_str(x_regs[1]);
                bool res_b = (s.find(sub) != std::string::npos);
                Term res(Atom{res_b ? "true" : "false"});
                x_regs[0] = res;
                last_result = res;
                return true;
            }
            if (is_fn(fn_name, "rapikan") || is_mod_fn(fn_name, "String", "rapikan")) {
                std::string s = clean_str(x_regs[0]);
                std::size_t first = s.find_first_not_of(" \t\n\r");
                std::string res_s = "";
                if (first != std::string::npos) {
                    std::size_t last = s.find_last_not_of(" \t\n\r");
                    res_s = s.substr(first, (last - first + 1));
                }
                Term res(res_s);
                x_regs[0] = res;
                last_result = res;
                return true;
            }
            if (is_fn(fn_name, "karakter_ke_int") || is_mod_fn(fn_name, "String", "karakter_ke_int")) {
                std::string s = clean_str(x_regs[0]);
                int64_t code = 0;
                if (!s.empty()) code = static_cast<int64_t>(static_cast<unsigned char>(s[0]));
                Term res(code);
                x_regs[0] = res;
                last_result = res;
                return true;
            }
            if (is_fn(fn_name, "int_ke_karakter") || is_mod_fn(fn_name, "String", "int_ke_karakter")) {
                int64_t code = 0;
                if (auto* i = x_regs[0].get_if<std::int64_t>()) code = *i;
                std::string s = "";
                if (code >= 0 && code <= 255) s = std::string(1, static_cast<char>(code));
                Term res(s);
                x_regs[0] = res;
                last_result = res;
                return true;
            }

            // String.ganti(S, From, To)
            if (is_fn(fn_name, "ganti") || is_mod_fn(fn_name, "String", "ganti")) {
                std::string s = clean_str(x_regs[0]);
                std::string from = clean_str(x_regs[1]);
                std::string to = clean_str(x_regs[2]);
                if (!from.empty()) {
                    std::size_t pos = 0;
                    while ((pos = s.find(from, pos)) != std::string::npos) {
                        s.replace(pos, from.length(), to);
                        pos += to.length();
                    }
                }
                Term res(s);
                x_regs[0] = res;
                last_result = res;
                return true;
            }

            // String.cari(S, Sub)
            if (is_fn(fn_name, "cari") || is_mod_fn(fn_name, "String", "cari")) {
                std::string s = clean_str(x_regs[0]);
                std::string sub = clean_str(x_regs[1]);
                std::size_t pos = s.find(sub);
                int64_t idx = (pos == std::string::npos) ? -1 : static_cast<int64_t>(pos);
                Term res(idx);
                x_regs[0] = res;
                last_result = res;
                return true;
            }

            // String.mulai_dengan(S, Prefix)
            if (is_fn(fn_name, "mulai_dengan") || is_mod_fn(fn_name, "String", "mulai_dengan")) {
                std::string s = clean_str(x_regs[0]);
                std::string prefix = clean_str(x_regs[1]);
                bool res_b = (s.rfind(prefix, 0) == 0);
                Term res(Atom{res_b ? "true" : "false"});
                x_regs[0] = res;
                last_result = res;
                return true;
            }

            // String.akhir_dengan(S, Suffix)
            if (is_fn(fn_name, "akhir_dengan") || is_mod_fn(fn_name, "String", "akhir_dengan")) {
                std::string s = clean_str(x_regs[0]);
                std::string suffix = clean_str(x_regs[1]);
                bool res_b = (s.size() >= suffix.size() &&
                              s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0);
                Term res(Atom{res_b ? "true" : "false"});
                x_regs[0] = res;
                last_result = res;
                return true;
            }

            // String.balik(S)
            if (is_fn(fn_name, "balik") || is_mod_fn(fn_name, "String", "balik")) {
                std::string s = clean_str(x_regs[0]);
                std::reverse(s.begin(), s.end());
                Term res(s);
                x_regs[0] = res;
                last_result = res;
                return true;
            }

            // String.ulang(S, N)
            if (is_fn(fn_name, "ulang") || is_mod_fn(fn_name, "String", "ulang")) {
                std::string s = clean_str(x_regs[0]);
                int64_t n = x_regs[1].get_if<int64_t>() ? *x_regs[1].get_if<int64_t>() : 0;
                std::string result;
                for (int64_t i = 0; i < n; ++i) result += s;
                Term res(result);
                x_regs[0] = res;
                last_result = res;
                return true;
            }

            // String.ke_besar_awal(S)
            if (is_fn(fn_name, "ke_besar_awal") || is_mod_fn(fn_name, "String", "ke_besar_awal")) {
                std::string s = clean_str(x_regs[0]);
                if (!s.empty()) s[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(s[0])));
                Term res(s);
                x_regs[0] = res;
                last_result = res;
                return true;
            }
            return false;
        };

        auto read_operand = [&](const compiler::Operand& op) -> Term {
            if (op.type == compiler::RegType::Literal) return op.literal;
            return x_regs[op.index % 256];
        };

        auto get_int_val = [&](const Term& t) -> int64_t {
            if (auto* i = t.get_if<std::int64_t>()) return *i;
            if (auto* d = t.get_if<double>()) return static_cast<int64_t>(*d);
            if (auto* a = t.get_if<Atom>()) {
                if (a->name == "true" || a->name == "benar") return 1LL;
                if (a->name == "false" || a->name == "salah") return 0LL;
            }
            return 0LL;
        };

        auto match_fn = [&](const std::string& name) -> std::optional<std::size_t> {
            // Prioritas 1: exact match (qualified name)
            auto it = cg_out.function_table.find(name);
            if (it != cg_out.function_table.end()) {
                return it->second;
            }
            // Prioritas 2: exact match dengan aritas (name + "/N")
            for (const auto& [k, v] : cg_out.function_table) {
                if (k.rfind(name + "/", 0) == 0) {
                    return v;
                }
            }
            // Prioritas 3: short name exact
            std::string short_name = name;
            auto dot_pos = name.rfind('.');
            if (dot_pos != std::string::npos) {
                short_name = name.substr(dot_pos + 1);
            }
            it = cg_out.function_table.find(short_name);
            if (it != cg_out.function_table.end()) {
                return it->second;
            }
            // Prioritas 4: short name dengan aritas (deterministic: pilih offset terkecil)
            std::optional<std::size_t> best;
            for (const auto& [k, v] : cg_out.function_table) {
                std::string k_short = k;
                auto k_dot = k.rfind('.');
                if (k_dot != std::string::npos) {
                    k_short = k.substr(k_dot + 1);
                }
                if (k_short.rfind(short_name + "/", 0) == 0) {
                    if (!best.has_value() || v < *best) {
                        best = v;
                    }
                }
            }
            return best;
        };

        std::size_t trace_count = 0;
        while (ip < cg_out.all_instructions.size()) {
            const auto& inst = cg_out.all_instructions[ip];

            if (is_tracing && trace_count < 300) {
                trace_count++;
                std::cout << "EXEC IP " << ip << " op=" << static_cast<int>(inst.op) << " x[0..14]=";
                for (int r = 0; r <= 14; ++r) {
                    std::cout << " " << to_string(x_regs[r]);
                }
                std::cout << "\n";
            }

            switch (inst.op) {
                case Opcode::OpMakeTuple: {
                    if (!inst.args.empty()) {
                        std::vector<Term> items;
                        for (std::size_t i = 1; i < inst.args.size(); ++i) {
                            items.push_back(read_operand(inst.args[i]));
                        }
                        Term res(Tuple{std::make_shared<const std::vector<Term>>(std::move(items))});
                        x_regs[inst.args[0].index % 256] = res;
                        last_result = res;
                    }
                    break;
                }
                case Opcode::OpGetTupleElem: {
                    if (inst.args.size() >= 3) {
                        Term tuple_term = read_operand(inst.args[0]);
                        std::size_t idx = static_cast<std::size_t>(get_int_val(read_operand(inst.args[1])));
                        if (auto* tup = tuple_term.get_if<Tuple>()) {
                            if (tup->items && idx < tup->items->size()) {
                                Term elem = (*tup->items)[idx];
                                x_regs[inst.args[2].index % 256] = elem;
                                last_result = elem;
                            }
                        } else if (auto* lst = tuple_term.get_if<List>()) {
                            if (lst->items && idx < lst->items->size()) {
                                Term elem = (*lst->items)[idx];
                                x_regs[inst.args[2].index % 256] = elem;
                                last_result = elem;
                            }
                        }
                    }
                    break;
                }
                case Opcode::OpMakeList: {
                    if (!inst.args.empty()) {
                        std::vector<Term> items;
                        for (std::size_t i = 1; i < inst.args.size(); ++i) {
                            items.push_back(read_operand(inst.args[i]));
                        }
                        Term res(List{std::make_shared<const std::vector<Term>>(std::move(items))});
                        x_regs[inst.args[0].index % 256] = res;
                        last_result = res;
                    }
                    break;
                }
                case Opcode::OpGetListLen: {
                    if (inst.args.size() >= 2) {
                        Term t = read_operand(inst.args[0]);
                        std::int64_t n = 0;
                        if (auto* l = t.get_if<List>()) {
                            n = l->items ? static_cast<std::int64_t>(l->items->size()) : 0;
                        }
                        Term res(n);
                        x_regs[inst.args[1].index % 256] = res;
                        last_result = res;
                    }
                    break;
                }
                case Opcode::OpGetListElem: {
                    if (inst.args.size() >= 3) {
                        Term t = read_operand(inst.args[0]);
                        std::size_t idx = static_cast<std::size_t>(get_int_val(read_operand(inst.args[1])));
                        if (auto* l = t.get_if<List>()) {
                            if (l->items && idx < l->items->size()) {
                                Term elem = (*l->items)[idx];
                                x_regs[inst.args[2].index % 256] = elem;
                                last_result = elem;
                            }
                        }
                    }
                    break;
                }
                case Opcode::OpGetListTail: {
                    if (inst.args.size() >= 2) {
                        Term t = read_operand(inst.args[0]);
                        std::vector<Term> rest;
                        if (auto* l = t.get_if<List>()) {
                            if (l->items && l->items->size() > 1) {
                                rest.assign(l->items->begin() + 1, l->items->end());
                            }
                        }
                        Term res(List{std::make_shared<const std::vector<Term>>(std::move(rest))});
                        x_regs[inst.args[1].index % 256] = res;
                        last_result = res;
                    }
                    break;
                }
                case Opcode::OpEq: {
                    if (inst.args.size() >= 3) {
                        Term v1 = read_operand(inst.args[0]);
                        Term v2 = read_operand(inst.args[1]);
                        bool eq = (v1 == v2);
                        x_regs[inst.args[2].index % 256] = Term(eq ? 1LL : 0LL);
                        last_result = Term(eq ? 1LL : 0LL);
                    }
                    break;
                }
                case Opcode::OpNeq: {
                    if (inst.args.size() >= 3) {
                        Term v1 = read_operand(inst.args[0]);
                        Term v2 = read_operand(inst.args[1]);
                        bool neq = (v1 != v2);
                        x_regs[inst.args[2].index % 256] = Term(neq ? 1LL : 0LL);
                        last_result = Term(neq ? 1LL : 0LL);
                    }
                    break;
                }
                case Opcode::OpLt: {
                    if (inst.args.size() >= 3) {
                        int64_t v1 = get_int_val(read_operand(inst.args[0]));
                        int64_t v2 = get_int_val(read_operand(inst.args[1]));
                        x_regs[inst.args[2].index % 256] = Term(v1 < v2 ? 1LL : 0LL);
                        last_result = Term(v1 < v2 ? 1LL : 0LL);
                    }
                    break;
                }
                case Opcode::OpLte: {
                    if (inst.args.size() >= 3) {
                        int64_t v1 = get_int_val(read_operand(inst.args[0]));
                        int64_t v2 = get_int_val(read_operand(inst.args[1]));
                        x_regs[inst.args[2].index % 256] = Term(v1 <= v2 ? 1LL : 0LL);
                        last_result = Term(v1 <= v2 ? 1LL : 0LL);
                    }
                    break;
                }
                case Opcode::OpGt: {
                    if (inst.args.size() >= 3) {
                        int64_t v1 = get_int_val(read_operand(inst.args[0]));
                        int64_t v2 = get_int_val(read_operand(inst.args[1]));
                        x_regs[inst.args[2].index % 256] = Term(v1 > v2 ? 1LL : 0LL);
                        last_result = Term(v1 > v2 ? 1LL : 0LL);
                    }
                    break;
                }
                case Opcode::OpGte: {
                    if (inst.args.size() >= 3) {
                        int64_t v1 = get_int_val(read_operand(inst.args[0]));
                        int64_t v2 = get_int_val(read_operand(inst.args[1]));
                        x_regs[inst.args[2].index % 256] = Term(v1 >= v2 ? 1LL : 0LL);
                        last_result = Term(v1 >= v2 ? 1LL : 0LL);
                    }
                    break;
                }
                case Opcode::OpJump: {
                    if (!inst.args.empty()) {
                        std::size_t target_ip = static_cast<std::size_t>(get_int_val(read_operand(inst.args[0])));
                        ip = target_ip;
                        continue;
                    }
                    break;
                }
                case Opcode::OpJumpIfFalse: {
                    if (inst.args.size() >= 2) {
                        Term cond_val = read_operand(inst.args[0]);
                        int64_t cond_int = get_int_val(cond_val);
                        if (cond_int == 0) {
                            std::size_t target_ip = static_cast<std::size_t>(get_int_val(read_operand(inst.args[1])));
                            ip = target_ip;
                            continue;
                        }
                    }
                    break;
                }
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

                        if (val_a.is<List>() || val_b.is<List>()) {
                            std::vector<Term> items;
                            auto add_items = [&](const Term& t) {
                                if (auto* l = t.get_if<List>()) {
                                    if (l->items) {
                                        for (const auto& elem : *l->items) items.push_back(elem);
                                    }
                                } else if (!t.is<std::monostate>()) {
                                    items.push_back(t);
                                }
                            };
                            add_items(val_a);
                            add_items(val_b);
                            Term res(List{std::make_shared<const std::vector<Term>>(std::move(items))});
                            x_regs[inst.args[2].index % 256] = res;
                            last_result = res;
                            break;
                        }

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
                        Term fn_term = read_operand(inst.args[0]);
                        std::string fn_name;
                        if (auto* a = fn_term.get_if<Atom>()) fn_name = a->name;
                        else if (auto* s = fn_term.get_if<std::string>()) fn_name = *s;

                        if (!fn_name.empty() && handle_builtin(fn_name)) break;

                        if (auto* i = fn_term.get_if<std::int64_t>()) {
                            call_stack.push_back(ip + 1);
                            frame_stack.push_back(x_regs);
                            ip = static_cast<std::size_t>(*i);
                            continue;
                        }

                        if (!fn_name.empty()) {
                            auto target_opt = match_fn(fn_name);
                            if (target_opt.has_value()) {
                                call_stack.push_back(ip + 1);
                                frame_stack.push_back(x_regs);
                                ip = *target_opt;
                                continue;
                            }
                        }
                    }
                    break;
                }
                case Opcode::OpTailCall: {
                    if (!inst.args.empty()) {
                        Term fn_term = read_operand(inst.args[0]);
                        std::string fn_name;
                        if (auto* a = fn_term.get_if<Atom>()) fn_name = a->name;
                        else if (auto* s = fn_term.get_if<std::string>()) fn_name = *s;

                        if (!fn_name.empty() && handle_builtin(fn_name)) break;

                        if (auto* i = fn_term.get_if<std::int64_t>()) {
                            ip = static_cast<std::size_t>(*i);
                            continue;
                        }

                        if (!fn_name.empty()) {
                            auto target_opt = match_fn(fn_name);
                            if (target_opt.has_value()) {
                                ip = *target_opt;
                                continue;
                            }
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
