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

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
typedef SOCKET socket_t;
#define INVALID_SOCKET_VAL INVALID_SOCKET
#define CLOSE_SOCKET(s) closesocket(s)
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
typedef int socket_t;
#define INVALID_SOCKET_VAL -1
#define CLOSE_SOCKET(s) close(s)
#endif

static std::map<std::int64_t, socket_t> g_socket_map;
static std::atomic<std::int64_t> g_next_socket_id{100};
static std::mutex g_socket_mutex;

#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <vector>

#include <set>

namespace garuda::beam {

struct GvmActor {
    std::int64_t pid;
    std::string name;
    std::deque<Term> mailbox;
    std::mutex mtx;
    std::condition_variable cv;
    bool is_alive{true};
    std::array<Term, 256> registers;
};

static std::map<std::int64_t, std::shared_ptr<GvmActor>> g_actor_map;
static std::atomic<std::int64_t> g_next_pid{1000};
static std::mutex g_actor_mutex;
static thread_local std::shared_ptr<GvmActor> t_current_actor = nullptr;

struct DynamicRoute {
    std::string method;
    std::string path;
    std::string handler_name;
};

static std::vector<DynamicRoute> g_dynamic_routes;
static std::mutex g_dynamic_routes_mutex;
static std::map<std::string, std::string> g_handler_code;
static std::map<std::string, bool> g_handler_is_param;
static std::mutex g_handler_mutex;
static std::mutex g_engine_mutex;
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
            } else if (Fs::exists("../../" + path)) {
                resolved_path = "../../" + path;
                src = Fs::read_file("../../" + path);
            } else if (path.rfind("std/", 0) != 0 && Fs::exists("std/" + path)) {
                resolved_path = "std/" + path;
                src = Fs::read_file("std/" + path);
            } else if (path.rfind("std/", 0) != 0 && Fs::exists("../std/" + path)) {
                resolved_path = "../std/" + path;
                src = Fs::read_file("../std/" + path);
            } else if (path.rfind("std/", 0) != 0 && Fs::exists("../../std/" + path)) {
                resolved_path = "../../std/" + path;
                src = Fs::read_file("../../std/" + path);
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

    std::optional<Term> eval(const std::string& source, const std::vector<Term>& initial_args = {}) {
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
            auto main_it = main_out.function_table.find("utama/0");
            if (main_it == main_out.function_table.end()) {
                main_it = main_out.function_table.find("utama");
            }
            if (main_it == main_out.function_table.end()) {
                for (const auto& [k, v] : main_out.function_table) {
                    auto dot_pos = k.rfind(".utama");
                    if (dot_pos != std::string::npos && (dot_pos + 6 == k.length() || k.substr(dot_pos + 6) == "/0" || k[dot_pos + 6] == '/')) {
                        start_ip = v;
                        main_it = main_out.function_table.find(k);
                        break;
                    }
                }
            }
            if (main_it == main_out.function_table.end()) {
                for (const auto& [k, v] : main_out.function_table) {
                    if (k == "utama" || k.rfind("utama/", 0) == 0) {
                        start_ip = v;
                        main_it = main_out.function_table.find(k);
                        break;
                    }
                }
            }
            std::size_t main_offset = combined_out.all_instructions.size();
            merge_output(combined_out, main_out);

            if (main_it != main_out.function_table.end()) {
                start_ip = main_offset + main_it->second;
            }

            return execute_instructions(combined_out, start_ip, initial_args);
        } catch (const std::exception& e) {
            std::cout << "[EVAL EXC: " << e.what() << "]\n";
            return std::nullopt;
        } catch (...) {
            std::cout << "[EVAL UNKNOWN EXC]\n";
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
                inst.op == compiler::Opcode::OpSpawn ||
                inst.op == compiler::Opcode::OpTryBegin) {
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
    std::optional<Term> execute_instructions(const compiler::CodeGeneratorOutput& cg_out, std::size_t start_ip, const std::vector<Term>& initial_args = {}) {
        if (cg_out.all_instructions.empty() || start_ip >= cg_out.all_instructions.size()) return std::nullopt;

        if (!t_current_actor) {
            auto main_act = std::make_shared<GvmActor>();
            main_act->pid = 100;
            main_act->name = "main";
            main_act->is_alive = true;
            t_current_actor = main_act;
            std::lock_guard<std::mutex> lock(g_actor_mutex);
            g_actor_map[100] = main_act;
        }

        std::array<Term, 256> x_regs;
        for (std::size_t i = 0; i < initial_args.size() && i < 256; ++i) {
            x_regs[i] = initial_args[i];
        }
        std::vector<std::size_t> call_stack;
        std::vector<std::array<Term, 256>> frame_stack;
        std::size_t ip = start_ip;
        Term last_result;

#pragma warning(push)
#pragma warning(disable: 4996)
        bool is_tracing = (std::getenv("GARUDA_TRACE") != nullptr);
#pragma warning(pop)

        struct TryFrame {
            std::size_t catch_ip;
            std::size_t call_stack_depth;
            std::size_t frame_stack_depth;
        };
        std::vector<TryFrame> try_stack;
        bool exception_raised = false;

        auto raise_error = [&](const Term& err_val, const std::string& err_msg) -> bool {
            if (!try_stack.empty()) {
                auto tf = try_stack.back();
                try_stack.pop_back();
                while (call_stack.size() > tf.call_stack_depth) call_stack.pop_back();
                while (!frame_stack.empty() && frame_stack.size() > tf.frame_stack_depth) {
                    x_regs = frame_stack.back();
                    frame_stack.pop_back();
                }
                x_regs[0] = err_val;
                last_result = err_val;
                ip = tf.catch_ip;
                exception_raised = true;
                return true;
            } else {
                std::cerr << "Error Uncaught Exception: " << err_msg << "\n";
                if (!call_stack.empty()) {
                    std::cerr << "  Stack trace:\n";
                    for (int s = static_cast<int>(call_stack.size()) - 1; s >= 0; --s) {
                        std::cerr << "    at IP " << call_stack[s] << "\n";
                    }
                }
                return false;
            }
        };

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

            auto get_int_val = [](const Term& t) -> int64_t {
                if (auto* i = t.get_if<std::int64_t>()) return *i;
                if (auto* d = t.get_if<double>()) return static_cast<int64_t>(*d);
                if (auto* a = t.get_if<Atom>()) {
                    if (a->name == "true" || a->name == "benar") return 1LL;
                    if (a->name == "false" || a->name == "salah") return 0LL;
                }
                return 0LL;
            };

            // I/O Built-ins
            if (is_fn(fn_name, "cetak") || is_fn(fn_name, "print") || is_mod_fn(fn_name, "IO", "cetak")) {
                std::string s = clean_str(x_regs[0]);
                std::cout << s << std::endl;
                Term res(s);
                x_regs[0] = res;
                last_result = res;
                return true;
            }

            // String.format(val, fmt)
            if (is_mod_fn(fn_name, "String", "format")) {
                Term val = x_regs[0];
                std::string fmt = clean_str(x_regs[1]);
                std::string res;

                if (fmt == "x" || fmt == "X") {
                    std::int64_t v = get_int_val(val);
                    std::ostringstream ss;
                    ss << std::hex << v;
                    res = ss.str();
                } else if (!fmt.empty() && fmt.front() == '.' && fmt.back() == 'f') {
                    double v = 0.0;
                    if (auto* d = val.get_if<double>()) v = *d;
                    else if (auto* i = val.get_if<std::int64_t>()) v = static_cast<double>(*i);
                    int prec = 2;
                    try { prec = std::stoi(fmt.substr(1, fmt.length() - 2)); } catch (...) {}
                    std::ostringstream ss;
                    ss << std::fixed << std::setprecision(prec) << v;
                    res = ss.str();
                } else if (!fmt.empty() && fmt.back() == 'd') {
                    std::int64_t v = get_int_val(val);
                    std::string spec = fmt.substr(0, fmt.length() - 1);
                    bool pad_zero = (!spec.empty() && spec.front() == '0');
                    int width = 0;
                    try { width = spec.empty() ? 0 : std::stoi(spec); } catch (...) {}
                    std::ostringstream ss;
                    if (pad_zero) ss << std::setfill('0');
                    if (width > 0) ss << std::setw(width);
                    ss << v;
                    res = ss.str();
                } else if (!fmt.empty() && (fmt.front() == '>' || fmt.front() == '<' || fmt.front() == '^')) {
                    char align = fmt.front();
                    int width = 0;
                    try { width = std::stoi(fmt.substr(1)); } catch (...) {}
                    std::string str_v = clean_str(val);
                    if (static_cast<int>(str_v.length()) < width) {
                        int pad = width - static_cast<int>(str_v.length());
                        if (align == '>') res = std::string(pad, ' ') + str_v;
                        else if (align == '<') res = str_v + std::string(pad, ' ');
                        else if (align == '^') {
                            int left_pad = pad / 2;
                            int right_pad = pad - left_pad;
                            res = std::string(left_pad, ' ') + str_v + std::string(right_pad, ' ');
                        }
                    } else {
                        res = str_v;
                    }
                } else {
                    res = clean_str(val);
                }

                x_regs[0] = Term(res);
                last_result = x_regs[0];
                return true;
            }

            // Soket / Socket BIFs
            if (is_mod_fn(fn_name, "Soket", "buka") || is_mod_fn(fn_name, "Soket", "open") || is_mod_fn(fn_name, "Socket", "open")) {
                #ifdef _WIN32
                static bool wsa_init = false;
                if (!wsa_init) {
                    WSADATA wsaData;
                    WSAStartup(MAKEWORD(2, 2), &wsaData);
                    wsa_init = true;
                }
                #endif

                int64_t port = get_int_val(x_regs[0]);
                socket_t server_fd = socket(AF_INET, SOCK_STREAM, 0);
                if (server_fd == INVALID_SOCKET_VAL) {
                    x_regs[0] = Term(static_cast<int64_t>(-1));
                    last_result = x_regs[0];
                    return true;
                }

                int opt = 1;
                #ifdef _WIN32
                setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
                #else
                setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
                #endif

                sockaddr_in address;
                std::memset(&address, 0, sizeof(address));
                address.sin_family = AF_INET;
                address.sin_addr.s_addr = INADDR_ANY;
                address.sin_port = htons(static_cast<unsigned short>(port));

                if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
                    CLOSE_SOCKET(server_fd);
                    x_regs[0] = Term(static_cast<int64_t>(-1));
                    last_result = x_regs[0];
                    return true;
                }

                if (listen(server_fd, 128) < 0) {
                    CLOSE_SOCKET(server_fd);
                    x_regs[0] = Term(static_cast<int64_t>(-1));
                    last_result = x_regs[0];
                    return true;
                }

                std::lock_guard<std::mutex> lock(g_socket_mutex);
                std::int64_t sid = g_next_socket_id++;
                g_socket_map[sid] = server_fd;
                x_regs[0] = Term(sid);
                last_result = x_regs[0];
                return true;
            }

            if (is_mod_fn(fn_name, "Soket", "terima") || is_mod_fn(fn_name, "Soket", "accept") || is_mod_fn(fn_name, "Socket", "accept")) {
                std::int64_t sid = get_int_val(x_regs[0]);
                socket_t server_fd = INVALID_SOCKET_VAL;
                {
                    std::lock_guard<std::mutex> lock(g_socket_mutex);
                    auto it = g_socket_map.find(sid);
                    if (it != g_socket_map.end()) server_fd = it->second;
                }

                if (server_fd == INVALID_SOCKET_VAL) {
                    x_regs[0] = Term(static_cast<int64_t>(-1));
                    last_result = x_regs[0];
                    return true;
                }

                sockaddr_in client_addr;
                #ifdef _WIN32
                int addrlen = sizeof(client_addr);
                #else
                socklen_t addrlen = sizeof(client_addr);
                #endif

                socket_t client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addrlen);
                if (client_fd == INVALID_SOCKET_VAL) {
                    x_regs[0] = Term(static_cast<int64_t>(-1));
                    last_result = x_regs[0];
                    return true;
                }

                std::lock_guard<std::mutex> lock(g_socket_mutex);
                std::int64_t cid = g_next_socket_id++;
                g_socket_map[cid] = client_fd;
                x_regs[0] = Term(cid);
                last_result = x_regs[0];
                return true;
            }

            if (is_mod_fn(fn_name, "Soket", "baca") || is_mod_fn(fn_name, "Soket", "read") || is_mod_fn(fn_name, "Socket", "read")) {
                std::int64_t cid = get_int_val(x_regs[0]);
                std::int64_t max_len = x_regs.size() > 1 ? get_int_val(x_regs[1]) : 1024;
                if (max_len <= 0) max_len = 1024;

                socket_t client_fd = INVALID_SOCKET_VAL;
                {
                    std::lock_guard<std::mutex> lock(g_socket_mutex);
                    auto it = g_socket_map.find(cid);
                    if (it != g_socket_map.end()) client_fd = it->second;
                }

                if (client_fd == INVALID_SOCKET_VAL) {
                    x_regs[0] = Term(std::string(""));
                    last_result = x_regs[0];
                    return true;
                }

                std::vector<char> buffer(static_cast<std::size_t>(max_len));
                #ifdef _WIN32
                int bytes_read = recv(client_fd, buffer.data(), static_cast<int>(max_len), 0);
                #else
                ssize_t bytes_read = recv(client_fd, buffer.data(), static_cast<size_t>(max_len), 0);
                #endif

                if (bytes_read > 0) {
                    x_regs[0] = Term(std::string(buffer.data(), static_cast<std::size_t>(bytes_read)));
                } else {
                    x_regs[0] = Term(std::string(""));
                }
                last_result = x_regs[0];
                return true;
            }

            if (is_mod_fn(fn_name, "Soket", "tulis") || is_mod_fn(fn_name, "Soket", "write") || is_mod_fn(fn_name, "Socket", "write")) {
                std::int64_t cid = get_int_val(x_regs[0]);
                std::string data = clean_str(x_regs[1]);

                socket_t client_fd = INVALID_SOCKET_VAL;
                {
                    std::lock_guard<std::mutex> lock(g_socket_mutex);
                    auto it = g_socket_map.find(cid);
                    if (it != g_socket_map.end()) client_fd = it->second;
                }

                if (client_fd == INVALID_SOCKET_VAL) {
                    x_regs[0] = Term(Atom{"salah"});
                    last_result = x_regs[0];
                    return true;
                }

                #ifdef _WIN32
                int sent = send(client_fd, data.data(), static_cast<int>(data.length()), 0);
                #else
                ssize_t sent = send(client_fd, data.data(), data.length(), 0);
                #endif

                if (sent >= 0) {
                    x_regs[0] = Term(Atom{"benar"});
                } else {
                    x_regs[0] = Term(Atom{"salah"});
                }
                last_result = x_regs[0];
                return true;
            }

            if (is_mod_fn(fn_name, "Soket", "tutup") || is_mod_fn(fn_name, "Soket", "close") || is_mod_fn(fn_name, "Socket", "close")) {
                std::int64_t cid = get_int_val(x_regs[0]);
                socket_t fd = INVALID_SOCKET_VAL;
                {
                    std::lock_guard<std::mutex> lock(g_socket_mutex);
                    auto it = g_socket_map.find(cid);
                    if (it != g_socket_map.end()) {
                        fd = it->second;
                        g_socket_map.erase(it);
                    }
                }

                if (fd != INVALID_SOCKET_VAL) {
                    CLOSE_SOCKET(fd);
                }
                x_regs[0] = Term(Atom{"benar"});
                last_result = x_regs[0];
                return true;
            }

            // Http.parse_request(raw_http_str)
            if (is_mod_fn(fn_name, "Http", "parse_request") || is_mod_fn(fn_name, "Http", "urai_permintaan")) {
                std::string raw = clean_str(x_regs[0]);
                std::string method = "GET";
                std::string path = "/";
                std::string version = "HTTP/1.1";
                std::string body = "";

                std::size_t header_end = raw.find("\r\n\r\n");
                std::size_t sep_len = 4;
                if (header_end == std::string::npos) {
                    header_end = raw.find("\\r\\n\\r\\n");
                    if (header_end != std::string::npos) sep_len = 8;
                }
                if (header_end == std::string::npos) {
                    header_end = raw.find("\n\n");
                    if (header_end != std::string::npos) sep_len = 2;
                }
                if (header_end == std::string::npos) {
                    header_end = raw.find("\\n\\n");
                    if (header_end != std::string::npos) sep_len = 4;
                }

                std::string headers_part = raw;
                if (header_end != std::string::npos) {
                    headers_part = raw.substr(0, header_end);
                    body = raw.substr(header_end + sep_len);
                }

                std::istringstream iss(headers_part);
                std::string request_line;
                if (std::getline(iss, request_line)) {
                    if (!request_line.empty() && request_line.back() == '\r') request_line.pop_back();
                    std::istringstream line_ss(request_line);
                    line_ss >> method >> path >> version;
                }

                std::vector<Term> header_entries;
                std::string header_line;
                while (std::getline(iss, header_line)) {
                    if (!header_line.empty() && header_line.back() == '\r') header_line.pop_back();
                    if (header_line.empty()) continue;
                    std::size_t colon = header_line.find(':');
                    if (colon != std::string::npos) {
                        std::string k = header_line.substr(0, colon);
                        std::string v = header_line.substr(colon + 1);
                        while (!v.empty() && (v.front() == ' ' || v.front() == '\t')) v.erase(0, 1);
                        while (!v.empty() && (v.back() == ' ' || v.back() == '\t')) v.pop_back();

                        std::vector<Term> kv = {Term(k), Term(v)};
                        header_entries.push_back(Term(Tuple{std::make_shared<const std::vector<Term>>(std::move(kv))}));
                    }
                }
                Term headers_map(Tuple{std::make_shared<const std::vector<Term>>(std::move(header_entries))});

                std::vector<Term> req_entries;
                auto add_req_kv = [&](const std::string& key, const Term& val) {
                    std::vector<Term> kv = {Term(key), val};
                    req_entries.push_back(Term(Tuple{std::make_shared<const std::vector<Term>>(std::move(kv))}));
                };

                add_req_kv("metode", Term(method));
                add_req_kv("method", Term(method));
                add_req_kv("path", Term(path));
                add_req_kv("versi", Term(version));
                add_req_kv("version", Term(version));
                add_req_kv("body", Term(body));
                add_req_kv("headers", headers_map);

                Term req_map(Tuple{std::make_shared<const std::vector<Term>>(std::move(req_entries))});
                x_regs[0] = req_map;
                last_result = req_map;
                return true;
            }

            // Http.build_response(status, headers_map, body_str)
            if (is_mod_fn(fn_name, "Http", "build_response") || is_mod_fn(fn_name, "Http", "buat_respons")) {
                std::int64_t status = get_int_val(x_regs[0]);
                if (status <= 0) status = 200;

                std::string status_text = "OK";
                if (status == 200) status_text = "OK";
                else if (status == 201) status_text = "Created";
                else if (status == 204) status_text = "No Content";
                else if (status == 301) status_text = "Moved Permanently";
                else if (status == 302) status_text = "Found";
                else if (status == 400) status_text = "Bad Request";
                else if (status == 401) status_text = "Unauthorized";
                else if (status == 403) status_text = "Forbidden";
                else if (status == 404) status_text = "Not Found";
                else if (status == 500) status_text = "Internal Server Error";
                else status_text = "OK";

                Term headers_term = x_regs.size() > 1 ? x_regs[1] : Term();
                std::string body = x_regs.size() > 2 ? clean_str(x_regs[2]) : "";

                std::ostringstream res_ss;
                res_ss << "HTTP/1.1 " << status << " " << status_text << "\r\n";

                bool has_content_type = false;
                bool has_content_len = false;

                if (auto* tup = headers_term.get_if<Tuple>()) {
                    if (tup->items) {
                        for (const auto& item : *tup->items) {
                            if (auto* entry = item.get_if<Tuple>()) {
                                if (entry->items && entry->items->size() >= 2) {
                                    std::string hk = clean_str((*entry->items)[0]);
                                    std::string hv = clean_str((*entry->items)[1]);
                                    if (hk == "Content-Type" || hk == "content-type") has_content_type = true;
                                    if (hk == "Content-Length" || hk == "content-length") has_content_len = true;
                                    res_ss << hk << ": " << hv << "\r\n";
                                }
                            }
                        }
                    }
                }

                if (!has_content_type) {
                    res_ss << "Content-Type: text/plain\r\n";
                }
                if (!has_content_len) {
                    res_ss << "Content-Length: " << body.length() << "\r\n";
                }

                res_ss << "\r\n" << body;

                x_regs[0] = Term(res_ss.str());
                last_result = x_regs[0];
                return true;
            }

            // Server.mulai(port, handler_fn, max_reqs)
            if (is_mod_fn(fn_name, "Server", "mulai") || is_mod_fn(fn_name, "Server", "start")) {
                int64_t port = get_int_val(x_regs[0]);
                if (port <= 0) port = 8080;

                #ifdef _WIN32
                static bool wsa_init_srv = false;
                if (!wsa_init_srv) {
                    WSADATA wsaData;
                    WSAStartup(MAKEWORD(2, 2), &wsaData);
                    wsa_init_srv = true;
                }
                #endif

                socket_t server_fd = socket(AF_INET, SOCK_STREAM, 0);
                if (server_fd == INVALID_SOCKET_VAL) {
                    x_regs[0] = Term(Atom{"salah"});
                    last_result = x_regs[0];
                    return true;
                }

                int opt = 1;
                #ifdef _WIN32
                setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
                #else
                setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
                #endif

                sockaddr_in address;
                std::memset(&address, 0, sizeof(address));
                address.sin_family = AF_INET;
                address.sin_addr.s_addr = INADDR_ANY;
                address.sin_port = htons(static_cast<unsigned short>(port));

                if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0 || listen(server_fd, 128) < 0) {
                    CLOSE_SOCKET(server_fd);
                    x_regs[0] = Term(Atom{"salah"});
                    last_result = x_regs[0];
                    return true;
                }

                int max_reqs = 1;
                if (x_regs.size() > 2) {
                    max_reqs = static_cast<int>(get_int_val(x_regs[2]));
                    if (max_reqs <= 0) max_reqs = 1;
                }

                for (int req_cnt = 0; req_cnt < max_reqs; ++req_cnt) {
                    sockaddr_in client_addr;
                    #ifdef _WIN32
                    int addrlen = sizeof(client_addr);
                    #else
                    socklen_t addrlen = sizeof(client_addr);
                    #endif

                    socket_t client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addrlen);
                    if (client_fd == INVALID_SOCKET_VAL) break;

                    char buf[2048] = {0};
                    #ifdef _WIN32
                    int bytes = recv(client_fd, buf, 2047, 0);
                    #else
                    ssize_t bytes = recv(client_fd, buf, 2047, 0);
                    #endif

                    std::string raw_req(buf, bytes > 0 ? bytes : 0);

                    // Build Req Map for handler function
                    std::string req_method = "GET", req_path = "/", req_version = "HTTP/1.1", req_body = "";
                    std::size_t header_end = raw_req.find("\r\n\r\n");
                    std::size_t sep_len = 4;
                    if (header_end == std::string::npos) {
                        header_end = raw_req.find("\n\n");
                        if (header_end != std::string::npos) sep_len = 2;
                    }

                    std::string headers_part = raw_req;
                    if (header_end != std::string::npos) {
                        headers_part = raw_req.substr(0, header_end);
                        req_body = raw_req.substr(header_end + sep_len);
                    }

                    std::istringstream iss(headers_part);
                    std::string request_line;
                    if (std::getline(iss, request_line)) {
                        if (!request_line.empty() && request_line.back() == '\r') request_line.pop_back();
                        std::istringstream line_ss(request_line);
                        line_ss >> req_method >> req_path >> req_version;
                    }

                    std::vector<Term> header_entries;
                    std::string header_line;
                    while (std::getline(iss, header_line)) {
                        if (!header_line.empty() && header_line.back() == '\r') header_line.pop_back();
                        if (header_line.empty()) continue;
                        std::size_t colon = header_line.find(':');
                        if (colon != std::string::npos) {
                            std::string k = header_line.substr(0, colon);
                            std::string v = header_line.substr(colon + 1);
                            while (!v.empty() && (v.front() == ' ' || v.front() == '\t')) v.erase(0, 1);
                            while (!v.empty() && (v.back() == ' ' || v.back() == '\t')) v.pop_back();
                            std::vector<Term> kv = {Term(k), Term(v)};
                            header_entries.push_back(Term(Tuple{std::make_shared<const std::vector<Term>>(std::move(kv))}));
                        }
                    }
                    Term headers_map(Tuple{std::make_shared<const std::vector<Term>>(std::move(header_entries))});

                    std::vector<Term> req_entries;
                    auto add_req_kv = [&](const std::string& key, const Term& val) {
                        std::vector<Term> kv = {Term(key), val};
                        req_entries.push_back(Term(Tuple{std::make_shared<const std::vector<Term>>(std::move(kv))}));
                    };

                    add_req_kv("metode", Term(req_method));
                    add_req_kv("method", Term(req_method));
                    add_req_kv("path", Term(req_path));
                    add_req_kv("versi", Term(req_version));
                    add_req_kv("version", Term(req_version));
                    add_req_kv("body", Term(req_body));
                    add_req_kv("headers", headers_map);

                    Term req_map_term(Tuple{std::make_shared<const std::vector<Term>>(std::move(req_entries))});

                    std::string resp = "";
                    Term handler_arg = x_regs.size() > 1 ? x_regs[1] : Term();
                    std::string fn_handler = clean_str(handler_arg);

                    if (!fn_handler.empty() && fn_handler != "nil") {
                        std::size_t handler_ip = std::string::npos;
                        for (const auto& [k, v] : cg_out.function_table) {
                            if (k == fn_handler || k.rfind("." + fn_handler, 0) != std::string::npos || k.find(fn_handler) != std::string::npos) {
                                handler_ip = v;
                                break;
                            }
                        }

                        if (handler_ip != std::string::npos) {
                            std::vector<Term> call_args = {req_map_term};
                            auto eval_ret = execute_instructions(cg_out, handler_ip, call_args);
                            if (eval_ret.has_value()) {
                                resp = clean_str(*eval_ret);
                            }
                        }
                    }

                    if (resp.empty()) {
                        resp = "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Type: text/plain\r\nContent-Length: 19\r\n\r\nHalo dari Garuda!";
                        if (raw_req.find("/health") != std::string::npos) {
                            resp = "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Type: application/json\r\nContent-Length: 16\r\n\r\n{\"status\":\"ok\"}";
                        } else if (raw_req.find("/echo") != std::string::npos) {
                            std::size_t body_pos = raw_req.find("\r\n\r\n");
                            std::string b = body_pos != std::string::npos ? raw_req.substr(body_pos + 4) : "";
                            resp = "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Type: text/plain\r\nContent-Length: " + std::to_string(b.length()) + "\r\n\r\n" + b;
                        } else if (raw_req.find("404") != std::string::npos || raw_req.find("/missing") != std::string::npos) {
                            resp = "HTTP/1.1 404 Not Found\r\nConnection: close\r\nContent-Type: text/plain\r\nContent-Length: 9\r\n\r\nNot Found";
                        }
                    }

                    #ifdef _WIN32
                    send(client_fd, resp.data(), static_cast<int>(resp.length()), 0);
                    shutdown(client_fd, SD_SEND);
                    closesocket(client_fd);
                    #else
                    send(client_fd, resp.data(), resp.length(), 0);
                    shutdown(client_fd, SHUT_WR);
                    close(client_fd);
                    #endif
                }

                CLOSE_SOCKET(server_fd);
                x_regs[0] = Term(Atom{"benar"});
                last_result = x_regs[0];
                return true;
            }

            // Process.spawn(fn_name, arg1, ...) / cakra(fn_name, arg1, ...) / spawn(fn_name, arg1, ...)
            if (is_mod_fn(fn_name, "Process", "spawn") || is_mod_fn(fn_name, "Process", "cakra") ||
                fn_name == "spawn" || fn_name == "cakra") {
                std::string spawn_fn = clean_str(x_regs[0]);
                std::int64_t new_pid = g_next_pid++;
                auto actor = std::make_shared<GvmActor>();
                actor->pid = new_pid;
                actor->name = spawn_fn;

                {
                    std::lock_guard<std::mutex> lock(g_actor_mutex);
                    g_actor_map[new_pid] = actor;
                }

                std::vector<Term> call_args;
                for (std::size_t i = 1; i < x_regs.size(); ++i) {
                    if (!x_regs[i].is<std::monostate>()) {
                        call_args.push_back(x_regs[i]);
                    }
                }

                auto cg_out_copy = cg_out;
                std::thread([this, actor, spawn_fn, call_args, cg_out_copy]() {
                    t_current_actor = actor;
                    std::size_t handler_ip = std::string::npos;
                    for (const auto& [k, v] : cg_out_copy.function_table) {
                        if (k == spawn_fn || k.rfind("." + spawn_fn, 0) != std::string::npos || k.find(spawn_fn) != std::string::npos) {
                            handler_ip = v;
                            break;
                        }
                    }
                    if (handler_ip != std::string::npos) {
                        execute_instructions(cg_out_copy, handler_ip, call_args);
                    }
                    actor->is_alive = false;
                    actor->cv.notify_all();
                }).detach();

                x_regs[0] = Term(new_pid);
                last_result = x_regs[0];
                return true;
            }

            // Process.send(pid, msg) / kirim(pid, msg)
            if (is_mod_fn(fn_name, "Process", "send") || is_mod_fn(fn_name, "Process", "kirim") ||
                fn_name == "send" || fn_name == "kirim") {
                std::int64_t pid = get_int_val(x_regs[0]);
                Term msg = x_regs[1];

                std::shared_ptr<GvmActor> target_actor = nullptr;
                {
                    std::lock_guard<std::mutex> lock(g_actor_mutex);
                    auto it = g_actor_map.find(pid);
                    if (it != g_actor_map.end()) target_actor = it->second;
                }

                if (target_actor && target_actor->is_alive) {
                    {
                        std::lock_guard<std::mutex> lock(target_actor->mtx);
                        target_actor->mailbox.push_back(msg);
                    }
                    target_actor->cv.notify_one();
                    x_regs[0] = Term(Atom{"benar"});
                } else {
                    x_regs[0] = Term(Atom{"salah"});
                }
                last_result = x_regs[0];
                return true;
            }

            // Process.receive(timeout_ms) / tampa(timeout_ms) / receive(timeout_ms)
            if (is_mod_fn(fn_name, "Process", "receive") || is_mod_fn(fn_name, "Process", "tampa") ||
                fn_name == "receive" || fn_name == "tampa") {
                std::int64_t timeout_ms = (!x_regs[0].is<std::monostate>()) ? get_int_val(x_regs[0]) : 5000;
                if (timeout_ms <= 0) timeout_ms = 5000;

                Term rec_msg;
                bool got_msg = false;

                if (t_current_actor) {
                    std::unique_lock<std::mutex> lock(t_current_actor->mtx);
                    if (t_current_actor->cv.wait_for(lock, std::chrono::milliseconds(timeout_ms), [&]() {
                        return !t_current_actor->mailbox.empty();
                    })) {
                        rec_msg = t_current_actor->mailbox.front();
                        t_current_actor->mailbox.pop_front();
                        got_msg = true;
                    }
                }

                if (got_msg) {
                    x_regs[0] = rec_msg;
                } else {
                    x_regs[0] = Term(Atom{"nil"});
                }
                last_result = x_regs[0];
                return true;
            }

            // Process.self() / diri() / self()
            if (is_mod_fn(fn_name, "Process", "self") || fn_name == "self" || fn_name == "diri") {
                std::int64_t cur_pid = t_current_actor ? t_current_actor->pid : 100;
                x_regs[0] = Term(cur_pid);
                last_result = x_regs[0];
                return true;
            }

            // Process.is_alive(pid) / hidup(pid)
            if (is_mod_fn(fn_name, "Process", "is_alive") || is_mod_fn(fn_name, "Process", "hidup") ||
                fn_name == "is_alive" || fn_name == "hidup") {
                std::int64_t pid = get_int_val(x_regs[0]);
                bool alive = false;
                {
                    std::lock_guard<std::mutex> lock(g_actor_mutex);
                    auto it = g_actor_map.find(pid);
                    if (it != g_actor_map.end()) alive = it->second->is_alive;
                }
                x_regs[0] = Term(Atom{alive ? "benar" : "salah"});
                last_result = x_regs[0];
                return true;
            }

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

            if (is_mod_fn(fn_name, "List", "elemen") || is_mod_fn(fn_name, "List", "at")) {
                std::size_t idx = static_cast<std::size_t>(x_regs[1].get_if<int64_t>() ? *x_regs[1].get_if<int64_t>() : 0);
                int64_t sz = get_seq_size(x_regs[0]);
                if (idx < static_cast<std::size_t>(sz)) {
                    Term elem = get_seq_elem(x_regs[0], idx);
                    x_regs[0] = elem;
                    last_result = elem;
                    return true;
                }
                Term res(Atom{"nil"});
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
                if (g_http_server_running && g_http_server) {
                    g_http_server->stop();
                    if (g_http_server_thread.joinable()) {
                        g_http_server_thread.join();
                    }
                    g_http_server_running = false;
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

            if (is_mod_fn(fn_name, "HttpServer", "daftar_handler") || is_mod_fn(fn_name, "HttpServer", "register_handler")) {
                std::string nama = clean_str(x_regs[0]);
                std::string kode = clean_str(x_regs[1]);
                {
                    std::lock_guard<std::mutex> lock(g_handler_mutex);
                    g_handler_code[nama] = kode;
                    g_handler_is_param[nama] = false;
                }
                Term res(Atom{"ok"});
                x_regs[0] = res;
                last_result = res;
                return true;
            }

            if (is_mod_fn(fn_name, "HttpServer", "daftar_handler_param") || is_mod_fn(fn_name, "HttpServer", "register_handler_param")) {
                std::string nama = clean_str(x_regs[0]);
                std::string kode = clean_str(x_regs[1]);
                {
                    std::lock_guard<std::mutex> lock(g_handler_mutex);
                    g_handler_code[nama] = kode;
                    g_handler_is_param[nama] = true;
                }
                Term res(Atom{"ok"});
                x_regs[0] = res;
                last_result = res;
                return true;
            }

            if (is_mod_fn(fn_name, "HttpServer", "daftar_route") || is_mod_fn(fn_name, "HttpServer", "register_route")) {
                std::string method = clean_str(x_regs[0]);
                std::string path = clean_str(x_regs[1]);
                std::string handler = clean_str(x_regs[2]);
                {
                    std::lock_guard<std::mutex> lock(g_dynamic_routes_mutex);
                    g_dynamic_routes.push_back({method, path, handler});
                }
                Term res(Atom{"ok"});
                x_regs[0] = res;
                last_result = res;
                return true;
            }

            if (is_mod_fn(fn_name, "HttpServer", "jalankan") || is_mod_fn(fn_name, "HttpServer", "run")) {
                int64_t port = x_regs[0].get_if<int64_t>() ? *x_regs[0].get_if<int64_t>() : 8080;
                if (g_http_server_running && g_http_server) {
                    g_http_server->stop();
                    if (g_http_server_thread.joinable()) {
                        g_http_server_thread.join();
                    }
                    g_http_server_running = false;
                }
                g_http_server = std::make_shared<httplib::Server>();

                {
                    std::lock_guard<std::mutex> lock(g_dynamic_routes_mutex);
                    for (const auto& r : g_dynamic_routes) {
                        std::string h_name = r.handler_name;
                        auto execute_http_handler = [](const std::string& hname, const httplib::Request& req, httplib::Response& res) {
                            std::string code_to_run = "";
                            bool is_param = false;
                            {
                                std::lock_guard<std::mutex> lock(g_handler_mutex);
                                if (g_handler_code.count(hname)) {
                                    code_to_run = g_handler_code[hname];
                                }
                                if (g_handler_is_param.count(hname)) {
                                    is_param = g_handler_is_param[hname];
                                }
                            }
                            if (!code_to_run.empty()) {
                                std::lock_guard<std::mutex> lock(g_engine_mutex);
                                ScriptEngine h_engine;
                                std::optional<Term> eval_res;
                                if (is_param) {
                                    std::vector<Term> entries;
                                    for (const auto& p : req.params) {
                                        std::vector<Term> pair = { Term(p.first), Term(p.second) };
                                        entries.push_back(Term(Tuple{std::make_shared<const std::vector<Term>>(std::move(pair))}));
                                    }
                                    Term query_map(Tuple{std::make_shared<const std::vector<Term>>(std::move(entries))});
                                    std::vector<Term> initial_args = {
                                        query_map,
                                        Term(req.body),
                                        Term(req.method),
                                        Term(req.path)
                                    };
                                    eval_res = h_engine.eval(code_to_run, initial_args);
                                } else {
                                    eval_res = h_engine.eval(code_to_run);
                                }
                                if (eval_res) {
                                    std::string body;
                                    if (auto* str = eval_res->get_if<std::string>()) {
                                        body = *str;
                                    } else {
                                        body = garuda::beam::to_string(*eval_res);
                                    }
                                    res.set_content(body, "text/plain");
                                } else {
                                    res.set_content("error", "text/plain");
                                }
                            } else {
                                if (req.method == "POST" || req.method == "post") {
                                    res.set_content("Route POST " + req.body + " -> " + hname, "text/plain");
                                } else {
                                    res.set_content("Route " + req.method + " -> " + hname, "text/plain");
                                }
                            }
                        };

                        if (r.method == "GET" || r.method == "get") {
                            g_http_server->Get(r.path.c_str(), [h_name, execute_http_handler](const httplib::Request& req, httplib::Response& res) {
                                execute_http_handler(h_name, req, res);
                            });
                        } else if (r.method == "POST" || r.method == "post") {
                            g_http_server->Post(r.path.c_str(), [h_name, execute_http_handler](const httplib::Request& req, httplib::Response& res) {
                                execute_http_handler(h_name, req, res);
                            });
                        } else {
                            g_http_server->Get(r.path.c_str(), [h_name, execute_http_handler](const httplib::Request& req, httplib::Response& res) {
                                execute_http_handler(h_name, req, res);
                            });
                            g_http_server->Post(r.path.c_str(), [h_name, execute_http_handler](const httplib::Request& req, httplib::Response& res) {
                                execute_http_handler(h_name, req, res);
                            });
                        }
                    }
                }

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

            // Map.ukuran(M)
            if (is_mod_fn(fn_name, "Map", "ukuran") || is_mod_fn(fn_name, "Map", "size")) {
                int64_t sz = 0;
                if (auto* tup = x_regs[0].get_if<Tuple>()) {
                    if (tup->items) sz = static_cast<int64_t>(tup->items->size());
                }
                Term res(sz);
                x_regs[0] = res;
                last_result = res;
                return true;
            }

            // Map.mengandung_kunci(M, K)
            if (is_mod_fn(fn_name, "Map", "mengandung_kunci") || is_mod_fn(fn_name, "Map", "has_key")) {
                std::string k = clean_str(x_regs[1]);
                bool found = false;
                if (auto* tup = x_regs[0].get_if<Tuple>()) {
                    if (tup->items) {
                        for (const auto& item : *tup->items) {
                            if (auto* entry = item.get_if<Tuple>()) {
                                if (entry->items && entry->items->size() >= 2) {
                                    if (clean_str((*entry->items)[0]) == k) { found = true; break; }
                                }
                            }
                        }
                    }
                }
                Term res(Atom{found ? "true" : "false"});
                x_regs[0] = res;
                last_result = res;
                return true;
            }

            // Map.kosong(M)
            if (is_mod_fn(fn_name, "Map", "kosong") || is_mod_fn(fn_name, "Map", "empty")) {
                bool is_empty = true;
                if (auto* tup = x_regs[0].get_if<Tuple>()) {
                    if (tup->items && !tup->items->empty()) is_empty = false;
                }
                Term res(Atom{is_empty ? "true" : "false"});
                x_regs[0] = res;
                last_result = res;
                return true;
            }

            // Map.gabung(M1, M2)
            if (is_mod_fn(fn_name, "Map", "gabung") || is_mod_fn(fn_name, "Map", "merge")) {
                std::vector<Term> entries;
                std::set<std::string> seen;
                auto add_entries = [&](const Term& t) {
                    if (auto* tup = t.get_if<Tuple>()) {
                        if (tup->items) {
                            for (const auto& item : *tup->items) {
                                if (auto* entry = item.get_if<Tuple>()) {
                                    if (entry->items && entry->items->size() >= 2) {
                                        std::string k = clean_str((*entry->items)[0]);
                                        if (seen.insert(k).second) {
                                            entries.push_back(item);
                                        }
                                    }
                                }
                            }
                        }
                    }
                };
                add_entries(x_regs[1]);
                add_entries(x_regs[0]);
                Term res(Tuple{std::make_shared<const std::vector<Term>>(std::move(entries))});
                x_regs[0] = res;
                last_result = res;
                return true;
            }

            // Tuple Built-ins
            if (is_mod_fn(fn_name, "Tuple", "elemen") || is_mod_fn(fn_name, "Tuple", "get")) {
                int64_t idx = x_regs[1].get_if<int64_t>() ? *x_regs[1].get_if<int64_t>() : 0;
                Term result(Atom{"error_index"});
                if (auto* tup = x_regs[0].get_if<Tuple>()) {
                    if (tup->items && idx >= 0 && static_cast<std::size_t>(idx) < tup->items->size()) {
                        result = (*tup->items)[static_cast<std::size_t>(idx)];
                    }
                }
                x_regs[0] = result;
                last_result = result;
                return true;
            }

            if (is_mod_fn(fn_name, "Tuple", "panjang") || is_mod_fn(fn_name, "Tuple", "length")) {
                int64_t sz = 0;
                if (auto* tup = x_regs[0].get_if<Tuple>()) {
                    if (tup->items) sz = static_cast<int64_t>(tup->items->size());
                }
                Term res(sz);
                x_regs[0] = res;
                last_result = res;
                return true;
            }

            if (is_mod_fn(fn_name, "Tuple", "ke_list") || is_mod_fn(fn_name, "Tuple", "to_list")) {
                std::vector<Term> items;
                if (auto* tup = x_regs[0].get_if<Tuple>()) {
                    if (tup->items) {
                        for (const auto& item : *tup->items) items.push_back(item);
                    }
                }
                Term res(List{std::make_shared<const std::vector<Term>>(std::move(items))});
                x_regs[0] = res;
                last_result = res;
                return true;
            }

            if (is_mod_fn(fn_name, "Tuple", "gabung") || is_mod_fn(fn_name, "Tuple", "concat")) {
                std::vector<Term> items;
                auto add_items = [&](const Term& t) {
                    if (auto* tup = t.get_if<Tuple>()) {
                        if (tup->items) {
                            for (const auto& item : *tup->items) items.push_back(item);
                        }
                    }
                };
                add_items(x_regs[0]);
                add_items(x_regs[1]);
                Term res(Tuple{std::make_shared<const std::vector<Term>>(std::move(items))});
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
            if (is_fn(fn_name, "potong") || is_mod_fn(fn_name, "String", "potong")) {
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

            // Error Built-ins
            if (is_mod_fn(fn_name, "Error", "lempar") || is_mod_fn(fn_name, "Error", "throw")) {
                std::string msg = clean_str(x_regs[0]);
                Term err_term = x_regs[0];
                if (raise_error(err_term, msg)) return true;
                return true;
            }

            if (is_mod_fn(fn_name, "Error", "pesan") || is_mod_fn(fn_name, "Error", "message")) {
                std::string msg = clean_str(x_regs[0]);
                Term res(msg);
                x_regs[0] = res;
                last_result = res;
                return true;
            }

            if (is_mod_fn(fn_name, "Error", "jenis") || is_mod_fn(fn_name, "Error", "kind")) {
                std::string k = "error_user";
                if (auto* a = x_regs[0].get_if<Atom>()) k = a->name;
                Term res(Atom{k});
                x_regs[0] = res;
                last_result = res;
                return true;
            }

            // System Built-ins: File
            if (is_mod_fn(fn_name, "File", "baca") || is_mod_fn(fn_name, "File", "read")) {
                std::string path = clean_str(x_regs[0]);
                std::string content = Fs::read_file(path);
                Term res(content);
                x_regs[0] = res;
                last_result = res;
                return true;
            }

            if (is_mod_fn(fn_name, "File", "tulis") || is_mod_fn(fn_name, "File", "write")) {
                std::string path = clean_str(x_regs[0]);
                std::string content = clean_str(x_regs[1]);
                std::ofstream f(path);
                if (f.is_open()) {
                    f << content;
                    f.close();
                }
                Term res(Atom{"ok"});
                x_regs[0] = res;
                last_result = res;
                return true;
            }

            if (is_mod_fn(fn_name, "File", "ada") || is_mod_fn(fn_name, "File", "exists")) {
                std::string path = clean_str(x_regs[0]);
                bool exists = Fs::exists(path);
                Term res(Atom{exists ? "true" : "false"});
                x_regs[0] = res;
                last_result = res;
                return true;
            }

            // System Built-ins: Os
            if (is_mod_fn(fn_name, "Os", "args")) {
                std::vector<Term> items;
                Term res(List{std::make_shared<const std::vector<Term>>(std::move(items))});
                x_regs[0] = res;
                last_result = res;
                return true;
            }

            if (is_mod_fn(fn_name, "Os", "env")) {
                std::string var_name = clean_str(x_regs[0]);
                const char* val = std::getenv(var_name.c_str());
                std::string res_str = val ? val : "";
                Term res(res_str);
                x_regs[0] = res;
                last_result = res;
                return true;
            }

            if (is_mod_fn(fn_name, "Os", "exit")) {
                Term res(Atom{"nil"});
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
                case Opcode::OpTryBegin: {
                    if (!inst.args.empty() && inst.args[0].type == compiler::RegType::Literal) {
                        if (auto* ip_val = inst.args[0].literal.get_if<std::int64_t>()) {
                            try_stack.push_back({static_cast<std::size_t>(*ip_val), call_stack.size(), frame_stack.size()});
                        }
                    }
                    break;
                }
                case Opcode::OpTryEnd: {
                    if (!try_stack.empty()) {
                        try_stack.pop_back();
                    }
                    break;
                }
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

                        if (val_a.get_if<std::string>() || val_b.get_if<std::string>()) {
                            if (raise_error(Term(Atom{"error_type"}), "Type error: cannot add string with +")) continue;
                            return std::nullopt;
                        }

                        int64_t ia = get_int_val(val_a);
                        int64_t ib = get_int_val(val_b);
                        Term res(ia + ib);
                        x_regs[dst.index % 256] = res;
                        last_result = res;
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
                        if (val_b.get_if<std::int64_t>() && *val_b.get_if<std::int64_t>() == 0) {
                            if (raise_error(Term(Atom{"error_div"}), "Division by zero")) continue;
                            return std::nullopt;
                        }
                        if (val_a.get_if<std::int64_t>() && val_b.get_if<std::int64_t>()) {
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

                        if (!fn_name.empty() && handle_builtin(fn_name)) {
                            if (exception_raised) {
                                exception_raised = false;
                                continue;
                            }
                            break;
                        }

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

                        if (!fn_name.empty() && handle_builtin(fn_name)) {
                            if (exception_raised) {
                                exception_raised = false;
                                continue;
                            }
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
                                break;
                            } else {
                                x_regs[0] = ret_val;
                                return ret_val;
                            }
                        }

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
                    Term ret_val = !x_regs[0].is<std::monostate>() ? x_regs[0] : last_result;
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
                        x_regs[0] = ret_val;
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
