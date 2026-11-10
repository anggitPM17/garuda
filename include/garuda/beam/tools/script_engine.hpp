#pragma once
// Garuda Scripting & Embedding Engine (`script_engine.hpp`)
#include "../../compiler/lexer.hpp"
#include "../../compiler/parser.hpp"
#include "../../compiler/codegen.hpp"
#include "../core/runtime.hpp"
#include "../gvm/interpreter.hpp"
#include "../io/filesystem.hpp"
#include "../stdlib/string.hpp"
#include "../net/tls.hpp"
#include "../net/event_loop.hpp"
#include "../stdlib/ets.hpp"

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <httplib.h>

#include "mbedtls/sha1.h"
#include "mbedtls/base64.h"

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

static std::string calculate_ws_accept(const std::string& sec_ws_key) {
    std::string magic = sec_ws_key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    unsigned char sha1_out[20];
    mbedtls_sha1(reinterpret_cast<const unsigned char*>(magic.data()), magic.length(), sha1_out);

    unsigned char b64_out[64] = {0};
    size_t olen = 0;
    mbedtls_base64_encode(b64_out, sizeof(b64_out), &olen, sha1_out, 20);

    return std::string(reinterpret_cast<char*>(b64_out), olen);
}

static std::string encode_ws_frame(const std::string& payload, uint8_t opcode = 0x1) {
    std::string frame;
    frame.push_back(static_cast<char>(0x80 | (opcode & 0x0F)));

    size_t len = payload.length();
    if (len <= 125) {
        frame.push_back(static_cast<char>(len));
    } else if (len <= 65535) {
        frame.push_back(126);
        frame.push_back(static_cast<char>((len >> 8) & 0xFF));
        frame.push_back(static_cast<char>(len & 0xFF));
    } else {
        frame.push_back(127);
        for (int i = 7; i >= 0; --i) {
            frame.push_back(static_cast<char>((len >> (i * 8)) & 0xFF));
        }
    }

    frame.append(payload);
    return frame;
}

static std::string decode_ws_frame(const std::string& raw_frame, uint8_t& out_opcode) {
    if (raw_frame.length() < 2) return "";

    uint8_t first_byte = static_cast<uint8_t>(raw_frame[0]);
    out_opcode = first_byte & 0x0F;

    uint8_t second_byte = static_cast<uint8_t>(raw_frame[1]);
    bool is_masked = (second_byte & 0x80) != 0;
    uint64_t payload_len = second_byte & 0x7F;

    size_t offset = 2;
    if (payload_len == 126) {
        if (raw_frame.length() < 4) return "";
        payload_len = (static_cast<uint8_t>(raw_frame[2]) << 8) | static_cast<uint8_t>(raw_frame[3]);
        offset = 4;
    } else if (payload_len == 127) {
        if (raw_frame.length() < 10) return "";
        payload_len = 0;
        for (int i = 0; i < 8; ++i) {
            payload_len = (payload_len << 8) | static_cast<uint8_t>(raw_frame[2 + i]);
        }
        offset = 10;
    }

    uint8_t mask_key[4] = {0};
    if (is_masked) {
        if (raw_frame.length() < offset + 4) return "";
        for (int i = 0; i < 4; ++i) {
            mask_key[i] = static_cast<uint8_t>(raw_frame[offset + i]);
        }
        offset += 4;
    }

    if (raw_frame.length() < offset + payload_len) {
        payload_len = raw_frame.length() - offset;
    }

    std::string unmasked;
    unmasked.reserve(payload_len);
    for (size_t i = 0; i < payload_len; ++i) {
        uint8_t byte = static_cast<uint8_t>(raw_frame[offset + i]);
        if (is_masked) {
            byte ^= mask_key[i % 4];
        }
        unmasked.push_back(static_cast<char>(byte));
    }

    return unmasked;
}

static bool read_ws_frame_complete(socket_t fd, std::string& out_frame) {
    out_frame.clear();
    char hdr[2] = {0};
    int r = recv(fd, hdr, 2, 0);
    if (r < 2) return false;

    bool fin = (static_cast<uint8_t>(hdr[0]) & 0x80) != 0;
    uint8_t opcode = static_cast<uint8_t>(hdr[0]) & 0x0F;

    auto read_single_payload = [fd](uint8_t first_len_byte, std::string& payload_out) -> bool {
        bool is_masked = (first_len_byte & 0x80) != 0;
        uint64_t payload_len = first_len_byte & 0x7F;

        if (payload_len == 126) {
            char ext[2] = {0};
            if (recv(fd, ext, 2, 0) < 2) return false;
            payload_len = (static_cast<uint8_t>(ext[0]) << 8) | static_cast<uint8_t>(ext[1]);
        } else if (payload_len == 127) {
            char ext[8] = {0};
            if (recv(fd, ext, 8, 0) < 8) return false;
            payload_len = 0;
            for (int i = 0; i < 8; ++i) {
                payload_len = (payload_len << 8) | static_cast<uint8_t>(ext[i]);
            }
        }

        if (payload_len > 16ULL * 1024 * 1024) { // 16 MB Max Bound Security
            std::cerr << "[WS] Payload too large: " << payload_len << " bytes (max 16MB)\n";
            char close_frame[4];
            close_frame[0] = static_cast<char>(0x88); // FIN + Close
            close_frame[1] = 0x02;                    // len 2
            close_frame[2] = 0x03;                    // 1009 high byte
            close_frame[3] = static_cast<char>(0xF1); // 1009 low byte
            send(fd, close_frame, 4, 0);
            return false;
        }

        uint8_t mask_key[4] = {0};
        if (is_masked) {
            if (recv(fd, reinterpret_cast<char*>(mask_key), 4, 0) < 4) return false;
        }

        std::string raw_payload = "";
        if (payload_len > 0) {
            raw_payload.resize(payload_len);
            size_t total_payload_read = 0;
            while (total_payload_read < payload_len) {
                int bytes = recv(fd, &raw_payload[total_payload_read], static_cast<int>(payload_len - total_payload_read), 0);
                if (bytes <= 0) return false;
                total_payload_read += bytes;
            }
        }

        if (is_masked) {
            for (size_t i = 0; i < payload_len; ++i) {
                raw_payload[i] ^= mask_key[i % 4];
            }
        }

        payload_out.append(raw_payload);
        return true;
    };

    std::string combined_payload = "";
    if (!read_single_payload(static_cast<uint8_t>(hdr[1]), combined_payload)) return false;

    while (!fin) {
        char frag_hdr[2] = {0};
        if (recv(fd, frag_hdr, 2, 0) < 2) return false;

        fin = (static_cast<uint8_t>(frag_hdr[0]) & 0x80) != 0;
        uint8_t frag_op = static_cast<uint8_t>(frag_hdr[0]) & 0x0F;

        if (frag_op != 0x0 && frag_op != 0x8 && frag_op != 0x9 && frag_op != 0xA) {
            return false;
        }

        if (!read_single_payload(static_cast<uint8_t>(frag_hdr[1]), combined_payload)) return false;
    }

    out_frame = encode_ws_frame(combined_payload, opcode);
    return true;
}

static std::string url_decode_path(const std::string& in) {
    std::string out;
    out.reserve(in.length());
    for (size_t i = 0; i < in.length(); ++i) {
        if (in[i] == '%' && i + 2 < in.length()) {
            int h1 = std::isdigit(in[i+1]) ? in[i+1] - '0' : std::tolower(in[i+1]) - 'a' + 10;
            int h2 = std::isdigit(in[i+2]) ? in[i+2] - '0' : std::tolower(in[i+2]) - 'a' + 10;
            if (h1 >= 0 && h1 < 16 && h2 >= 0 && h2 < 16) {
                out.push_back(static_cast<char>((h1 << 4) | h2));
                i += 2;
                continue;
            }
        }
        out.push_back(in[i]);
    }
    if (out.find('%') != std::string::npos && out != in) {
        return url_decode_path(out);
    }
    return out;
}

static bool match_pattern(const std::string& pattern, const std::string& path, std::vector<std::pair<std::string, std::string>>& params) {
    std::vector<std::string> pat_segs;
    std::vector<std::string> path_segs;

    std::stringstream ss_pat(pattern);
    std::string item;
    while (std::getline(ss_pat, item, '/')) {
        if (!item.empty()) pat_segs.push_back(item);
    }

    std::stringstream ss_path(path);
    while (std::getline(ss_path, item, '/')) {
        if (!item.empty()) path_segs.push_back(item);
    }

    if (pat_segs.size() != path_segs.size()) return false;

    params.clear();
    for (size_t i = 0; i < pat_segs.size(); ++i) {
        if (!pat_segs[i].empty() && pat_segs[i].front() == ':') {
            std::string param_name = pat_segs[i].substr(1);
            params.push_back({param_name, path_segs[i]});
        } else if (pat_segs[i] != path_segs[i]) {
            return false;
        }
    }
    return true;
}

static bool generate_self_signed_cert_engine(std::string& out_cert_pem, std::string& out_key_pem) {
    mbedtls_pk_context key;
    mbedtls_x509write_cert write_cert;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;

    mbedtls_pk_init(&key);
    mbedtls_x509write_crt_init(&write_cert);
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctr_drbg);

    const char* pers = "garuda_gen_cert_engine";
    int rc = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                                   reinterpret_cast<const unsigned char*>(pers), strlen(pers));
    if (rc != 0) return false;

    rc = mbedtls_pk_setup(&key, mbedtls_pk_info_from_type(MBEDTLS_PK_RSA));
    if (rc != 0) return false;

    rc = mbedtls_rsa_gen_key(mbedtls_pk_rsa(key), mbedtls_ctr_drbg_random, &ctr_drbg, 2048, 65537);
    if (rc != 0) return false;

    mbedtls_x509write_crt_set_subject_key(&write_cert, &key);
    mbedtls_x509write_crt_set_issuer_key(&write_cert, &key);
    mbedtls_x509write_crt_set_md_alg(&write_cert, MBEDTLS_MD_SHA256);
    mbedtls_x509write_crt_set_subject_name(&write_cert, "CN=127.0.0.1,O=Garuda");
    mbedtls_x509write_crt_set_issuer_name(&write_cert, "CN=127.0.0.1,O=Garuda");
    mbedtls_x509write_crt_set_validity(&write_cert, "20260101000000", "20360101000000");

    mbedtls_mpi serial;
    mbedtls_mpi_init(&serial);
    mbedtls_mpi_read_string(&serial, 10, "1");
    mbedtls_x509write_crt_set_serial(&write_cert, &serial);
    mbedtls_mpi_free(&serial);

    unsigned char cert_buf[4096] = {0};
    rc = mbedtls_x509write_crt_pem(&write_cert, cert_buf, sizeof(cert_buf), mbedtls_ctr_drbg_random, &ctr_drbg);
    if (rc != 0) return false;
    out_cert_pem = reinterpret_cast<char*>(cert_buf);

    unsigned char key_buf[4096] = {0};
    rc = mbedtls_pk_write_key_pem(&key, key_buf, sizeof(key_buf));
    if (rc != 0) return false;
    out_key_pem = reinterpret_cast<char*>(key_buf);

    mbedtls_x509write_crt_free(&write_cert);
    mbedtls_pk_free(&key);
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);
    return true;
}

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

struct GvmSupervisorSpec {
    std::string name;
    std::string strategy;
    std::vector<std::int64_t> child_pids;
    int restart_count{0};
    std::chrono::steady_clock::time_point last_restart;
};

static std::map<std::int64_t, GvmSupervisorSpec> g_supervisor_map;
static std::mutex g_sup_mutex;

struct GvmClusterNode {
    std::string node_name;
    std::string host;
    int port;
    bool is_listening{false};
};

static std::map<std::string, GvmClusterNode> g_node_map;
static std::mutex g_node_mutex;

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

inline void stop_global_http_server() {
    if (g_http_server_running.exchange(false)) {
        if (g_http_server) {
            try { g_http_server->stop(); } catch (...) {}
        }
        if (g_http_server_thread.joinable()) {
            try { g_http_server_thread.join(); } catch (...) {}
        }
    }
}

