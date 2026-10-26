#pragma once
// Garuda Standard Library: Native HTTP/1.1 Server & Router Engine (`http_server.hpp`)
// Mengimplementasikan Actor-per-Connection Worker Engine, Dynamic Path Parameters, Cookie Parsing/Setting, & WebSocket Upgrade.
#include "../core/term.hpp"
#include "http.hpp"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#endif

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace garuda::beam::stdlib {

struct HttpRequest {
    std::string method = "GET";
    std::string path = "/";
    std::map<std::string, std::string> headers;
    std::map<std::string, std::string> path_params;
    std::map<std::string, std::string> cookies;
    std::string body;
};

using HttpHandler = std::function<HttpResponse(const HttpRequest&)>;

class Router {
public:
    void get(const std::string& path, HttpHandler handler) {
        routes_["GET:" + path] = std::move(handler);
    }

    void post(const std::string& path, HttpHandler handler) {
        routes_["POST:" + path] = std::move(handler);
    }

    HttpResponse dispatch(HttpRequest& req) const {
        std::string req_route = req.method + ":" + req.path;

        // 1. Direct exact match
        auto it = routes_.find(req_route);
        if (it != routes_.end()) {
            return it->second(req);
        }

        // 2. Dynamic path parameter pattern matching (/users/:id)
        for (const auto& [pattern_key, handler] : routes_) {
            auto colon_pos = pattern_key.find(':');
            if (colon_pos == std::string::npos) continue;

            std::string pattern_method = pattern_key.substr(0, colon_pos);
            std::string pattern_path = pattern_key.substr(colon_pos + 1);

            if (pattern_method != req.method) continue;

            std::map<std::string, std::string> extracted_params;
            if (match_path_pattern(pattern_path, req.path, extracted_params)) {
                req.path_params = std::move(extracted_params);
                return handler(req);
            }
        }

        // 3. Unmatched route -> 404 Not Found
        HttpResponse res;
        res.status_code = 404;
        res.status_text = "Not Found";
        res.body = "404 Not Found";
        return res;
    }

private:
    static bool match_path_pattern(
        const std::string& pattern,
        const std::string& actual_path,
        std::map<std::string, std::string>& params
    ) {
        std::vector<std::string> p_tokens = split_path(pattern);
        std::vector<std::string> a_tokens = split_path(actual_path);

        if (p_tokens.size() != a_tokens.size()) return false;

        for (std::size_t i = 0; i < p_tokens.size(); ++i) {
            if (!p_tokens[i].empty() && p_tokens[i].front() == ':') {
                std::string param_name = p_tokens[i].substr(1);
                params[param_name] = a_tokens[i];
            } else if (p_tokens[i] != a_tokens[i]) {
                return false;
            }
        }
        return true;
    }

    static std::vector<std::string> split_path(const std::string& path) {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(path);
        while (std::getline(tokenStream, token, '/')) {
            if (!token.empty()) {
                tokens.push_back(token);
            }
        }
        return tokens;
    }

    std::map<std::string, HttpHandler> routes_;
};

#ifdef _WIN32
using SocketHandle = SOCKET;
#else
using SocketHandle = int;
#endif

class HttpServer {
public:
    HttpServer() = default;
    ~HttpServer() { stop(); }

    static HttpResponse make_ok(const std::string& body, const std::string& content_type = "text/plain") {
        HttpResponse res;
        res.status_code = 200;
        res.status_text = "OK";
        res.headers["content-type"] = content_type;
        res.body = body;
        return res;
    }

    static HttpResponse make_json(const std::string& json_str) {
        return make_ok(json_str, "application/json");
    }

    static void register_cluster_endpoints(
        Router& router,
        const std::string& leader_id,
        std::uint64_t term,
        std::size_t log_size,
        std::function<void(const std::string& node_id)> on_join = nullptr
    ) {
        router.get("/cluster/status", [leader_id, term, log_size](const HttpRequest&) {
            std::ostringstream ss;
            ss << "{\"cluster_status\":\"ok\",\"leader\":\"" << leader_id << "\",\"term\":" << term << ",\"log_size\":" << log_size << "}";
            return make_json(ss.str());
        });

        router.post("/cluster/join", [on_join](const HttpRequest& req) {
            std::string node_id = "new_node";
            auto pos = req.body.find("\"node_id\":");
            if (pos != std::string::npos) {
                auto q1 = req.body.find('"', pos + 10);
                auto q2 = req.body.find('"', q1 + 1);
                if (q1 != std::string::npos && q2 != std::string::npos) {
                    node_id = req.body.substr(q1 + 1, q2 - q1 - 1);
                }
            }
            if (on_join) on_join(node_id);
            return make_json("{\"status\":\"joined\",\"node_id\":\"" + node_id + "\"}");
        });
    }

    static HttpResponse make_websocket_upgrade_response(const std::string& sec_ws_key) {
        std::string sec_accept = sec_ws_key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
        std::size_t h = std::hash<std::string>{}(sec_accept);
        std::string accept_key = std::to_string(h);

        HttpResponse res;
        res.status_code = 101;
        res.status_text = "Switching Protocols";
        res.headers["Upgrade"] = "websocket";
        res.headers["Connection"] = "Upgrade";
        res.headers["Sec-WebSocket-Accept"] = accept_key;
        res.body = "";
        return res;
    }

    static HttpResponse make_not_found(const std::string& body = "404 Not Found") {
        HttpResponse res;
        res.status_code = 404;
        res.status_text = "Not Found";
        res.headers["content-type"] = "text/plain";
        res.body = body;
        return res;
    }

    bool start(int port, Router router, std::size_t worker_actor_count = 64) {
        router_ = std::move(router);
        port_ = port;

#ifdef _WIN32
        WSADATA wsa;
        WSAStartup(MAKEWORD(2, 2), &wsa);
        listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
        if (listen_fd_ == INVALID_SOCKET) return false;
#else
        listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
        if (listen_fd_ < 0) return false;
#endif

        int opt = 1;
        setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        addr.sin_port = htons(static_cast<uint16_t>(port));

        if (bind(listen_fd_, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
            stop();
            return false;
        }

        if (listen(listen_fd_, 1024) != 0) {
            stop();
            return false;
        }

        running_ = true;

        // HIGH CONCURRENCY: Start Worker Pool directly calling accept() to bypass Queue Mutex bottleneck!
        for (std::size_t i = 0; i < worker_actor_count; ++i) {
            worker_actors_.emplace_back([this]() { worker_actor_loop(); });
        }

        return true;
    }

    void stop() {
        if (!running_.exchange(false)) return;

#ifdef _WIN32
        if (listen_fd_ != INVALID_SOCKET) {
            closesocket(listen_fd_);
            listen_fd_ = INVALID_SOCKET;
        }
#else
        if (listen_fd_ >= 0) {
            close(listen_fd_);
            listen_fd_ = -1;
        }
#endif

        for (auto& worker : worker_actors_) {
            if (worker.joinable()) worker.join();
        }
        worker_actors_.clear();
    }

    bool is_running() const { return running_; }
    int port() const { return port_; }

private:
    void worker_actor_loop() {
        while (running_) {
            sockaddr_in client_addr{};
#ifdef _WIN32
            int client_len = sizeof(client_addr);
            SocketHandle client_fd = accept(listen_fd_, (struct sockaddr*)&client_addr, &client_len);
            if (client_fd == INVALID_SOCKET) {
                if (!running_) break;
                continue;
            }
#else
            socklen_t client_len = sizeof(client_addr);
            SocketHandle client_fd = accept(listen_fd_, (struct sockaddr*)&client_addr, &client_len);
            if (client_fd < 0) {
                if (!running_) break;
                continue;
            }
#endif
            handle_client(client_fd);
        }
    }

    void handle_client(SocketHandle client_fd) {
        int nodelay = 1;
        setsockopt(client_fd, IPPROTO_TCP, TCP_NODELAY, (const char*)&nodelay, sizeof(nodelay));

        char buf[4096];
        while (running_) {
            int bytes = recv(client_fd, buf, sizeof(buf) - 1, 0);
            if (bytes <= 0) break; // Connection closed or error

            buf[bytes] = '\0';
            std::string raw_req(buf);
            auto req_opt = parse_request(raw_req);
            if (req_opt.has_value()) {
                HttpResponse resp = router_.dispatch(*req_opt);

                // Determine if we should keep-alive
                bool keep_alive = false;
                auto it = req_opt->headers.find("connection");
                if (it != req_opt->headers.end() && it->second == "keep-alive") {
                    keep_alive = true;
                }

                std::string resp_str = format_response(resp, keep_alive);
                send(client_fd, resp_str.c_str(), static_cast<int>(resp_str.length()), 0);

                if (!keep_alive) break;
            } else {
                break; // Invalid request
            }
        }
#ifdef _WIN32
        closesocket(client_fd);
#else
        close(client_fd);
#endif
    }

    static std::string trim(const std::string& str) {
        std::size_t first = str.find_first_not_of(" \t\n\r");
        if (first == std::string::npos) return "";
        std::size_t last = str.find_last_not_of(" \t\n\r");
        return str.substr(first, (last - first + 1));
    }

    static std::optional<HttpRequest> parse_request(const std::string& raw) {
        HttpRequest req;
        const char* p = raw.c_str();
        const char* end = p + raw.size();

        // 1. Method
        const char* m_end = strchr(p, ' ');
        if (!m_end) return std::nullopt;
        req.method.assign(p, m_end - p);
        p = m_end + 1;

        // 2. Path
        const char* p_end = strchr(p, ' ');
        if (!p_end) return std::nullopt;
        req.path.assign(p, p_end - p);
        p = p_end + 1;

        // 3. HTTP Version (skip to \r\n)
        const char* v_end = strstr(p, "\r\n");
        if (!v_end) return std::nullopt;
        p = v_end + 2;

        // 4. Headers
        while (p < end && *p != '\r') {
            const char* colon = strchr(p, ':');
            const char* line_end = strstr(p, "\r\n");
            if (!colon || !line_end || colon > line_end) break;

            std::string k(p, colon - p);
            for (char& c : k) {
                if (c >= 'A' && c <= 'Z') c += ('a' - 'A'); // Fast tolower
            }

            const char* val_start = colon + 1;
            while (val_start < line_end && *val_start == ' ') val_start++;
            req.headers[k] = std::string(val_start, line_end - val_start);
            p = line_end + 2;
        }

        // Skip \r\n
        if (p + 2 <= end) p += 2;

        // 5. Body
        if (p < end) req.body.assign(p, end - p);

        // Cookie Parsing
        auto c_it = req.headers.find("cookie");
        if (c_it != req.headers.end()) {
            std::istringstream c_ss(c_it->second);
            std::string cookie_pair;
            while (std::getline(c_ss, cookie_pair, ';')) {
                auto eq = cookie_pair.find('=');
                if (eq != std::string::npos) {
                    std::string ck = trim(cookie_pair.substr(0, eq));
                    std::string cv = trim(cookie_pair.substr(eq + 1));
                    req.cookies[ck] = cv;
                }
            }
        }

        return req;
    }

    static std::string format_response(const HttpResponse& resp, bool keep_alive = false) {
        std::string out;
        out.reserve(256 + resp.body.length());

        out += "HTTP/1.1 ";
        out += std::to_string(resp.status_code);
        out += " ";
        out += resp.status_text;
        out += "\r\nContent-Length: ";
        out += std::to_string(resp.body.length());
        out += "\r\n";

        for (const auto& [k, v] : resp.headers) {
            out += k;
            out += ": ";
            out += v;
            out += "\r\n";
        }
        for (const auto& [ck, cv] : resp.set_cookies) {
            out += "Set-Cookie: ";
            out += ck;
            out += "=";
            out += cv;
            out += "; HttpOnly; Path=/\r\n";
        }
        if (keep_alive) {
            out += "Connection: keep-alive\r\n\r\n";
        } else {
            out += "Connection: close\r\n\r\n";
        }
        out += resp.body;
        return out;
    }

#ifdef _WIN32
    SocketHandle listen_fd_ = INVALID_SOCKET;
#else
    SocketHandle listen_fd_ = -1;
#endif
    int port_ = 0;
    std::atomic<bool> running_{false};
    Router router_;
    std::thread server_thread_;

    // Actor Worker Pool Queue & Condition Variable
    std::mutex queue_mu_;
    std::condition_variable queue_cv_;
    std::queue<SocketHandle> connection_queue_;
    std::vector<std::thread> worker_actors_;
};

} // namespace garuda::beam::stdlib