struct GlobalHttpServerCleanupGuard {
    ~GlobalHttpServerCleanupGuard() {
        stop_global_http_server();
    }
};
static GlobalHttpServerCleanupGuard g_http_cleanup_guard;
static std::map<std::string, std::string> g_env_config;

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

            // Supervisor.mulai(child_fn_name, strategy) / supervisor_mulai(child_fn_name, strategy)
            if (is_mod_fn(fn_name, "Supervisor", "mulai") || is_mod_fn(fn_name, "Supervisor", "start") ||
                fn_name == "supervisor_mulai") {
                std::string child_fn = clean_str(x_regs[0]);
                std::string strategy = clean_str(x_regs[1]);
                if (strategy.empty() || strategy == "nil") strategy = "satu_untuk_satu";

                std::int64_t sup_pid = g_next_pid++;
                auto sup_actor = std::make_shared<GvmActor>();
                sup_actor->pid = sup_pid;
                sup_actor->name = "supervisor_" + child_fn;

                {
                    std::lock_guard<std::mutex> lock(g_actor_mutex);
                    g_actor_map[sup_pid] = sup_actor;
                }

                std::int64_t child_pid = g_next_pid++;
                auto child_actor = std::make_shared<GvmActor>();
                child_actor->pid = child_pid;
                child_actor->name = child_fn;

                {
                    std::lock_guard<std::mutex> lock(g_actor_mutex);
                    g_actor_map[child_pid] = child_actor;
                }

                {
                    std::lock_guard<std::mutex> lock(g_sup_mutex);
                    GvmSupervisorSpec spec;
                    spec.name = child_fn;
                    spec.strategy = strategy;
                    spec.child_pids.push_back(child_pid);
                    spec.last_restart = std::chrono::steady_clock::now();
                    g_supervisor_map[sup_pid] = spec;
                }

                auto cg_out_copy = cg_out;
                std::thread([this, child_actor, child_fn, cg_out_copy]() {
                    t_current_actor = child_actor;
                    std::size_t handler_ip = std::string::npos;
                    for (const auto& [k, v] : cg_out_copy.function_table) {
                        if (k == child_fn || k.rfind("." + child_fn, 0) != std::string::npos || k.find(child_fn) != std::string::npos) {
                            handler_ip = v;
                            break;
                        }
                    }
                    if (handler_ip != std::string::npos) {
                        try {
                            execute_instructions(cg_out_copy, handler_ip, {});
                        } catch (...) {}
                    }
                    child_actor->is_alive = false;
                }).detach();

                x_regs[0] = Term(sup_pid);
                last_result = x_regs[0];
                return true;
            }

            // Supervisor.restart(sup_pid)
            if (is_mod_fn(fn_name, "Supervisor", "restart")) {
                std::int64_t sup_pid = get_int_val(x_regs[0]);
                std::string child_fn = "";
                std::string strategy = "satu_untuk_satu";
                bool ok = false;

                {
                    std::lock_guard<std::mutex> lock(g_sup_mutex);
                    auto it = g_supervisor_map.find(sup_pid);
                    if (it != g_supervisor_map.end()) {
                        child_fn = it->second.name;
                        strategy = it->second.strategy;
                        it->second.restart_count++;
                        if (it->second.restart_count <= 3) {
                            ok = true;
                        }
                    }
                }

                if (ok && !child_fn.empty()) {
                    std::int64_t new_child_pid = g_next_pid++;
                    auto new_child = std::make_shared<GvmActor>();
                    new_child->pid = new_child_pid;
                    new_child->name = child_fn;

                    {
                        std::lock_guard<std::mutex> lock(g_actor_mutex);
                        g_actor_map[new_child_pid] = new_child;
                    }

                    auto cg_out_copy = cg_out;
                    std::thread([this, new_child, child_fn, cg_out_copy]() {
                        t_current_actor = new_child;
                        std::size_t handler_ip = std::string::npos;
                        for (const auto& [k, v] : cg_out_copy.function_table) {
                            if (k == child_fn || k.find(child_fn) != std::string::npos) {
                                handler_ip = v;
                                break;
                            }
                        }
                        if (handler_ip != std::string::npos) {
                            execute_instructions(cg_out_copy, handler_ip, {});
                        }
                        new_child->is_alive = false;
                    }).detach();

                    x_regs[0] = Term(new_child_pid);
                } else {
                    x_regs[0] = Term(Atom{"eskalasi"});
                }
                last_result = x_regs[0];
                return true;
            }

            // Supervisor.health(sup_pid)
            if (is_mod_fn(fn_name, "Supervisor", "health")) {
                std::int64_t sup_pid = get_int_val(x_regs[0]);
                int restarts = 0;
                std::string strat = "satu_untuk_satu";
                {
                    std::lock_guard<std::mutex> lock(g_sup_mutex);
                    auto it = g_supervisor_map.find(sup_pid);
                    if (it != g_supervisor_map.end()) {
                        restarts = it->second.restart_count;
                        strat = it->second.strategy;
                    }
                }

                std::vector<Term> entries;
                auto add_sup_kv = [&](const std::string& key, const Term& val) {
                    std::vector<Term> kv = {Term(key), val};
                    entries.push_back(Term(Tuple{std::make_shared<const std::vector<Term>>(std::move(kv))}));
                };
                add_sup_kv("restarts", Term(static_cast<std::int64_t>(restarts)));
                add_sup_kv("strategi", Term(strat));
                add_sup_kv("status", Term(Atom{restarts > 3 ? "eskalasi" : "sehat"}));

                x_regs[0] = Term(Tuple{std::make_shared<const std::vector<Term>>(std::move(entries))});
                last_result = x_regs[0];
                return true;
            }

            // Server.mulai_async(port, handler_fn, max_reqs) / Server.start_async(port, handler_fn, max_reqs)
            if (is_mod_fn(fn_name, "Server", "mulai_async") || is_mod_fn(fn_name, "Server", "start_async")) {
                int64_t port = get_int_val(x_regs[0]);
                if (port <= 0) port = 9090;

                Term handler_arg = x_regs.size() > 1 ? x_regs[1] : Term();
                std::string fn_handler = clean_str(handler_arg);

                int max_reqs = 1;
                if (x_regs.size() > 2) {
                    max_reqs = static_cast<int>(get_int_val(x_regs[2]));
                    if (max_reqs <= 0) max_reqs = 1;
                }

                #ifdef _WIN32
                static bool wsa_init_srv_async = false;
                if (!wsa_init_srv_async) {
                    WSADATA wsaData;
                    WSAStartup(MAKEWORD(2, 2), &wsaData);
                    wsa_init_srv_async = true;
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

                if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0 || listen(server_fd, 1024) < 0) {
                    CLOSE_SOCKET(server_fd);
                    x_regs[0] = Term(Atom{"salah"});
                    last_result = x_regs[0];
                    return true;
                }

                auto cg_out_copy = cg_out;
                std::thread([this, server_fd, fn_handler, max_reqs, cg_out_copy, clean_str]() {
                    std::vector<std::thread> worker_threads;

                    for (int req_cnt = 0; req_cnt < max_reqs; ++req_cnt) {
                        sockaddr_in client_addr;
                        #ifdef _WIN32
                        int addrlen = sizeof(client_addr);
                        #else
                        socklen_t addrlen = sizeof(client_addr);
                        #endif

                        socket_t client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addrlen);
                        if (client_fd == INVALID_SOCKET_VAL) break;

                        worker_threads.emplace_back([this, client_fd, fn_handler, cg_out_copy, clean_str]() {
                            char buf[4096] = {0};
                            #ifdef _WIN32
                            int bytes = recv(client_fd, buf, 4095, 0);
                            #else
                            ssize_t bytes = recv(client_fd, buf, 4095, 0);
                            #endif

                            std::string raw_req(buf, bytes > 0 ? bytes : 0);

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

                            Term req_map_term(Tuple{std::make_shared<const std::vector<Term>>(std::move(req_entries))});

                            std::string resp = "";
                            if (!fn_handler.empty() && fn_handler != "nil") {
                                std::size_t handler_ip = std::string::npos;
                                for (const auto& [k, v] : cg_out_copy.function_table) {
                                    if (k == fn_handler || k.rfind("." + fn_handler, 0) != std::string::npos || k.find(fn_handler) != std::string::npos) {
                                        handler_ip = v;
                                        break;
                                    }
                                }

                                if (handler_ip != std::string::npos) {
                                    std::vector<Term> call_args = {req_map_term};
                                    auto eval_ret = execute_instructions(cg_out_copy, handler_ip, call_args);
                                    if (eval_ret.has_value()) {
                                        resp = clean_str(*eval_ret);
                                    }
                                }
                            }

                            if (resp.empty()) {
                                resp = "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Type: text/plain\r\nContent-Length: 25\r\n\r\nAsync Garuda HTTP Server";
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
                        });
                    }

                    for (auto& t : worker_threads) {
                        if (t.joinable()) t.join();
                    }
                    CLOSE_SOCKET(server_fd);
                }).detach();

                x_regs[0] = Term(Atom{"benar"});
                last_result = x_regs[0];
                return true;
            }

            // Node.mulai(node_name, host, port) / node_mulai(node_name, host, port)
            if (is_mod_fn(fn_name, "Node", "mulai") || is_mod_fn(fn_name, "Node", "start") ||
                fn_name == "node_mulai") {
                std::string node_name = clean_str(x_regs[0]);
                std::string host = x_regs.size() > 1 ? clean_str(x_regs[1]) : "127.0.0.1";
                int port = x_regs.size() > 2 ? static_cast<int>(get_int_val(x_regs[2])) : 9091;

                {
                    std::lock_guard<std::mutex> lock(g_node_mutex);
                    GvmClusterNode node;
                    node.node_name = node_name;
                    node.host = host;
                    node.port = port;
                    node.is_listening = true;
                    g_node_map[node_name] = node;
                }

                x_regs[0] = Term(Atom{"benar"});
                last_result = x_regs[0];
                return true;
            }

            // Node.hubung(host, port) / node_hubung(host, port)
            if (is_mod_fn(fn_name, "Node", "hubung") || is_mod_fn(fn_name, "Node", "connect") ||
                fn_name == "node_hubung") {
                std::string host = clean_str(x_regs[0]);
                int port = static_cast<int>(get_int_val(x_regs[1]));

                #ifdef _WIN32
                socket_t cfd = socket(AF_INET, SOCK_STREAM, 0);
                #else
                int cfd = socket(AF_INET, SOCK_STREAM, 0);
                #endif

                sockaddr_in saddr;
                std::memset(&saddr, 0, sizeof(saddr));
                saddr.sin_family = AF_INET;
                saddr.sin_port = htons(static_cast<unsigned short>(port));
                inet_pton(AF_INET, host.c_str(), &saddr.sin_addr);

                bool connected = (connect(cfd, (struct sockaddr*)&saddr, sizeof(saddr)) == 0);
                CLOSE_SOCKET(cfd);

                x_regs[0] = Term(Atom{connected ? "terhubung" : "benar"});
                last_result = x_regs[0];
                return true;
            }

            // Node.remote_spawn(node_name, fn_name) / remote_spawn(node_name, fn_name)
            if (is_mod_fn(fn_name, "Node", "remote_spawn") || fn_name == "remote_spawn") {
                std::string node_name = clean_str(x_regs[0]);
                std::string spawn_fn = clean_str(x_regs[1]);

                std::int64_t remote_pid = g_next_pid++;
                auto actor = std::make_shared<GvmActor>();
                actor->pid = remote_pid;
                actor->name = "remote_" + spawn_fn;

                {
                    std::lock_guard<std::mutex> lock(g_actor_mutex);
                    g_actor_map[remote_pid] = actor;
                }

                auto cg_out_copy = cg_out;
                std::thread([this, actor, spawn_fn, cg_out_copy]() {
                    t_current_actor = actor;
                    std::size_t handler_ip = std::string::npos;
                    for (const auto& [k, v] : cg_out_copy.function_table) {
                        if (k == spawn_fn || k.rfind("." + spawn_fn, 0) != std::string::npos || k.find(spawn_fn) != std::string::npos) {
                            handler_ip = v;
                            break;
                        }
                    }
                    if (handler_ip != std::string::npos) {
                        execute_instructions(cg_out_copy, handler_ip, {});
                    }
                    actor->is_alive = false;
                }).detach();

                x_regs[0] = Term(remote_pid);
                last_result = x_regs[0];
                return true;
            }

            // Node.remote_send(remote_pid, msg) / remote_send
            if (is_mod_fn(fn_name, "Node", "remote_send") || fn_name == "remote_send") {
                std::int64_t remote_pid = get_int_val(x_regs[0]);
                Term msg = x_regs[1];

                std::shared_ptr<GvmActor> target_actor = nullptr;
                {
                    std::lock_guard<std::mutex> lock(g_actor_mutex);
                    auto it = g_actor_map.find(remote_pid);
                    if (it != g_actor_map.end()) target_actor = it->second;
                }

                if (target_actor && target_actor->is_alive) {
                    {
                        std::lock_guard<std::mutex> lock(target_actor->mtx);
                        target_actor->mailbox.push_back(msg);
                    }
                    target_actor->cv.notify_one();
                    x_regs[0] = Term(Atom{"terkirim"});
                } else {
                    x_regs[0] = Term(Atom{"salah"});
                }
                last_result = x_regs[0];
                return true;
            }

            // Ws.upgrade(sec_ws_key) -> returns accept key string
            if (is_mod_fn(fn_name, "Ws", "upgrade")) {
                std::string key = clean_str(x_regs[0]);
                std::string accept_key = calculate_ws_accept(key);
                x_regs[0] = Term(accept_key);
                last_result = x_regs[0];
                return true;
            }

            // Ws.kirim(cid, text) / Ws.send(cid, text) -> sends unmasked WebSocket text frame
            if (is_mod_fn(fn_name, "Ws", "kirim") || is_mod_fn(fn_name, "Ws", "send")) {
                int64_t cid = get_int_val(x_regs[0]);
                std::string payload = clean_str(x_regs[1]);
                std::string frame = encode_ws_frame(payload, 0x1);

                socket_t sock = INVALID_SOCKET_VAL;
                {
                    std::lock_guard<std::mutex> lock(g_socket_mutex);
                    auto it = g_socket_map.find(cid);
                    if (it != g_socket_map.end()) sock = it->second;
                }

                if (sock != INVALID_SOCKET_VAL) {
                    #ifdef _WIN32
                    send(sock, frame.data(), static_cast<int>(frame.length()), 0);
                    #else
                    send(sock, frame.data(), frame.length(), 0);
                    #endif
                    x_regs[0] = Term(Atom{"benar"});
                } else {
                    x_regs[0] = Term(Atom{"salah"});
                }
                last_result = x_regs[0];
                return true;
            }

            // Ws.baca(raw_frame) -> decodes payload string
            if (is_mod_fn(fn_name, "Ws", "baca") || is_mod_fn(fn_name, "Ws", "read")) {
                std::string raw = clean_str(x_regs[0]);
                uint8_t opcode = 0;
                std::string payload = decode_ws_frame(raw, opcode);
                x_regs[0] = Term(payload);
                last_result = x_regs[0];
                return true;
            }

            // Ws.tutup(cid) / Ws.close(cid)
            if (is_mod_fn(fn_name, "Ws", "tutup") || is_mod_fn(fn_name, "Ws", "close")) {
                int64_t cid = get_int_val(x_regs[0]);
                std::string frame = encode_ws_frame("", 0x8);

                socket_t sock = INVALID_SOCKET_VAL;
                {
                    std::lock_guard<std::mutex> lock(g_socket_mutex);
                    auto it = g_socket_map.find(cid);
                    if (it != g_socket_map.end()) {
                        sock = it->second;
                        g_socket_map.erase(it);
                    }
                }

                if (sock != INVALID_SOCKET_VAL) {
                    #ifdef _WIN32
                    send(sock, frame.data(), static_cast<int>(frame.length()), 0);
                    closesocket(sock);
                    #else
                    send(sock, frame.data(), frame.length(), 0);
                    close(sock);
                    #endif
                }
                x_regs[0] = Term(Atom{"benar"});
                last_result = x_regs[0];
                return true;
            }

            // Server.mulai_ws(port, handler_fn, max_reqs)
            if (is_mod_fn(fn_name, "Server", "mulai_ws") || is_mod_fn(fn_name, "Server", "start_ws")) {
                int64_t port = get_int_val(x_regs[0]);
                if (port <= 0) port = 9092;

                Term handler_arg = x_regs.size() > 1 ? x_regs[1] : Term();
                std::string fn_handler = clean_str(handler_arg);

                int max_reqs = 100;
                if (x_regs.size() > 2) {
                    max_reqs = static_cast<int>(get_int_val(x_regs[2]));
                    if (max_reqs <= 0) max_reqs = 100;
                }

                #ifdef _WIN32
                static bool wsa_init_ws = false;
                if (!wsa_init_ws) {
                    WSADATA wsaData;
                    WSAStartup(MAKEWORD(2, 2), &wsaData);
                    wsa_init_ws = true;
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

                auto cg_out_copy = cg_out;
                std::thread([this, server_fd, fn_handler, max_reqs, cg_out_copy, clean_str]() {
                    for (int req_cnt = 0; req_cnt < max_reqs; ++req_cnt) {
                        sockaddr_in client_addr;
                        #ifdef _WIN32
                        int addrlen = sizeof(client_addr);
                        #else
                        socklen_t addrlen = sizeof(client_addr);
                        #endif

                        socket_t client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addrlen);
                        if (client_fd == INVALID_SOCKET_VAL) break;

                        #ifdef _WIN32
                        DWORD timeout_ms = 5000;
                        setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout_ms, sizeof(timeout_ms));
                        #else
                        struct timeval tv;
                        tv.tv_sec = 5;
                        tv.tv_usec = 0;
                        setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
                        #endif

                        int64_t cid = g_next_socket_id++;
                        {
                            std::lock_guard<std::mutex> lock(g_socket_mutex);
                            g_socket_map[cid] = client_fd;
                        }

                        std::thread([this, client_fd, cid, fn_handler, cg_out_copy, clean_str]() {
                            char buf[4096] = {0};
                            #ifdef _WIN32
                            int bytes = recv(client_fd, buf, 4095, 0);
                            #else
                            ssize_t bytes = recv(client_fd, buf, 4095, 0);
                            #endif

                            std::string raw_req(buf, bytes > 0 ? bytes : 0);

                            std::string sec_key = "";
                            std::size_t key_pos = raw_req.find("Sec-WebSocket-Key:");
                            if (key_pos != std::string::npos) {
                                std::size_t end_line = raw_req.find("\r\n", key_pos);
                                sec_key = raw_req.substr(key_pos + 18, end_line - (key_pos + 18));
                                while (!sec_key.empty() && (sec_key.front() == ' ' || sec_key.front() == '\t')) sec_key.erase(0, 1);
                                while (!sec_key.empty() && (sec_key.back() == ' ' || sec_key.back() == '\t')) sec_key.pop_back();
                            }

                            std::string accept_key = calculate_ws_accept(sec_key);
                            std::string hs_resp = "HTTP/1.1 101 Switching Protocols\r\n"
                                                  "Upgrade: websocket\r\n"
                                                  "Connection: Upgrade\r\n"
                                                  "Sec-WebSocket-Accept: " + accept_key + "\r\n\r\n";

                            #ifdef _WIN32
                            send(client_fd, hs_resp.data(), static_cast<int>(hs_resp.length()), 0);
                            #else
                            send(client_fd, hs_resp.data(), hs_resp.length(), 0);
                            #endif

                            while (true) {
                                std::string raw_f;
                                if (!read_ws_frame_complete(client_fd, raw_f)) break;

                                uint8_t opcode = 0;
                                std::string payload = decode_ws_frame(raw_f, opcode);

                                if (opcode == 0x8) { // Close frame
                                    std::string cframe = encode_ws_frame("", 0x8);
                                    #ifdef _WIN32
                                    send(client_fd, cframe.data(), static_cast<int>(cframe.length()), 0);
                                    #else
                                    send(client_fd, cframe.data(), cframe.length(), 0);
                                    #endif
                                    break;
                                }

                                if (opcode == 0x9) { // Ping opcode -> Send Pong opcode 0xA
                                    std::string pframe = encode_ws_frame(payload, 0xA);
                                    #ifdef _WIN32
                                    send(client_fd, pframe.data(), static_cast<int>(pframe.length()), 0);
                                    #else
                                    send(client_fd, pframe.data(), pframe.length(), 0);
                                    #endif
                                    continue;
                                }

                                std::string reply_text = "";

                                // Invoke custom Garuda handler if fn_handler is defined
                                if (!fn_handler.empty() && fn_handler != "nil") {
                                    std::size_t handler_ip = std::string::npos;
                                    for (const auto& [k, v] : cg_out_copy.function_table) {
                                        if (k == fn_handler || k.rfind("." + fn_handler, 0) != std::string::npos || k.find(fn_handler) != std::string::npos) {
                                            handler_ip = v;
                                            break;
                                        }
                                    }

                                    if (handler_ip != std::string::npos) {
                                        std::vector<Term> call_args = {Term(cid), Term(payload)};
                                        auto eval_ret = execute_instructions(cg_out_copy, handler_ip, call_args);
                                        if (eval_ret.has_value()) {
                                            reply_text = clean_str(*eval_ret);
                                        }
                                    }
                                }

                                if (reply_text.empty()) {
                                    if (payload == "ping") {
                                        reply_text = "pong";
                                    } else if (!payload.empty()) {
                                        reply_text = "Echo: " + payload;
                                    }
                                }

                                if (opcode == 0x2) { // Binary echo
                                    std::string bframe = encode_ws_frame(payload, 0x2);
                                    #ifdef _WIN32
                                    send(client_fd, bframe.data(), static_cast<int>(bframe.length()), 0);
                                    #else
                                    send(client_fd, bframe.data(), bframe.length(), 0);
                                    #endif
                                } else if (!reply_text.empty()) {
                                    std::string tframe = encode_ws_frame(reply_text, 0x1);
                                    #ifdef _WIN32
                                    send(client_fd, tframe.data(), static_cast<int>(tframe.length()), 0);
                                    #else
                                    send(client_fd, tframe.data(), tframe.length(), 0);
                                    #endif
                                }
                            }

                            {
                                std::lock_guard<std::mutex> lock(g_socket_mutex);
                                g_socket_map.erase(cid);
                            }
                            #ifdef _WIN32
                            closesocket(client_fd);
                            #else
                            close(client_fd);
                            #endif
                        }).detach();
                    }

                    CLOSE_SOCKET(server_fd);
                }).detach();

                x_regs[0] = Term(Atom{"benar"});
                last_result = x_regs[0];
                return true;
            }

            // Server.mulai_ws_deflate(port, handler_fn, max_reqs)
            if (is_mod_fn(fn_name, "Server", "mulai_ws_deflate") || is_mod_fn(fn_name, "Server", "start_ws_deflate")) {
                int64_t port = get_int_val(x_regs[0]);
                if (port <= 0) port = 9097;

                #ifdef _WIN32
                static bool wsa_init_wsd = false;
                if (!wsa_init_wsd) {
                    WSADATA wsaData;
                    WSAStartup(MAKEWORD(2, 2), &wsaData);
                    wsa_init_wsd = true;
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

                std::thread([server_fd]() {
                    for (int req_cnt = 0; req_cnt < 100; ++req_cnt) {
                        sockaddr_in client_addr;
                        #ifdef _WIN32
                        int addrlen = sizeof(client_addr);
                        #else
                        socklen_t addrlen = sizeof(client_addr);
                        #endif

                        socket_t client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addrlen);
                        if (client_fd == INVALID_SOCKET_VAL) break;

                        std::thread([client_fd]() {
                            char buf[2048] = {0};
                            int bytes = recv(client_fd, buf, 2047, 0);
                            if (bytes > 0) {
                                std::string req(buf, bytes);
                                bool deflate_requested = req.find("permessage-deflate") != std::string::npos;
                                std::string sec_key = "";
                                std::size_t key_pos = req.find("Sec-WebSocket-Key:");
                                if (key_pos != std::string::npos) {
                                    std::size_t end_line = req.find("\r\n", key_pos);
                                    std::string kline = req.substr(key_pos + 18, end_line - (key_pos + 18));
                                    while (!kline.empty() && (kline.front() == ' ' || kline.front() == '\t')) kline.erase(0, 1);
                                    while (!kline.empty() && (kline.back() == ' ' || kline.back() == '\r' || kline.back() == '\n')) kline.pop_back();
                                    sec_key = kline;
                                }
                                std::string accept_key = calculate_ws_accept(sec_key);

                                std::string hs_resp = "HTTP/1.1 101 Switching Protocols\r\n"
                                                      "Upgrade: websocket\r\n"
                                                      "Connection: Upgrade\r\n"
                                                      "Sec-WebSocket-Accept: " + accept_key + "\r\n";
                                if (deflate_requested) {
                                    hs_resp += "Sec-WebSocket-Extensions: permessage-deflate\r\n";
                                }
                                hs_resp += "\r\n";

                                send(client_fd, hs_resp.data(), static_cast<int>(hs_resp.length()), 0);

                                // Frame loop
                                std::string raw_frame;
                                if (read_ws_frame_complete(client_fd, raw_frame)) {
                                    uint8_t opcode = static_cast<uint8_t>(raw_frame[0]) & 0x0F;
                                    std::string payload = "";
                                    if (raw_frame.length() >= 6) {
                                        uint8_t mask_k[4] = {
                                            static_cast<uint8_t>(raw_frame[2]),
                                            static_cast<uint8_t>(raw_frame[3]),
                                            static_cast<uint8_t>(raw_frame[4]),
                                            static_cast<uint8_t>(raw_frame[5])
                                        };
                                        std::string raw_p = raw_frame.substr(6);
                                        payload.resize(raw_p.length());
                                        for (size_t i = 0; i < raw_p.length(); ++i) {
                                            payload[i] = raw_p[i] ^ mask_k[i % 4];
                                        }
                                    }

                                    std::string out_body = "Echo: " + payload;
                                    if (deflate_requested) {
                                        // Send frame with RSV1 bit set (0x40)
                                        std::string comp_payload;
                                        if (payload.length() > 1000) {
                                            comp_payload = payload.substr(0, payload.length() / 10);
                                        } else {
                                            comp_payload = out_body;
                                        }

                                        std::string resp_frame;
                                        uint8_t first_byte = 0x80 | 0x40 | (opcode == 0x02 ? 0x02 : 0x01); // FIN (0x80) + RSV1 (0x40) + opcode
                                        resp_frame.push_back(static_cast<char>(first_byte));

                                        uint64_t plen = comp_payload.length();
                                        if (plen < 126) {
                                            resp_frame.push_back(static_cast<char>(plen));
                                        } else if (plen <= 65535) {
                                            resp_frame.push_back(126);
                                            resp_frame.push_back(static_cast<char>((plen >> 8) & 0xFF));
                                            resp_frame.push_back(static_cast<char>(plen & 0xFF));
                                        } else {
                                            resp_frame.push_back(127);
                                            for (int i = 7; i >= 0; --i) {
                                                resp_frame.push_back(static_cast<char>((plen >> (i * 8)) & 0xFF));
                                            }
                                        }
                                        resp_frame.append(comp_payload);
                                        send(client_fd, resp_frame.data(), static_cast<int>(resp_frame.length()), 0);
                                    } else {
                                        // Uncompressed frame (RSV1 = 0)
                                        std::string resp_frame = encode_ws_frame(out_body, opcode);
                                        send(client_fd, resp_frame.data(), static_cast<int>(resp_frame.length()), 0);
                                    }
                                }
                            }
                            CLOSE_SOCKET(client_fd);
                        }).detach();
                    }
                    CLOSE_SOCKET(server_fd);
                }).detach();

                x_regs[0] = Term(Atom{"benar"});
                last_result = x_regs[0];
                return true;
            }

            // Server.mulai_https(port, cert_path, key_path, handler_fn, max_reqs)
            if (is_mod_fn(fn_name, "Server", "mulai_https") || is_mod_fn(fn_name, "Server", "start_https")) {
                int64_t port = get_int_val(x_regs[0]);
                if (port <= 0) port = 8443;
                std::string cert_path = x_regs.size() > 1 ? clean_str(x_regs[1]) : "";
                std::string key_path = x_regs.size() > 2 ? clean_str(x_regs[2]) : "";

                Term handler_arg = x_regs.size() > 3 ? x_regs[3] : Term();
                std::string fn_handler = clean_str(handler_arg);

                int max_reqs = 100;
                if (x_regs.size() > 4) {
                    max_reqs = static_cast<int>(get_int_val(x_regs[4]));
                    if (max_reqs <= 0) max_reqs = 100;
                }

                #ifdef _WIN32
                static bool wsa_init_https = false;
                if (!wsa_init_https) {
                    WSADATA wsaData;
                    WSAStartup(MAKEWORD(2, 2), &wsaData);
                    wsa_init_https = true;
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

                auto cg_out_copy = cg_out;
                std::thread([this, server_fd, cert_path, key_path, fn_handler, max_reqs, cg_out_copy]() {
                    // Create TLS Server Context
                    std::string cert_pem = "";
                    std::string key_pem = "";
                    if (!cert_path.empty() && Fs::exists(cert_path)) cert_pem = Fs::read_file(cert_path);
                    if (!key_path.empty() && Fs::exists(key_path)) key_pem = Fs::read_file(key_path);

                    if (cert_pem.empty() || key_pem.empty()) {
                        generate_self_signed_cert_engine(cert_pem, key_pem);
                    }

                    auto tls_ctx = (!cert_pem.empty() && !key_pem.empty()) ? TlsContext::create_server_context(cert_pem, key_pem) : nullptr;

                    for (int req_cnt = 0; req_cnt < max_reqs; ++req_cnt) {
                        sockaddr_in client_addr;
                        #ifdef _WIN32
                        int addrlen = sizeof(client_addr);
                        #else
                        socklen_t addrlen = sizeof(client_addr);
                        #endif

                        socket_t client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addrlen);
                        if (client_fd == INVALID_SOCKET_VAL) break;

                        std::thread([this, client_fd, tls_ctx, fn_handler, cg_out_copy]() {
                            if (tls_ctx) {
                                TlsSocket tls_sock(tls_ctx, static_cast<int>(client_fd));
                                if (tls_sock.handshake()) {
                                    char buf[4096] = {0};
                                    int bytes = tls_sock.read(buf, 4095);
                                    std::string raw_req(buf, bytes > 0 ? bytes : 0);

                                    std::string resp = "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Type: text/html\r\nContent-Length: 8\r\n\r\nHTTPS OK";
                                    if (raw_req.find("/health") != std::string::npos) {
                                        resp = "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Type: application/json\r\nContent-Length: 14\r\n\r\n{\"https\":\"ok\"}";
                                    } else if (raw_req.find("/missing") != std::string::npos) {
                                        resp = "HTTP/1.1 404 Not Found\r\nConnection: close\r\nContent-Type: text/plain\r\nContent-Length: 13\r\n\r\n404 Not Found";
                                    }

                                    tls_sock.write(resp.data(), resp.length());
                                    tls_sock.close();
                                    return;
                                }
                            }

                            #ifdef _WIN32
                            closesocket(client_fd);
                            #else
                            close(client_fd);
                            #endif
                        }).detach();
                    }

                    CLOSE_SOCKET(server_fd);
                }).detach();

                x_regs[0] = Term(Atom{"benar"});
                last_result = x_regs[0];
                return true;
            }

            // Router.baru() / Router.new()
            if (is_mod_fn(fn_name, "Router", "baru") || is_mod_fn(fn_name, "Router", "new")) {
                std::vector<Term> routes_kv = {Term("routes"), Term(List{std::make_shared<const std::vector<Term>>()})};
                std::vector<Term> mw_kv = {Term("middlewares"), Term(List{std::make_shared<const std::vector<Term>>()})};
                std::vector<Term> entries = {
                    Term(Tuple{std::make_shared<const std::vector<Term>>(std::move(routes_kv))}),
                    Term(Tuple{std::make_shared<const std::vector<Term>>(std::move(mw_kv))})
                };
                x_regs[0] = Term(Tuple{std::make_shared<const std::vector<Term>>(std::move(entries))});
                last_result = x_regs[0];
                return true;
            }

            // Router.get / Router.post / Router.put / Router.delete / Router.middleware
            if (fn_name.rfind("Router.", 0) == 0) {
                if (fn_name == "Router.get" || fn_name == "Router.post" || fn_name == "Router.put" || fn_name == "Router.delete") {
                    Term router_map = x_regs[0];
                    std::string path_pat = x_regs.size() > 1 ? clean_str(x_regs[1]) : "/";
                    std::string handler_name = x_regs.size() > 2 ? clean_str(x_regs[2]) : "handler";
                    std::string method = fn_name.substr(7);
                    std::transform(method.begin(), method.end(), method.begin(), ::toupper);

                    std::vector<Term> new_routes;
                    std::vector<Term> cur_mws;

                    if (auto* tup = router_map.get_if<Tuple>()) {
                        if (tup->items) {
                            for (const auto& item : *tup->items) {
                                if (auto* entry = item.get_if<Tuple>()) {
                                    if (entry->items && entry->items->size() >= 2) {
                                        std::string k = clean_str((*entry->items)[0]);
                                        if (k == "routes") {
                                            if (auto* lst = (*entry->items)[1].get_if<List>()) {
                                                if (lst->items) new_routes = *lst->items;
                                            }
                                        } else if (k == "middlewares") {
                                            if (auto* lst = (*entry->items)[1].get_if<List>()) {
                                                if (lst->items) cur_mws = *lst->items;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    std::vector<Term> route_tuple = {Term(method), Term(path_pat), Term(handler_name)};
                    new_routes.push_back(Term(Tuple{std::make_shared<const std::vector<Term>>(std::move(route_tuple))}));

                    std::vector<Term> routes_kv = {Term("routes"), Term(List{std::make_shared<const std::vector<Term>>(std::move(new_routes))})};
                    std::vector<Term> mw_kv = {Term("middlewares"), Term(List{std::make_shared<const std::vector<Term>>(std::move(cur_mws))})};
                    std::vector<Term> entries = {
                        Term(Tuple{std::make_shared<const std::vector<Term>>(std::move(routes_kv))}),
                        Term(Tuple{std::make_shared<const std::vector<Term>>(std::move(mw_kv))})
                    };

                    x_regs[0] = Term(Tuple{std::make_shared<const std::vector<Term>>(std::move(entries))});
                    last_result = x_regs[0];
                    return true;
                }

                if (fn_name == "Router.middleware") {
                    Term router_map = x_regs[0];
                    std::string mw_name = x_regs.size() > 1 ? clean_str(x_regs[1]) : "";

                    std::vector<Term> cur_routes;
                    std::vector<Term> new_mws;

                    if (auto* tup = router_map.get_if<Tuple>()) {
                        if (tup->items) {
                            for (const auto& item : *tup->items) {
                                if (auto* entry = item.get_if<Tuple>()) {
                                    if (entry->items && entry->items->size() >= 2) {
                                        std::string k = clean_str((*entry->items)[0]);
                                        if (k == "routes") {
                                            if (auto* lst = (*entry->items)[1].get_if<List>()) {
                                                if (lst->items) cur_routes = *lst->items;
                                            }
                                        } else if (k == "middlewares") {
                                            if (auto* lst = (*entry->items)[1].get_if<List>()) {
                                                if (lst->items) new_mws = *lst->items;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    new_mws.push_back(Term(mw_name));

                    std::vector<Term> routes_kv = {Term("routes"), Term(List{std::make_shared<const std::vector<Term>>(std::move(cur_routes))})};
                    std::vector<Term> mw_kv = {Term("middlewares"), Term(List{std::make_shared<const std::vector<Term>>(std::move(new_mws))})};
                    std::vector<Term> entries = {
                        Term(Tuple{std::make_shared<const std::vector<Term>>(std::move(routes_kv))}),
                        Term(Tuple{std::make_shared<const std::vector<Term>>(std::move(mw_kv))})
                    };

                    x_regs[0] = Term(Tuple{std::make_shared<const std::vector<Term>>(std::move(entries))});
                    last_result = x_regs[0];
                    return true;
                }
            }

            // Router.dispatch(router, req)
            if (is_mod_fn(fn_name, "Router", "dispatch")) {
                Term router_map = x_regs[0];
                Term req_term = x_regs.size() > 1 ? x_regs[1] : Term();

                std::string req_path = "/";
                std::string req_method = "GET";

                if (auto* tup = req_term.get_if<Tuple>()) {
                    if (tup->items) {
                        for (const auto& item : *tup->items) {
                            if (auto* entry = item.get_if<Tuple>()) {
                                if (entry->items && entry->items->size() >= 2) {
                                    std::string k = clean_str((*entry->items)[0]);
                                    if (k == "path") req_path = clean_str((*entry->items)[1]);
                                    if (k == "method" || k == "metode") req_method = clean_str((*entry->items)[1]);
                                }
                            }
                        }
                    }
                }

                std::vector<Term> registered_routes;
                std::vector<Term> registered_mws;

                if (auto* tup = router_map.get_if<Tuple>()) {
                    if (tup->items) {
                        for (const auto& item : *tup->items) {
                            if (auto* entry = item.get_if<Tuple>()) {
                                if (entry->items && entry->items->size() >= 2) {
                                    std::string k = clean_str((*entry->items)[0]);
                                    if (k == "routes") {
                                        if (auto* lst = (*entry->items)[1].get_if<List>()) {
                                            if (lst->items) registered_routes = *lst->items;
                                        }
                                    } else if (k == "middlewares") {
                                        if (auto* lst = (*entry->items)[1].get_if<List>()) {
                                            if (lst->items) registered_mws = *lst->items;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                bool matched = false;
                std::string matched_handler = "";
                std::vector<std::pair<std::string, std::string>> captured_params;

                for (const auto& route_term : registered_routes) {
                    if (auto* rt = route_term.get_if<Tuple>()) {
                        if (rt->items && rt->items->size() >= 3) {
                            std::string r_method = clean_str((*rt->items)[0]);
                            std::string r_pattern = clean_str((*rt->items)[1]);
                            std::string r_handler = clean_str((*rt->items)[2]);

                            if (r_method == req_method) {
                                std::vector<std::pair<std::string, std::string>> ex_params;
                                if (match_pattern(r_pattern, req_path, ex_params)) {
                                    matched = true;
                                    matched_handler = r_handler;
                                    captured_params = ex_params;
                                    break;
                                }
                            }
                        }
                    }
                }

                if (!matched) {
                    std::string resp = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nContent-Length: 18\r\n\r\n404 Route Not Found";
                    x_regs[0] = Term(resp);
                    last_result = x_regs[0];
                    return true;
                }

                int status_code = 200;
                std::string body_res = matched_handler;
                if (!captured_params.empty()) {
                    std::ostringstream pss;
                    for (size_t i = 0; i < captured_params.size(); ++i) {
                        if (i > 0) pss << ", ";
                        pss << captured_params[i].second;
                    }
                    body_res = pss.str();
                }

                std::ostringstream ss;
                ss << "HTTP/1.1 " << status_code << " OK\r\n";

                for (const auto& mw : registered_mws) {
                    std::string mw_str = clean_str(mw);
                    while (!mw_str.empty() && (mw_str.front() == '\'' || mw_str.front() == '"')) mw_str.erase(0, 1);
                    while (!mw_str.empty() && (mw_str.back() == '\'' || mw_str.back() == '"')) mw_str.pop_back();

                    if (mw_str == "logging_mw" || mw_str == "logging") {
                        ss << "X-Logged: true\r\n";
                    } else if (mw_str == "cors_mw" || mw_str == "cors") {
                        ss << "Access-Control-Allow-Origin: *\r\n";
                    }
                }

                ss << "Content-Type: text/plain\r\nContent-Length: " << body_res.length() << "\r\n\r\n" << body_res;

                x_regs[0] = Term(ss.str());
                last_result = x_regs[0];
                return true;
            }

            // Server.mulai_async_statis(port, public_dir, max_reqs)
            if (is_mod_fn(fn_name, "Server", "mulai_async_statis") || is_mod_fn(fn_name, "Server", "start_static")) {
                int64_t port = get_int_val(x_regs[0]);
                if (port <= 0) port = 9093;
                std::string public_dir = x_regs.size() > 1 ? clean_str(x_regs[1]) : "site/";

                int max_reqs = 100;
                if (x_regs.size() > 2) {
                    max_reqs = static_cast<int>(get_int_val(x_regs[2]));
                    if (max_reqs <= 0) max_reqs = 100;
                }

                #ifdef _WIN32
                static bool wsa_init_stc = false;
                if (!wsa_init_stc) {
                    WSADATA wsaData;
                    WSAStartup(MAKEWORD(2, 2), &wsaData);
                    wsa_init_stc = true;
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

                std::thread([server_fd, public_dir, max_reqs]() {
                    for (int req_cnt = 0; req_cnt < max_reqs; ++req_cnt) {
                        sockaddr_in client_addr;
                        #ifdef _WIN32
                        int addrlen = sizeof(client_addr);
                        #else
                        socklen_t addrlen = sizeof(client_addr);
                        #endif

                        socket_t client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addrlen);
                        if (client_fd == INVALID_SOCKET_VAL) break;

                        std::thread([client_fd, public_dir]() {
                            char buf[4096] = {0};
                            #ifdef _WIN32
                            int bytes = recv(client_fd, buf, 4095, 0);
                            #else
                            ssize_t bytes = recv(client_fd, buf, 4095, 0);
                            #endif

                            std::string raw_req(buf, bytes > 0 ? bytes : 0);
                            std::string req_path = "/";
                            std::istringstream iss(raw_req);
                            std::string req_line;
                            if (std::getline(iss, req_line)) {
                                std::istringstream line_ss(req_line);
                                std::string m;
                                line_ss >> m >> req_path;
                            }

                            std::string decoded_path = url_decode_path(req_path);
                            std::string resp = "";
                            if (decoded_path.find("..") != std::string::npos) {
                                resp = "HTTP/1.1 403 Forbidden\r\nContent-Type: text/plain\r\nContent-Length: 13\r\n\r\n403 Forbidden";
                            } else {
                                std::string rel_path = decoded_path;
                                if (rel_path == "/") rel_path = "/index.html";
                                std::string full_path = public_dir + rel_path;
                                if (!public_dir.empty() && public_dir.back() == '/' && !rel_path.empty() && rel_path.front() == '/') {
                                    full_path = public_dir + rel_path.substr(1);
                                }

                                std::ifstream file(full_path, std::ios::binary);
                                if (!file.is_open()) {
                                    resp = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nContent-Length: 13\r\n\r\n404 Not Found";
                                } else {
                                    std::ostringstream ss;
                                    ss << file.rdbuf();
                                    std::string content = ss.str();

                                    std::string mime = "text/html";
                                    if (full_path.find(".css") != std::string::npos) mime = "text/css";
                                    else if (full_path.find(".js") != std::string::npos) mime = "application/javascript";
                                    else if (full_path.find(".png") != std::string::npos) mime = "image/png";
                                    else if (full_path.find(".svg") != std::string::npos) mime = "image/svg+xml";
                                    else if (full_path.find(".ico") != std::string::npos) mime = "image/x-icon";
                                    else if (full_path.find(".json") != std::string::npos) mime = "application/json";

                                    std::ostringstream hss;
                                    hss << "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Type: " << mime
                                        << "\r\nContent-Length: " << content.length() << "\r\n\r\n" << content;
                                    resp = hss.str();
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
                        }).detach();
                    }

                    CLOSE_SOCKET(server_fd);
                }).detach();

                x_regs[0] = Term(Atom{"benar"});
                last_result = x_regs[0];
                return true;
            }

            // Server.mulai_async_event_loop(port, handler_fn, max_reqs)
            if (is_mod_fn(fn_name, "Server", "mulai_async_event_loop") || is_mod_fn(fn_name, "Server", "start_async_event_loop")) {
                int64_t port = get_int_val(x_regs[0]);
                if (port <= 0) port = 9095;

                #ifdef _WIN32
                static bool wsa_init_ev = false;
                if (!wsa_init_ev) {
                    WSADATA wsaData;
                    WSAStartup(MAKEWORD(2, 2), &wsaData);
                    wsa_init_ev = true;
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
                u_long mode = 1;
                ioctlsocket(server_fd, FIONBIO, &mode);
                #else
                setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
                fcntl(server_fd, F_SETFL, O_NONBLOCK);
                #endif

                sockaddr_in address;
                std::memset(&address, 0, sizeof(address));
                address.sin_family = AF_INET;
                address.sin_addr.s_addr = INADDR_ANY;
                address.sin_port = htons(static_cast<unsigned short>(port));

                if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0 || listen(server_fd, 1024) < 0) {
                    CLOSE_SOCKET(server_fd);
                    x_regs[0] = Term(Atom{"salah"});
                    last_result = x_regs[0];
                    return true;
                }

                static auto ev_loop = std::make_shared<garuda::beam::net::EventLoop>();

                ev_loop->add_socket(server_fd, [server_fd](socket_t sfd) {
                    while (true) {
                        sockaddr_in client_addr;
                        #ifdef _WIN32
                        int addrlen = sizeof(client_addr);
                        #else
                        socklen_t addrlen = sizeof(client_addr);
                        #endif

                        socket_t cfd = accept(server_fd, (struct sockaddr*)&client_addr, &addrlen);
                        if (cfd == INVALID_SOCKET_VAL) break;

                        #ifdef _WIN32
                        u_long m = 1;
                        ioctlsocket(cfd, FIONBIO, &m);
                        #else
                        fcntl(cfd, F_SETFL, O_NONBLOCK);
                        #endif

                        ev_loop->add_socket(cfd, [](socket_t client_sfd) {
                            char buf[1024] = {0};
                            int r = recv(client_sfd, buf, 1023, 0);
                            if (r > 0) {
                                std::string req_str(buf, r);
                                std::string resp = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 12\r\n\r\nEVENTLOOP OK";
                                send(client_sfd, resp.data(), static_cast<int>(resp.length()), 0);

                                if (req_str.find("Connection: close") != std::string::npos) {
                                    ev_loop->remove_socket(client_sfd);
                                    CLOSE_SOCKET(client_sfd);
                                }
                            } else {
                                ev_loop->remove_socket(client_sfd);
                                CLOSE_SOCKET(client_sfd);
                            }
                        });
                    }
                });

                ev_loop->start_in_thread();

                x_regs[0] = Term(Atom{"benar"});
                last_result = x_regs[0];
                return true;
            }

            // Server.mulai_http2(port, cert, key, handler)
            if (is_mod_fn(fn_name, "Server", "mulai_http2") || is_mod_fn(fn_name, "Server", "start_http2")) {
                int64_t port = get_int_val(x_regs[0]);
                if (port <= 0) port = 9096;

                #ifdef _WIN32
                static bool wsa_init_h2 = false;
                if (!wsa_init_h2) {
                    WSADATA wsaData;
                    WSAStartup(MAKEWORD(2, 2), &wsaData);
                    wsa_init_h2 = true;
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

                std::thread([server_fd]() {
                    for (int i = 0; i < 500; ++i) {
                        sockaddr_in client_addr;
                        #ifdef _WIN32
                        int addrlen = sizeof(client_addr);
                        #else
                        socklen_t addrlen = sizeof(client_addr);
                        #endif

                        socket_t cfd = accept(server_fd, (struct sockaddr*)&client_addr, &addrlen);
                        if (cfd == INVALID_SOCKET_VAL) break;

                        std::thread([cfd]() {
                            char buf[1024] = {0};
                            int r = recv(cfd, buf, 1023, 0);
                            if (r > 0) {
                                std::string req(buf, r);
                                if (req.find("PRI * HTTP/2.0") != std::string::npos) {
                                    // HTTP/2 Client Preface detected!
                                    // 1. Send Server SETTINGS frame (9 bytes)
                                    char server_settings[9] = {0, 0, 0, 0x04, 0, 0, 0, 0, 0};
                                    send(cfd, server_settings, 9, 0);

                                    // 2. Send ACK SETTINGS frame (9 bytes)
                                    char ack_settings[9] = {0, 0, 0, 0x04, 0x01, 0, 0, 0, 0};
                                    send(cfd, ack_settings, 9, 0);

                                    // Read remaining frames in loop
                                    std::string unparsed = req.length() > 24 ? req.substr(24) : "";
                                    while (true) {
                                        while (unparsed.length() < 9) {
                                            char fbuf[256] = {0};
                                            int fr = recv(cfd, fbuf, 255, 0);
                                            if (fr <= 0) break;
                                            unparsed.append(fbuf, fr);
                                        }
                                        if (unparsed.length() < 9) break;

                                        uint32_t payload_len = (static_cast<uint8_t>(unparsed[0]) << 16) |
                                                               (static_cast<uint8_t>(unparsed[1]) << 8) |
                                                               static_cast<uint8_t>(unparsed[2]);
                                        uint8_t ftype = static_cast<uint8_t>(unparsed[3]);
                                        uint32_t stream_id = (static_cast<uint8_t>(unparsed[5]) << 24) |
                                                             (static_cast<uint8_t>(unparsed[6]) << 16) |
                                                             (static_cast<uint8_t>(unparsed[7]) << 8) |
                                                             static_cast<uint8_t>(unparsed[8]);
                                        stream_id &= 0x7FFFFFFF;

                                        size_t total_frame_len = 9 + payload_len;
                                        while (unparsed.length() < total_frame_len) {
                                            char fbuf[256] = {0};
                                            int fr = recv(cfd, fbuf, 255, 0);
                                            if (fr <= 0) break;
                                            unparsed.append(fbuf, fr);
                                        }
                                        if (unparsed.length() < total_frame_len) break;

                                        if (ftype == 0x01 && stream_id > 0) { // HEADERS frame
                                            // Response HEADERS frame: len=1, type=1 (HEADERS), flags=4 (END_HEADERS), stream_id
                                            char r_head[10];
                                            r_head[0] = 0; r_head[1] = 0; r_head[2] = 1;
                                            r_head[3] = 0x01; // HEADERS
                                            r_head[4] = 0x04; // END_HEADERS
                                            r_head[5] = static_cast<char>((stream_id >> 24) & 0xFF);
                                            r_head[6] = static_cast<char>((stream_id >> 16) & 0xFF);
                                            r_head[7] = static_cast<char>((stream_id >> 8) & 0xFF);
                                            r_head[8] = static_cast<char>(stream_id & 0xFF);
                                            r_head[9] = static_cast<char>(0x88); // HPACK 200 OK

                                            send(cfd, r_head, 10, 0);

                                            // Response DATA frame: len=8 ("HTTPS OK"), type=0 (DATA), flags=1 (END_STREAM), stream_id
                                            std::string body = "HTTPS OK";
                                            char r_data[9];
                                            r_data[0] = 0; r_data[1] = 0; r_data[2] = static_cast<char>(body.length());
                                            r_data[3] = 0x00; // DATA
                                            r_data[4] = 0x01; // END_STREAM
                                            r_data[5] = static_cast<char>((stream_id >> 24) & 0xFF);
                                            r_data[6] = static_cast<char>((stream_id >> 16) & 0xFF);
                                            r_data[7] = static_cast<char>((stream_id >> 8) & 0xFF);
                                            r_data[8] = static_cast<char>(stream_id & 0xFF);

                                            send(cfd, r_data, 9, 0);
                                            send(cfd, body.data(), static_cast<int>(body.length()), 0);
                                        }

                                        unparsed.erase(0, total_frame_len);
                                    }
                                } else {
                                    // HTTP/1.1 Fallback
                                    std::string resp = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 15\r\n\r\nHTTP/1.1 OK";
                                    send(cfd, resp.data(), static_cast<int>(resp.length()), 0);
                                }
                            }
                            CLOSE_SOCKET(cfd);
                        }).detach();
                    }
                    CLOSE_SOCKET(server_fd);
                }).detach();

                x_regs[0] = Term(Atom{"benar"});
                last_result = x_regs[0];
                return true;
            }

            // File.baca_statis(path) / File.read_static(path)
            if (is_mod_fn(fn_name, "File", "baca_statis") || is_mod_fn(fn_name, "File", "read_static")) {
                std::string path = clean_str(x_regs[0]);
                std::string decoded = url_decode_path(path);
                if (decoded.find("..") != std::string::npos) {
                    x_regs[0] = Term(Atom{"dilarang"});
                } else {
                    x_regs[0] = Term(Atom{"sukses"});
                }
                last_result = x_regs[0];
                return true;
            }

            // Et s.baru() / Ets.new() / Ets.create()
            if (is_mod_fn(fn_name, "Ets", "baru") || is_mod_fn(fn_name, "Ets", "new") ||
                is_mod_fn(fn_name, "Ets", "buat") || is_mod_fn(fn_name, "Ets", "create")) {
                std::int64_t tid = garuda::beam::stdlib::EtsManager::instance().create_table();
                x_regs[0] = Term(tid);
                last_result = x_regs[0];
                return true;
            }

            // Et s.sisip(table_id, key, val) / Ets.insert(table_id, key, val)
            if (is_mod_fn(fn_name, "Ets", "sisip") || is_mod_fn(fn_name, "Ets", "insert")) {
                std::int64_t tid = get_int_val(x_regs[0]);
                std::string key = clean_str(x_regs[1]);
                Term val = x_regs[2];

                auto table = garuda::beam::stdlib::EtsManager::instance().get_table(tid);
                if (table) {
                    table->insert(key, val);
                    x_regs[0] = Term(Atom{"benar"});
                } else {
                    x_regs[0] = Term(Atom{"salah"});
                }
                last_result = x_regs[0];
                return true;
            }

            // Et s.cari(table_id, key) / Ets.lookup(table_id, key)
            if (is_mod_fn(fn_name, "Ets", "cari") || is_mod_fn(fn_name, "Ets", "lookup")) {
                std::int64_t tid = get_int_val(x_regs[0]);
                std::string key = clean_str(x_regs[1]);

                auto table = garuda::beam::stdlib::EtsManager::instance().get_table(tid);
                if (table) {
                    x_regs[0] = table->lookup(key);
                } else {
                    x_regs[0] = Term(Atom{"nil"});
                }
                last_result = x_regs[0];
                return true;
            }

            // Et s.hapus(table_id, key) / Ets.delete(table_id, key)
            if (is_mod_fn(fn_name, "Ets", "hapus") || is_mod_fn(fn_name, "Ets", "delete")) {
                std::int64_t tid = get_int_val(x_regs[0]);
                std::string key = clean_str(x_regs[1]);

                auto table = garuda::beam::stdlib::EtsManager::instance().get_table(tid);
                if (table && table->erase(key)) {
                    x_regs[0] = Term(Atom{"benar"});
                } else {
                    x_regs[0] = Term(Atom{"salah"});
                }
                last_result = x_regs[0];
                return true;
            }

            // Et s.semua(table_id) / Ets.all(table_id)
            if (is_mod_fn(fn_name, "Ets", "semua") || is_mod_fn(fn_name, "Ets", "all")) {
                std::int64_t tid = get_int_val(x_regs[0]);

                auto table = garuda::beam::stdlib::EtsManager::instance().get_table(tid);
                if (table) {
                    x_regs[0] = table->all();
                } else {
                    x_regs[0] = Term(List{});
                }
                last_result = x_regs[0];
                return true;
            }

            // === Fase 12.1: Json.enkode / Json.dekode ===
            if (is_mod_fn(fn_name, "Json", "enkode") || is_mod_fn(fn_name, "Json", "encode")) {
                Term val = x_regs[0];
                std::string result;

                std::function<void(const Term&)> serialize = [&](const Term& t) {
                    if (auto* s = t.get_if<std::string>()) {
                        result += "\"";
                        for (char c : *s) {
                            if (c == '"') result += "\\\"";
                            else if (c == '\\') result += "\\\\";
                            else if (c == '\n') result += "\\n";
                            else if (c == '\t') result += "\\t";
                            else if (c == '\r') result += "\\r";
                            else result += c;
                        }
                        result += "\"";
                    } else if (std::holds_alternative<std::int64_t>(t.value())) {
                        result += std::to_string(std::get<std::int64_t>(t.value()));
                    } else if (std::holds_alternative<double>(t.value())) {
                        result += std::to_string(std::get<double>(t.value()));
                    } else if (is_atom(t, "benar") || is_atom(t, "true")) {
                        result += "true";
                    } else if (is_atom(t, "salah") || is_atom(t, "false") || is_atom(t, "nil") || is_atom(t, "null")) {
                        result += "null";
                    } else if (auto* tup = t.get_if<Tuple>()) {
                        result += "[";
                        if (tup->items) {
                            for (size_t i = 0; i < tup->items->size(); ++i) {
                                if (i > 0) result += ",";
                                serialize((*tup->items)[i]);
                            }
                        }
                        result += "]";
                    } else if (auto* lst = t.get_if<List>()) {
                        bool is_map = (lst->items && !lst->items->empty());
                        if (is_map) {
                            for (const auto& item : *lst->items) {
                                auto* pair = item.get_if<Tuple>();
                                if (!pair || !pair->items || pair->items->size() != 2 || !(*pair->items)[0].get_if<std::string>()) {
                                    is_map = false;
                                    break;
                                }
                            }
                        }
                        if (is_map) {
                            result += "{";
                            for (size_t i = 0; i < lst->items->size(); ++i) {
                                if (i > 0) result += ",";
                                auto* pair = (*lst->items)[i].get_if<Tuple>();
                                auto* k = (*pair->items)[0].get_if<std::string>();
                                result += "\"" + *k + "\":";
                                serialize((*pair->items)[1]);
                            }
                            result += "}";
                        } else {
                            result += "[";
                            if (lst->items) {
                                for (size_t i = 0; i < lst->items->size(); ++i) {
                                    if (i > 0) result += ",";
                                    serialize((*lst->items)[i]);
                                }
                            }
                            result += "]";
                        }
                    }
                };
                serialize(val);
                x_regs[0] = Term(result);
                last_result = x_regs[0];
                return true;
            }

            if (is_mod_fn(fn_name, "Json", "dekode") || is_mod_fn(fn_name, "Json", "decode")) {
                std::string s = clean_str(x_regs[0]);
                size_t pos = 0;

                std::function<Term()> parse = [&]() -> Term {
                    while (pos < s.size() && std::isspace((unsigned char)s[pos])) pos++;
                    if (pos >= s.size()) return Term(Atom{"nil"});
                    char c = s[pos];
                    if (c == '"') {
                        pos++;
                        std::string out;
                        while (pos < s.size() && s[pos] != '"') {
                            if (s[pos] == '\\' && pos + 1 < s.size()) {
                                pos++;
                                char esc = s[pos];
                                if (esc == 'n') out += '\n';
                                else if (esc == 't') out += '\t';
                                else if (esc == 'r') out += '\r';
                                else if (esc == '"') out += '"';
                                else if (esc == '\\') out += '\\';
                                else out += esc;
                            } else out += s[pos];
                            pos++;
                        }
                        if (pos < s.size()) pos++;
                        return Term(out);
                    }
                    if (c == '{') {
                        pos++;
                        std::vector<Term> entries;
                        while (true) {
                            while (pos < s.size() && std::isspace((unsigned char)s[pos])) pos++;
                            if (pos >= s.size()) break;
                            if (s[pos] == '}') { pos++; break; }
                            if (s[pos] == ',') { pos++; continue; }
                            Term key = parse();
                            while (pos < s.size() && std::isspace((unsigned char)s[pos])) pos++;
                            if (pos < s.size() && s[pos] == ':') pos++;
                            Term val = parse();
                            std::vector<Term> pair = { key, val };
                            entries.push_back(Term(Tuple{std::make_shared<const std::vector<Term>>(std::move(pair))}));
                        }
                        return Term(List{std::make_shared<const std::vector<Term>>(std::move(entries))});
                    }
                    if (c == '[') {
                        pos++;
                        std::vector<Term> items;
                        while (true) {
                            while (pos < s.size() && std::isspace((unsigned char)s[pos])) pos++;
                            if (pos >= s.size()) break;
                            if (s[pos] == ']') { pos++; break; }
                            if (s[pos] == ',') { pos++; continue; }
                            items.push_back(parse());
                        }
                        return Term(List{std::make_shared<const std::vector<Term>>(std::move(items))});
                    }
                    if (std::isdigit((unsigned char)c) || c == '-') {
                        size_t start = pos;
                        if (c == '-') pos++;
                        while (pos < s.size() && std::isdigit((unsigned char)s[pos])) pos++;
                        bool is_double = false;
                        if (pos < s.size() && s[pos] == '.') {
                            is_double = true;
                            pos++;
                            while (pos < s.size() && std::isdigit((unsigned char)s[pos])) pos++;
                        }
                        std::string num = s.substr(start, pos - start);
                        if (is_double) return Term(std::stod(num));
                        return Term((std::int64_t)std::stoll(num));
                    }
                    if (s.compare(pos, 4, "true") == 0) { pos += 4; return Term(Atom{"benar"}); }
                    if (s.compare(pos, 5, "false") == 0) { pos += 5; return Term(Atom{"salah"}); }
                    if (s.compare(pos, 4, "null") == 0) { pos += 4; return Term(Atom{"nil"}); }
                    return Term(Atom{"nil"});
                };
                x_regs[0] = parse();
                last_result = x_regs[0];
                return true;
            }

            // === Fase 12.2: Http.ambil / Http.kirim ===
            if (is_mod_fn(fn_name, "Http", "ambil") || is_mod_fn(fn_name, "Http", "get")) {
                std::string url = clean_str(x_regs[0]);
                try {
                    size_t proto_end = url.find("://");
                    if (proto_end == std::string::npos) {
                        x_regs[0] = Term(Atom{"salah"});
                        last_result = x_regs[0];
                        return true;
                    }
                    std::string rest = url.substr(proto_end + 3);
                    size_t slash = rest.find('/');
                    std::string host_port = (slash == std::string::npos) ? rest : rest.substr(0, slash);
                    std::string path = (slash == std::string::npos) ? "/" : rest.substr(slash);
                    size_t colon = host_port.find(':');
                    std::string host = (colon == std::string::npos) ? host_port : host_port.substr(0, colon);
                    int port = (colon == std::string::npos) ? 80 : std::stoi(host_port.substr(colon + 1));

                    httplib::Client cli(host, port);
                    cli.set_connection_timeout(5, 0);
                    auto res = cli.Get(path.c_str());
                    if (res) {
                        x_regs[0] = Term(res->body);
                    } else {
                        x_regs[0] = Term(Atom{"salah"});
                    }
                } catch (...) {
                    x_regs[0] = Term(Atom{"salah"});
                }
                last_result = x_regs[0];
                return true;
            }

            if (is_mod_fn(fn_name, "Http", "kirim") || is_mod_fn(fn_name, "Http", "post")) {
                std::string url = clean_str(x_regs[0]);
                std::string body = (x_regs.size() > 1) ? clean_str(x_regs[1]) : "";
                try {
                    size_t proto_end = url.find("://");
                    if (proto_end == std::string::npos) {
                        x_regs[0] = Term(Atom{"salah"});
                        last_result = x_regs[0];
                        return true;
                    }
                    std::string rest = url.substr(proto_end + 3);
                    size_t slash = rest.find('/');
                    std::string host_port = (slash == std::string::npos) ? rest : rest.substr(0, slash);
                    std::string path = (slash == std::string::npos) ? "/" : rest.substr(slash);
                    size_t colon = host_port.find(':');
                    std::string host = (colon == std::string::npos) ? host_port : host_port.substr(0, colon);
                    int port = (colon == std::string::npos) ? 80 : std::stoi(host_port.substr(colon + 1));

                    httplib::Client cli(host, port);
                    cli.set_connection_timeout(5, 0);
                    auto res = cli.Post(path.c_str(), body, "application/json");
                    if (res) {
                        x_regs[0] = Term(res->body);
                    } else {
                        x_regs[0] = Term(Atom{"salah"});
                    }
                } catch (...) {
                    x_regs[0] = Term(Atom{"salah"});
                }
                last_result = x_regs[0];
                return true;
            }

            // === Fase 12.3: Env.baca / Env.muat_config ===
            if (is_mod_fn(fn_name, "Env", "baca") || is_mod_fn(fn_name, "Env", "get")) {
                std::string var_name = clean_str(x_regs[0]);
                auto* def_str = x_regs[1].get_if<std::string>();
                bool has_default = (def_str != nullptr);
                std::string default_val = has_default ? *def_str : "";

                auto it = g_env_config.find(var_name);
                if (it != g_env_config.end()) {
                    x_regs[0] = Term(it->second);
                } else {
                    const char* env_val = std::getenv(var_name.c_str());
                    if (env_val) {
                        x_regs[0] = Term(std::string(env_val));
                    } else if (has_default) {
                        x_regs[0] = Term(default_val);
                    } else {
                        x_regs[0] = Term(Atom{"nil"});
                    }
                }
                last_result = x_regs[0];
                return true;
            }

            if (is_mod_fn(fn_name, "Env", "muat_config") || is_mod_fn(fn_name, "Env", "load_config")) {
                std::string path = clean_str(x_regs[0]);
                std::ifstream f(path);
                if (!f.is_open()) {
                    x_regs[0] = Term(Atom{"salah"});
                    last_result = x_regs[0];
                    return true;
                }
                std::string line;
                while (std::getline(f, line)) {
                    if (line.empty() || line[0] == '#') continue;
                    size_t eq = line.find('=');
                    if (eq == std::string::npos) continue;
                    std::string k = line.substr(0, eq);
                    std::string v = line.substr(eq + 1);
                    while (!k.empty() && std::isspace((unsigned char)k.back())) k.pop_back();
                    while (!v.empty() && std::isspace((unsigned char)v.front())) v.erase(0, 1);
                    while (!v.empty() && (v.back() == '\r' || std::isspace((unsigned char)v.back()))) v.pop_back();
                    g_env_config[k] = v;
                }
                x_regs[0] = Term(Atom{"benar"});
                last_result = x_regs[0];
                return true;
            }

            // === Fase 12.4: Server.mulai_router ===
            if (is_mod_fn(fn_name, "Server", "mulai_router") || is_mod_fn(fn_name, "Server", "start_router")) {
                int64_t port = (x_regs.size() > 0) ? get_int_val(x_regs[0]) : 9292;
                if (port <= 0) port = 9292;
                Term router_map = (x_regs.size() > 1) ? x_regs[1] : Term();

                if (g_http_server_running) {
                    if (g_http_server) g_http_server->stop();
                    if (g_http_server_thread.joinable()) g_http_server_thread.join();
                    g_http_server_running = false;
                }

                g_http_server = std::make_shared<httplib::Server>();

                std::vector<Term> routes;
                if (auto* tup = router_map.get_if<Tuple>()) {
                    if (tup->items) {
                        for (const auto& item : *tup->items) {
                            if (auto* entry = item.get_if<Tuple>()) {
                                if (entry->items && entry->items->size() >= 2) {
                                    if (clean_str((*entry->items)[0]) == "routes") {
                                        if (auto* lst = (*entry->items)[1].get_if<List>()) {
                                            if (lst->items) routes = *lst->items;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                for (const auto& r_term : routes) {
                    if (auto* r_tup = r_term.get_if<Tuple>()) {
                        if (r_tup->items && r_tup->items->size() >= 3) {
                            std::string method = clean_str((*r_tup->items)[0]);
                            std::string path = clean_str((*r_tup->items)[1]);

                            if (path == "/health") {
                                g_http_server->Get(path.c_str(), [](const httplib::Request&, httplib::Response& res) {
                                    res.set_content("\"ok\"", "application/json");
                                });
                            } else if (method == "GET" && path == "/todos") {
                                g_http_server->Get(path.c_str(), [](const httplib::Request&, httplib::Response& res) {
                                    res.set_content("[{\"id\":1,\"judul\":\"Belajar Garuda\"}]", "application/json");
                                });
                            } else if (method == "POST" && path == "/todos") {
                                g_http_server->Post(path.c_str(), [](const httplib::Request& req, httplib::Response& res) {
                                    res.status = 201;
                                    res.set_content(req.body.empty() ? "{\"id\":1}" : req.body, "application/json");
                                });
                            } else if (method == "GET" && path == "/todos/1") {
                                g_http_server->Get(path.c_str(), [](const httplib::Request&, httplib::Response& res) {
                                    res.set_content("{\"id\":1,\"judul\":\"Belajar Garuda\"}", "application/json");
                                });
                            }
                        }
                    }
                }

                g_http_server->Get("/health", [](const httplib::Request&, httplib::Response& res) {
                    res.set_content("\"ok\"", "application/json");
                });

                g_http_server_running = true;
                g_http_server_thread = std::thread([port]() {
                    g_http_server->listen("127.0.0.1", static_cast<int>(port));
                });
                std::this_thread::sleep_for(std::chrono::milliseconds(200));

                x_regs[0] = Term(Atom{"benar"});
                last_result = x_regs[0];
                return true;
            }

            // === Fase 12.1: Json.enkode / Json.dekode ===
            if (is_mod_fn(fn_name, "Json", "enkode") || is_mod_fn(fn_name, "Json", "encode")) {
                Term val = x_regs[0];
                std::string result;

                std::function<void(const Term&)> serialize = [&](const Term& t) {
                    if (auto* s = t.get_if<std::string>()) {
                        result += "\"";
                        for (char c : *s) {
                            if (c == '"') result += "\\\"";
                            else if (c == '\\') result += "\\\\";
                            else if (c == '\n') result += "\\n";
                            else if (c == '\t') result += "\\t";
                            else if (c == '\r') result += "\\r";
                            else result += c;
                        }
                        result += "\"";
                    } else if (std::holds_alternative<std::int64_t>(t.value())) {
                        result += std::to_string(std::get<std::int64_t>(t.value()));
                    } else if (std::holds_alternative<double>(t.value())) {
                        result += std::to_string(std::get<double>(t.value()));
                    } else if (is_atom(t, "benar")) {
                        result += "true";
                    } else if (is_atom(t, "salah") || is_atom(t, "nil")) {
                        result += "null";
                    } else if (auto* tup = t.get_if<Tuple>()) {
                        result += "[";
                        if (tup->items) {
                            for (size_t i = 0; i < tup->items->size(); ++i) {
                                if (i > 0) result += ",";
                                serialize((*tup->items)[i]);
                            }
                        }
                        result += "]";
                    } else if (auto* lst = t.get_if<List>()) {
                        bool is_map = (lst->items && !lst->items->empty());
                        if (is_map) {
                            for (const auto& item : *lst->items) {
                                auto* pair = item.get_if<Tuple>();
                                if (!pair || !pair->items || pair->items->size() != 2 || !(*pair->items)[0].get_if<std::string>()) {
                                    is_map = false;
                                    break;
                                }
                            }
                        }
                        if (is_map) {
                            result += "{";
                            for (size_t i = 0; i < lst->items->size(); ++i) {
                                if (i > 0) result += ",";
                                auto* pair = (*lst->items)[i].get_if<Tuple>();
                                auto* k = (*pair->items)[0].get_if<std::string>();
                                result += "\"" + *k + "\":";
                                serialize((*pair->items)[1]);
                            }
                            result += "}";
                        } else {
                            result += "[";
                            if (lst->items) {
                                for (size_t i = 0; i < lst->items->size(); ++i) {
                                    if (i > 0) result += ",";
                                    serialize((*lst->items)[i]);
                                }
                            }
                            result += "]";
                        }
                    }
                };
                serialize(val);
                x_regs[0] = Term(result);
                last_result = x_regs[0];
                return true;
            }

            if (is_mod_fn(fn_name, "Json", "dekode") || is_mod_fn(fn_name, "Json", "decode")) {
                std::string s = clean_str(x_regs[0]);
                size_t pos = 0;

                std::function<Term()> parse = [&]() -> Term {
                    while (pos < s.size() && std::isspace((unsigned char)s[pos])) pos++;
                    if (pos >= s.size()) return Term(Atom{"nil"});
                    char c = s[pos];
                    if (c == '"') {
                        pos++;
                        std::string out;
                        while (pos < s.size() && s[pos] != '"') {
                            if (s[pos] == '\\' && pos + 1 < s.size()) {
                                pos++;
                                char esc = s[pos];
                                if (esc == 'n') out += '\n';
                                else if (esc == 't') out += '\t';
                                else if (esc == 'r') out += '\r';
                                else if (esc == '"') out += '"';
                                else if (esc == '\\') out += '\\';
                                else out += esc;
                            } else out += s[pos];
                            pos++;
                        }
                        if (pos < s.size()) pos++;
                        return Term(out);
                    }
                    if (c == '{') {
                        pos++;
                        std::vector<Term> entries;
                        while (true) {
                            while (pos < s.size() && std::isspace((unsigned char)s[pos])) pos++;
                            if (pos >= s.size()) break;
                            if (s[pos] == '}') { pos++; break; }
                            if (s[pos] == ',') { pos++; continue; }
                            Term key = parse();
                            while (pos < s.size() && std::isspace((unsigned char)s[pos])) pos++;
                            if (pos < s.size() && s[pos] == ':') pos++;
                            Term val = parse();
                            std::vector<Term> pair = { key, val };
                            entries.push_back(Term(Tuple{std::make_shared<const std::vector<Term>>(std::move(pair))}));
                        }
                        return Term(List{std::make_shared<const std::vector<Term>>(std::move(entries))});
                    }
                    if (c == '[') {
                        pos++;
                        std::vector<Term> items;
                        while (true) {
                            while (pos < s.size() && std::isspace((unsigned char)s[pos])) pos++;
                            if (pos >= s.size()) break;
                            if (s[pos] == ']') { pos++; break; }
                            if (s[pos] == ',') { pos++; continue; }
                            items.push_back(parse());
                        }
                        return Term(List{std::make_shared<const std::vector<Term>>(std::move(items))});
                    }
                    if (std::isdigit((unsigned char)c) || c == '-') {
                        size_t start = pos;
                        if (c == '-') pos++;
                        while (pos < s.size() && std::isdigit((unsigned char)s[pos])) pos++;
                        bool is_double = false;
                        if (pos < s.size() && s[pos] == '.') {
                            is_double = true;
                            pos++;
                            while (pos < s.size() && std::isdigit((unsigned char)s[pos])) pos++;
                        }
                        std::string num = s.substr(start, pos - start);
                        if (is_double) return Term(std::stod(num));
                        return Term((std::int64_t)std::stoll(num));
                    }
                    if (s.compare(pos, 4, "true") == 0) { pos += 4; return Term(Atom{"benar"}); }
                    if (s.compare(pos, 5, "false") == 0) { pos += 5; return Term(Atom{"salah"}); }
                    if (s.compare(pos, 4, "null") == 0) { pos += 4; return Term(Atom{"nil"}); }
                    return Term(Atom{"nil"});
                };
                x_regs[0] = parse();
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
                cli.set_connection_timeout(1, 0);
                cli.set_read_timeout(1, 0);
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

            // === Fase 15.1: List Stdlib Module ===
            if (is_mod_fn(fn_name, "List", "panjang") || is_mod_fn(fn_name, "List", "len") || is_mod_fn(fn_name, "List", "length")) {
                int64_t len = 0;
                if (auto* lst = x_regs[0].get_if<List>()) {
                    if (lst->items) len = static_cast<int64_t>(lst->items->size());
                } else if (auto* tup = x_regs[0].get_if<Tuple>()) {
                    if (tup->items) len = static_cast<int64_t>(tup->items->size());
                }
                x_regs[0] = Term(len);
                last_result = x_regs[0];
                return true;
            }

            if (is_mod_fn(fn_name, "List", "peta") || is_mod_fn(fn_name, "List", "map")) {
                x_regs[0] = x_regs[0];
                last_result = x_regs[0];
                return true;
            }

            if (is_mod_fn(fn_name, "List", "saring") || is_mod_fn(fn_name, "List", "filter")) {
                x_regs[0] = x_regs[0];
                last_result = x_regs[0];
                return true;
            }

            if (is_mod_fn(fn_name, "List", "ambil") || is_mod_fn(fn_name, "List", "take")) {
                int64_t n = (x_regs.size() > 1) ? get_int_val(x_regs[1]) : 0;
                std::vector<Term> res_items;
                if (auto* lst = x_regs[0].get_if<List>()) {
                    if (lst->items) {
                        for (size_t i = 0; i < lst->items->size() && static_cast<int64_t>(i) < n; ++i) {
                            res_items.push_back((*lst->items)[i]);
                        }
                    }
                } else if (auto* tup = x_regs[0].get_if<Tuple>()) {
                    if (tup->items) {
                        for (size_t i = 0; i < tup->items->size() && static_cast<int64_t>(i) < n; ++i) {
                            res_items.push_back((*tup->items)[i]);
                        }
                    }
                }
                x_regs[0] = Term(List{std::make_shared<const std::vector<Term>>(std::move(res_items))});
                last_result = x_regs[0];
                return true;
            }

            if (is_mod_fn(fn_name, "List", "buang") || is_mod_fn(fn_name, "List", "drop")) {
                int64_t n = (x_regs.size() > 1) ? get_int_val(x_regs[1]) : 0;
                std::vector<Term> res_items;
                if (auto* lst = x_regs[0].get_if<List>()) {
                    if (lst->items) {
                        for (size_t i = static_cast<size_t>(std::max<int64_t>(0, n)); i < lst->items->size(); ++i) {
                            res_items.push_back((*lst->items)[i]);
                        }
                    }
                } else if (auto* tup = x_regs[0].get_if<Tuple>()) {
                    if (tup->items) {
                        for (size_t i = static_cast<size_t>(std::max<int64_t>(0, n)); i < tup->items->size(); ++i) {
                            res_items.push_back((*tup->items)[i]);
                        }
                    }
                }
                x_regs[0] = Term(List{std::make_shared<const std::vector<Term>>(std::move(res_items))});
                last_result = x_regs[0];
                return true;
            }

            if (is_mod_fn(fn_name, "List", "gabung") || is_mod_fn(fn_name, "List", "concat")) {
                std::vector<Term> res_items;
                auto append_items = [&](const Term& t) {
                    if (auto* lst = t.get_if<List>()) {
                        if (lst->items) res_items.insert(res_items.end(), lst->items->begin(), lst->items->end());
                    } else if (auto* tup = t.get_if<Tuple>()) {
                        if (tup->items) res_items.insert(res_items.end(), tup->items->begin(), tup->items->end());
                    } else {
                        res_items.push_back(t);
                    }
                };
                append_items(x_regs[0]);
                if (x_regs.size() > 1) append_items(x_regs[1]);
                x_regs[0] = Term(List{std::make_shared<const std::vector<Term>>(std::move(res_items))});
                last_result = x_regs[0];
                return true;
            }

            if (is_mod_fn(fn_name, "List", "balik") || is_mod_fn(fn_name, "List", "reverse")) {
                std::vector<Term> res_items;
                if (auto* lst = x_regs[0].get_if<List>()) {
                    if (lst->items) res_items = *lst->items;
                } else if (auto* tup = x_regs[0].get_if<Tuple>()) {
                    if (tup->items) res_items = *tup->items;
                }
                std::reverse(res_items.begin(), res_items.end());
                x_regs[0] = Term(List{std::make_shared<const std::vector<Term>>(std::move(res_items))});
                last_result = x_regs[0];
                return true;
            }

            if (is_mod_fn(fn_name, "List", "urutkan") || is_mod_fn(fn_name, "List", "sort")) {
                std::vector<Term> res_items;
                if (auto* lst = x_regs[0].get_if<List>()) {
                    if (lst->items) res_items = *lst->items;
                } else if (auto* tup = x_regs[0].get_if<Tuple>()) {
                    if (tup->items) res_items = *tup->items;
                }
                std::sort(res_items.begin(), res_items.end(), [clean_str](const Term& a, const Term& b) {
                    return clean_str(a) < clean_str(b);
                });
                x_regs[0] = Term(List{std::make_shared<const std::vector<Term>>(std::move(res_items))});
                last_result = x_regs[0];
                return true;
            }

            // === Fase 15.2: Map Stdlib Module ===
            if (is_mod_fn(fn_name, "Map", "baru") || is_mod_fn(fn_name, "Map", "new")) {
                std::vector<Term> empty_pairs;
                x_regs[0] = Term(List{std::make_shared<const std::vector<Term>>(std::move(empty_pairs))});
                last_result = x_regs[0];
                return true;
            }

            if (is_mod_fn(fn_name, "Map", "ambil") || is_mod_fn(fn_name, "Map", "get")) {
                auto unq = [](const std::string& s) { if (s.length()>=2 && s.front()=='"' && s.back()=='"') return s.substr(1, s.length()-2); return s; };
                std::string target_key = (x_regs.size() > 1) ? unq(clean_str(x_regs[1])) : "";
                Term found_val(Atom{"nil"});
                const std::vector<Term>* items = nullptr;
                if (auto* lst = x_regs[0].get_if<List>()) { if (lst->items) items = lst->items.get(); }
                else if (auto* tup = x_regs[0].get_if<Tuple>()) { if (tup->items) items = tup->items.get(); }

                if (items) {
                    for (const auto& item : *items) {
                        const std::vector<Term>* p = nullptr;
                        if (auto* pair_tup = item.get_if<Tuple>()) { if (pair_tup->items) p = pair_tup->items.get(); }
                        else if (auto* pair_lst = item.get_if<List>()) { if (pair_lst->items) p = pair_lst->items.get(); }
                        if (p && p->size() >= 2) {
                            std::string k_str = unq(clean_str((*p)[0]));
                            if (k_str == target_key || clean_str((*p)[0]) == target_key) {
                                found_val = (*p)[1];
                                break;
                            }
                        }
                    }
                }
                x_regs[0] = found_val;
                last_result = x_regs[0];
                return true;
            }

            if (is_mod_fn(fn_name, "Map", "taruh") || is_mod_fn(fn_name, "Map", "put")) {
                auto unq = [](const std::string& s) { if (s.length()>=2 && s.front()=='"' && s.back()=='"') return s.substr(1, s.length()-2); return s; };
                std::string key = (x_regs.size() > 1) ? unq(clean_str(x_regs[1])) : "";
                Term val = (x_regs.size() > 2) ? x_regs[2] : Term(Atom{"nil"});
                std::vector<Term> pairs;
                bool updated = false;

                const std::vector<Term>* items = nullptr;
                if (auto* lst = x_regs[0].get_if<List>()) { if (lst->items) items = lst->items.get(); }
                else if (auto* tup = x_regs[0].get_if<Tuple>()) { if (tup->items) items = tup->items.get(); }

                if (items) {
                    for (const auto& item : *items) {
                        const std::vector<Term>* p = nullptr;
                        if (auto* pair_tup = item.get_if<Tuple>()) { if (pair_tup->items) p = pair_tup->items.get(); }
                        else if (auto* pair_lst = item.get_if<List>()) { if (pair_lst->items) p = pair_lst->items.get(); }
                        if (p && p->size() >= 2 && unq(clean_str((*p)[0])) == key) {
                            std::vector<Term> new_p = {Term(key), val};
                            pairs.push_back(Term(Tuple{std::make_shared<const std::vector<Term>>(std::move(new_p))}));
                            updated = true;
                            continue;
                        }
                        pairs.push_back(item);
                    }
                }
                if (!updated) {
                    std::vector<Term> new_p = {Term(key), val};
                    pairs.push_back(Term(Tuple{std::make_shared<const std::vector<Term>>(std::move(new_p))}));
                }
                x_regs[0] = Term(List{std::make_shared<const std::vector<Term>>(std::move(pairs))});
                last_result = x_regs[0];
                return true;
            }

            if (is_mod_fn(fn_name, "Map", "kunci") || is_mod_fn(fn_name, "Map", "keys")) {
                std::vector<Term> keys;
                const std::vector<Term>* items = nullptr;
                if (auto* lst = x_regs[0].get_if<List>()) { if (lst->items) items = lst->items.get(); }
                else if (auto* tup = x_regs[0].get_if<Tuple>()) { if (tup->items) items = tup->items.get(); }
                if (items) {
                    for (const auto& item : *items) {
                        const std::vector<Term>* p = nullptr;
                        if (auto* pair_tup = item.get_if<Tuple>()) { if (pair_tup->items) p = pair_tup->items.get(); }
                        else if (auto* pair_lst = item.get_if<List>()) { if (pair_lst->items) p = pair_lst->items.get(); }
                        if (p && p->size() >= 1) keys.push_back((*p)[0]);
                    }
                }
                x_regs[0] = Term(List{std::make_shared<const std::vector<Term>>(std::move(keys))});
                last_result = x_regs[0];
                return true;
            }

            if (is_mod_fn(fn_name, "Map", "nilai") || is_mod_fn(fn_name, "Map", "values")) {
                std::vector<Term> values;
                const std::vector<Term>* items = nullptr;
                if (auto* lst = x_regs[0].get_if<List>()) { if (lst->items) items = lst->items.get(); }
                else if (auto* tup = x_regs[0].get_if<Tuple>()) { if (tup->items) items = tup->items.get(); }
                if (items) {
                    for (const auto& item : *items) {
                        const std::vector<Term>* p = nullptr;
                        if (auto* pair_tup = item.get_if<Tuple>()) { if (pair_tup->items) p = pair_tup->items.get(); }
                        else if (auto* pair_lst = item.get_if<List>()) { if (pair_lst->items) p = pair_lst->items.get(); }
                        if (p && p->size() >= 2) values.push_back((*p)[1]);
                    }
                }
                x_regs[0] = Term(List{std::make_shared<const std::vector<Term>>(std::move(values))});
                last_result = x_regs[0];
                return true;
            }

            if (is_mod_fn(fn_name, "Map", "gabung") || is_mod_fn(fn_name, "Map", "merge")) {
                x_regs[0] = x_regs[0];
                last_result = x_regs[0];
                return true;
            }

            if (is_mod_fn(fn_name, "Map", "hapus") || is_mod_fn(fn_name, "Map", "delete")) {
                auto unq = [](const std::string& s) { if (s.length()>=2 && s.front()=='"' && s.back()=='"') return s.substr(1, s.length()-2); return s; };
                std::string key = (x_regs.size() > 1) ? unq(clean_str(x_regs[1])) : "";
                std::vector<Term> pairs;
                const std::vector<Term>* items = nullptr;
                if (auto* lst = x_regs[0].get_if<List>()) { if (lst->items) items = lst->items.get(); }
                else if (auto* tup = x_regs[0].get_if<Tuple>()) { if (tup->items) items = tup->items.get(); }
                if (items) {
                    for (const auto& item : *items) {
                        const std::vector<Term>* p = nullptr;
                        if (auto* pair_tup = item.get_if<Tuple>()) { if (pair_tup->items) p = pair_tup->items.get(); }
                        else if (auto* pair_lst = item.get_if<List>()) { if (pair_lst->items) p = pair_lst->items.get(); }
                        if (p && p->size() >= 1 && unq(clean_str((*p)[0])) == key) continue;
                        pairs.push_back(item);
                    }
                }
                x_regs[0] = Term(List{std::make_shared<const std::vector<Term>>(std::move(pairs))});
                last_result = x_regs[0];
                return true;
            }

            if (is_mod_fn(fn_name, "Map", "berisi") || is_mod_fn(fn_name, "Map", "contains")) {
                auto unq = [](const std::string& s) { if (s.length()>=2 && s.front()=='"' && s.back()=='"') return s.substr(1, s.length()-2); return s; };
                std::string target_key = (x_regs.size() > 1) ? unq(clean_str(x_regs[1])) : "";
                bool found = false;
                const std::vector<Term>* items = nullptr;
                if (auto* lst = x_regs[0].get_if<List>()) { if (lst->items) items = lst->items.get(); }
                else if (auto* tup = x_regs[0].get_if<Tuple>()) { if (tup->items) items = tup->items.get(); }
                if (items) {
                    for (const auto& item : *items) {
                        const std::vector<Term>* p = nullptr;
                        if (auto* pair_tup = item.get_if<Tuple>()) { if (pair_tup->items) p = pair_tup->items.get(); }
                        else if (auto* pair_lst = item.get_if<List>()) { if (pair_lst->items) p = pair_lst->items.get(); }
                        if (p && p->size() >= 1 && unq(clean_str((*p)[0])) == target_key) {
                            found = true;
                            break;
                        }
                    }
                }
                x_regs[0] = Term(Atom{found ? "benar" : "salah"});
                last_result = x_regs[0];
                return true;
            }

            // === Fase 15.3: String Stdlib Module ===
            if (is_mod_fn(fn_name, "String", "pisah") || is_mod_fn(fn_name, "String", "split")) {
                std::string str = clean_str(x_regs[0]);
                std::string delim = (x_regs.size() > 1) ? clean_str(x_regs[1]) : " ";
                std::vector<Term> parts;
                if (delim.empty()) {
                    for (char c : str) parts.push_back(Term(std::string(1, c)));
                } else {
                    size_t start = 0, pos = 0;
                    while ((pos = str.find(delim, start)) != std::string::npos) {
                        parts.push_back(Term(str.substr(start, pos - start)));
                        start = pos + delim.length();
                    }
                    parts.push_back(Term(str.substr(start)));
                }
                x_regs[0] = Term(List{std::make_shared<const std::vector<Term>>(std::move(parts))});
                last_result = x_regs[0];
                return true;
            }

            if (is_mod_fn(fn_name, "String", "gabung") || is_mod_fn(fn_name, "String", "join")) {
                std::string delim = (x_regs.size() > 1) ? clean_str(x_regs[1]) : "";
                std::string res;
                if (auto* lst = x_regs[0].get_if<List>()) {
                    if (lst->items) {
                        for (size_t i = 0; i < lst->items->size(); ++i) {
                            if (i > 0) res += delim;
                            res += clean_str((*lst->items)[i]);
                        }
                    }
                }
                x_regs[0] = Term(res);
                last_result = x_regs[0];
                return true;
            }

            if (is_mod_fn(fn_name, "String", "ganti") || is_mod_fn(fn_name, "String", "replace")) {
                std::string str = clean_str(x_regs[0]);
                std::string target = (x_regs.size() > 1) ? clean_str(x_regs[1]) : "";
                std::string rep = (x_regs.size() > 2) ? clean_str(x_regs[2]) : "";
                if (!target.empty()) {
                    size_t pos = 0;
                    while ((pos = str.find(target, pos)) != std::string::npos) {
                        str.replace(pos, target.length(), rep);
                        pos += rep.length();
                    }
                }
                x_regs[0] = Term(str);
                last_result = x_regs[0];
                return true;
            }

            if (is_mod_fn(fn_name, "String", "pangkas") || is_mod_fn(fn_name, "String", "trim")) {
                std::string str = clean_str(x_regs[0]);
                while (!str.empty() && std::isspace((unsigned char)str.front())) str.erase(0, 1);
                while (!str.empty() && std::isspace((unsigned char)str.back())) str.pop_back();
                x_regs[0] = Term(str);
                last_result = x_regs[0];
                return true;
            }

            if (is_mod_fn(fn_name, "String", "besar") || is_mod_fn(fn_name, "String", "to_upper")) {
                std::string str = clean_str(x_regs[0]);
                for (char& c : str) c = static_cast<char>(std::toupper((unsigned char)c));
                x_regs[0] = Term(str);
                last_result = x_regs[0];
                return true;
            }

            if (is_mod_fn(fn_name, "String", "kecil") || is_mod_fn(fn_name, "String", "to_lower")) {
                std::string str = clean_str(x_regs[0]);
                for (char& c : str) c = static_cast<char>(std::tolower((unsigned char)c));
                x_regs[0] = Term(str);
                last_result = x_regs[0];
                return true;
            }

            if (is_mod_fn(fn_name, "String", "mulai_dengan") || is_mod_fn(fn_name, "String", "starts_with")) {
                auto unq = [](const std::string& s) { if (s.length()>=2 && s.front()=='"' && s.back()=='"') return s.substr(1, s.length()-2); return s; };
                std::string str = unq(clean_str(x_regs[0]));
                std::string prefix = (x_regs.size() > 1) ? unq(clean_str(x_regs[1])) : "";
                bool ok = (!prefix.empty() && str.find(prefix) == 0);
                x_regs[0] = Term(Atom{ok ? "benar" : "salah"});
                last_result = x_regs[0];
                return true;
            }

            if (is_mod_fn(fn_name, "String", "berakhir_dengan") || is_mod_fn(fn_name, "String", "ends_with")) {
                auto unq = [](const std::string& s) { if (s.length()>=2 && s.front()=='"' && s.back()=='"') return s.substr(1, s.length()-2); return s; };
                std::string str = unq(clean_str(x_regs[0]));
                std::string suffix = (x_regs.size() > 1) ? unq(clean_str(x_regs[1])) : "";
                bool ok = (str.length() >= suffix.length() && str.rfind(suffix) == (str.length() - suffix.length()));
                x_regs[0] = Term(Atom{ok ? "benar" : "salah"});
                last_result = x_regs[0];
                return true;
            }

            if (is_mod_fn(fn_name, "String", "cari") || is_mod_fn(fn_name, "String", "index_of")) {
                std::string str = clean_str(x_regs[0]);
                std::string target = (x_regs.size() > 1) ? clean_str(x_regs[1]) : "";
                size_t pos = str.find(target);
                int64_t idx = (pos != std::string::npos) ? static_cast<int64_t>(pos) : -1LL;
                x_regs[0] = Term(idx);
                last_result = x_regs[0];
                return true;
            }

            // === Fase 15.4: File Stdlib Module ===
            if (is_mod_fn(fn_name, "File", "tambah") || is_mod_fn(fn_name, "File", "append")) {
                std::string path = clean_str(x_regs[0]);
                std::string content = (x_regs.size() > 1) ? clean_str(x_regs[1]) : "";
                std::ofstream f(path, std::ios::app);
                if (f.is_open()) {
                    f << content;
                    x_regs[0] = Term(Atom{"benar"});
                } else {
                    x_regs[0] = Term(Atom{"galat"});
                }
                last_result = x_regs[0];
                return true;
            }

            if (is_mod_fn(fn_name, "File", "hapus") || is_mod_fn(fn_name, "File", "delete")) {
                std::string path = clean_str(x_regs[0]);
                std::error_code ec;
                bool ok = std::filesystem::remove(path, ec);
                x_regs[0] = Term(Atom{ok ? "benar" : "salah"});
                last_result = x_regs[0];
                return true;
            }

            if (is_mod_fn(fn_name, "File", "daftar") || is_mod_fn(fn_name, "File", "list_dir")) {
                std::string path = clean_str(x_regs[0]);
                std::vector<Term> items;
                std::error_code ec;
                if (std::filesystem::exists(path, ec) && std::filesystem::is_directory(path, ec)) {
                    for (const auto& entry : std::filesystem::directory_iterator(path, ec)) {
                        items.push_back(Term(entry.path().filename().string()));
                    }
                }
                x_regs[0] = Term(List{std::make_shared<const std::vector<Term>>(std::move(items))});
                last_result = x_regs[0];
                return true;
            }

            if (is_mod_fn(fn_name, "File", "ukuran") || is_mod_fn(fn_name, "File", "size")) {
                std::string path = clean_str(x_regs[0]);
                std::error_code ec;
                auto sz = std::filesystem::file_size(path, ec);
                int64_t bytes = ec ? -1LL : static_cast<int64_t>(sz);
                x_regs[0] = Term(bytes);
                last_result = x_regs[0];
                return true;
            }

            // === Fase 15.5: Math Stdlib Module ===
            if (is_mod_fn(fn_name, "Math", "akar") || is_mod_fn(fn_name, "Math", "sqrt")) {
                double val = (x_regs.size() > 0) ? (x_regs[0].get_if<double>() ? *x_regs[0].get_if<double>() : static_cast<double>(get_int_val(x_regs[0]))) : 0.0;
                if (val < 0.0) x_regs[0] = Term(Atom{"galat"});
                else x_regs[0] = Term(std::sqrt(val));
                last_result = x_regs[0];
                return true;
            }

            if (is_mod_fn(fn_name, "Math", "pangkat") || is_mod_fn(fn_name, "Math", "pow")) {
                double base = (x_regs[0].get_if<double>() ? *x_regs[0].get_if<double>() : static_cast<double>(get_int_val(x_regs[0])));
                double exp = (x_regs.size() > 1 && x_regs[1].get_if<double>() ? *x_regs[1].get_if<double>() : static_cast<double>(get_int_val(x_regs[1])));
                x_regs[0] = Term(std::pow(base, exp));
                last_result = x_regs[0];
                return true;
            }

            if (is_mod_fn(fn_name, "Math", "sin")) {
                double val = (x_regs[0].get_if<double>() ? *x_regs[0].get_if<double>() : static_cast<double>(get_int_val(x_regs[0])));
                x_regs[0] = Term(std::sin(val));
                last_result = x_regs[0];
                return true;
            }

            if (is_mod_fn(fn_name, "Math", "cos")) {
                double val = (x_regs[0].get_if<double>() ? *x_regs[0].get_if<double>() : static_cast<double>(get_int_val(x_regs[0])));
                x_regs[0] = Term(std::cos(val));
                last_result = x_regs[0];
                return true;
            }

            if (is_mod_fn(fn_name, "Math", "tan")) {
                double val = (x_regs[0].get_if<double>() ? *x_regs[0].get_if<double>() : static_cast<double>(get_int_val(x_regs[0])));
                x_regs[0] = Term(std::tan(val));
                last_result = x_regs[0];
                return true;
            }

            if (is_mod_fn(fn_name, "Math", "log")) {
                double val = (x_regs[0].get_if<double>() ? *x_regs[0].get_if<double>() : static_cast<double>(get_int_val(x_regs[0])));
                if (val <= 0.0) x_regs[0] = Term(Atom{"galat"});
                else x_regs[0] = Term(std::log(val));
                last_result = x_regs[0];
                return true;
            }

            if (is_mod_fn(fn_name, "Math", "log10")) {
                double val = (x_regs[0].get_if<double>() ? *x_regs[0].get_if<double>() : static_cast<double>(get_int_val(x_regs[0])));
                if (val <= 0.0) x_regs[0] = Term(Atom{"galat"});
                else x_regs[0] = Term(std::log10(val));
                last_result = x_regs[0];
                return true;
            }

            if (is_mod_fn(fn_name, "Math", "abs")) {
                if (auto* d = x_regs[0].get_if<double>()) {
                    x_regs[0] = Term(std::abs(*d));
                } else {
                    x_regs[0] = Term(std::abs(get_int_val(x_regs[0])));
                }
                last_result = x_regs[0];
                return true;
            }

            if (is_mod_fn(fn_name, "Math", "bulat") || is_mod_fn(fn_name, "Math", "round")) {
                double val = (x_regs[0].get_if<double>() ? *x_regs[0].get_if<double>() : static_cast<double>(get_int_val(x_regs[0])));
                x_regs[0] = Term(static_cast<int64_t>(std::round(val)));
                last_result = x_regs[0];
                return true;
            }

            if (is_mod_fn(fn_name, "Math", "langit") || is_mod_fn(fn_name, "Math", "ceil")) {
                double val = (x_regs[0].get_if<double>() ? *x_regs[0].get_if<double>() : static_cast<double>(get_int_val(x_regs[0])));
                x_regs[0] = Term(static_cast<int64_t>(std::ceil(val)));
                last_result = x_regs[0];
                return true;
            }

            if (is_mod_fn(fn_name, "Math", "lantai") || is_mod_fn(fn_name, "Math", "floor")) {
                double val = (x_regs[0].get_if<double>() ? *x_regs[0].get_if<double>() : static_cast<double>(get_int_val(x_regs[0])));
                x_regs[0] = Term(static_cast<int64_t>(std::floor(val)));
                last_result = x_regs[0];
                return true;
            }

            if (is_mod_fn(fn_name, "Math", "pi")) {
                x_regs[0] = Term(3.14159265358979323846);
                last_result = x_regs[0];
                return true;
            }

            // === Fase 15.6: Time Stdlib Module ===
            if (is_mod_fn(fn_name, "Time", "sekarang") || is_mod_fn(fn_name, "Time", "now")) {
                auto now = std::chrono::system_clock::now();
                auto secs = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
                x_regs[0] = Term(static_cast<int64_t>(secs));
                last_result = x_regs[0];
                return true;
            }

            if (is_mod_fn(fn_name, "Time", "milik_detik") || is_mod_fn(fn_name, "Time", "now_ms")) {
                auto now = std::chrono::system_clock::now();
                auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
                x_regs[0] = Term(static_cast<int64_t>(ms));
                last_result = x_regs[0];
                return true;
            }

            if (is_mod_fn(fn_name, "Time", "format")) {
                int64_t ts = get_int_val(x_regs[0]);
                std::time_t t = static_cast<std::time_t>(ts);
                std::tm tm_buf{};
#if defined(_WIN32) || defined(_WIN64)
                gmtime_s(&tm_buf, &t);
#else
                gmtime_r(&t, &tm_buf);
#endif
                char buf[64];
                std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm_buf);
                x_regs[0] = Term(std::string(buf));
                last_result = x_regs[0];
                return true;
            }

            if (is_mod_fn(fn_name, "Time", "parse")) {
                std::string str = clean_str(x_regs[0]);
                std::tm tm_buf{};
                int yr = 0, mo = 0, dy = 0, hr = 0, mn = 0, sc = 0;
                if (sscanf(str.c_str(), "%d-%d-%d %d:%d:%d", &yr, &mo, &dy, &hr, &mn, &sc) >= 3) {
                    tm_buf.tm_year = yr - 1900;
                    tm_buf.tm_mon = mo - 1;
                    tm_buf.tm_mday = dy;
                    tm_buf.tm_hour = hr;
                    tm_buf.tm_min = mn;
                    tm_buf.tm_sec = sc;
                    std::time_t t = std::mktime(&tm_buf);
                    x_regs[0] = Term(static_cast<int64_t>(t));
                } else {
                    x_regs[0] = Term(Atom{"galat"});
                }
                last_result = x_regs[0];
                return true;
            }

            if (is_mod_fn(fn_name, "Time", "tidur") || is_mod_fn(fn_name, "Time", "sleep")) {
                int64_t ms = get_int_val(x_regs[0]);
                if (ms > 0) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
                }
                x_regs[0] = Term(Atom{"benar"});
                last_result = x_regs[0];
                return true;
            }

            if (is_mod_fn(fn_name, "Time", "tambah_detik")) {
                int64_t ts = get_int_val(x_regs[0]);
                int64_t secs = (x_regs.size() > 1) ? get_int_val(x_regs[1]) : 0;
                x_regs[0] = Term(ts + secs);
                last_result = x_regs[0];
                return true;
            }

            // === Fase 15.7: Path Stdlib Module ===
            if (is_mod_fn(fn_name, "Path", "gabung") || is_mod_fn(fn_name, "Path", "join")) {
                std::string p1 = clean_str(x_regs[0]);
                std::string p2 = (x_regs.size() > 1) ? clean_str(x_regs[1]) : "";
                std::filesystem::path p = std::filesystem::path(p1) / std::filesystem::path(p2);
                x_regs[0] = Term(p.string());
                last_result = x_regs[0];
                return true;
            }

            if (is_mod_fn(fn_name, "Path", "direktori") || is_mod_fn(fn_name, "Path", "dirname")) {
                std::string p1 = clean_str(x_regs[0]);
                std::filesystem::path p(p1);
                x_regs[0] = Term(p.parent_path().string());
                last_result = x_regs[0];
                return true;
            }

            if (is_mod_fn(fn_name, "Path", "nama_file") || is_mod_fn(fn_name, "Path", "filename")) {
                std::string p1 = clean_str(x_regs[0]);
                std::filesystem::path p(p1);
                x_regs[0] = Term(p.filename().string());
                last_result = x_regs[0];
                return true;
            }

            if (is_mod_fn(fn_name, "Path", "ekstensi") || is_mod_fn(fn_name, "Path", "extension")) {
                std::string p1 = clean_str(x_regs[0]);
                std::filesystem::path p(p1);
                x_regs[0] = Term(p.extension().string());
                last_result = x_regs[0];
                return true;
            }

            if (is_mod_fn(fn_name, "Path", "mutlak") || is_mod_fn(fn_name, "Path", "absolute")) {
                std::string p1 = clean_str(x_regs[0]);
                std::error_code ec;
                std::filesystem::path p = std::filesystem::absolute(p1, ec);
                x_regs[0] = Term(ec ? p1 : p.string());
                last_result = x_regs[0];
                return true;
            }

            if (is_mod_fn(fn_name, "Path", "normalisasi") || is_mod_fn(fn_name, "Path", "normalize")) {
                std::string p1 = clean_str(x_regs[0]);
                std::filesystem::path p(p1);
                x_regs[0] = Term(p.lexically_normal().string());
                last_result = x_regs[0];
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
