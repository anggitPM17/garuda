#pragma once
// Garuda BEAM-core: HTTP/1.1 & WebSocket Server
// Non-blocking TCP socket server terintegrasi dengan EventLoop & Runtime Garuda.
#include "event_loop.hpp"
#include "../core/runtime.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#ifdef DELETE
#undef DELETE
#endif
#ifdef OPTIONAL
#undef OPTIONAL
#endif
using socket_t = SOCKET;
#define INVALID_SOCK INVALID_SOCKET
#define CLOSE_SOCK(s) ::closesocket(s)
#else
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
using socket_t = int;
#define INVALID_SOCK -1
#define CLOSE_SOCK(s) ::close(s)
#endif

namespace garuda::beam {

enum class HttpMethod { Get, Post, Put, Delete, Options, Unknown };

struct HttpRequest {
    HttpMethod method = HttpMethod::Unknown;
    std::string path;
    std::map<std::string, std::string> headers;
    std::string body;
    std::map<std::string, std::string> query_params;
};

struct HttpResponse {
    int status_code = 200;
    std::string status_text = "OK";
    std::map<std::string, std::string> headers;
    std::string body;

    std::string to_string() const {
        std::ostringstream ss;
        ss << "HTTP/1.1 " << status_code << " " << status_text << "\r\n";
        ss << "Content-Length: " << body.length() << "\r\n";
        ss << "Connection: close\r\n";
        for (const auto& [k, v] : headers) {
            ss << k << ": " << v << "\r\n";
        }
        ss << "\r\n" << body;
        return ss.str();
    }
};

using HttpHandler = std::function<HttpResponse(const HttpRequest&)>;
using WsHandler = std::function<void(const std::string& message, std::function<void(const std::string&)> send_reply)>;

class HttpWsServer {
public:
    HttpWsServer(EventLoop& loop, int port)
        : loop_(loop), port_(port), server_fd_(INVALID_SOCK) {
        init_winsock();
    }

    ~HttpWsServer() {
        stop();
    }

    void set_http_handler(HttpHandler handler) {
        http_handler_ = std::move(handler);
    }

    void set_ws_handler(WsHandler handler) {
        ws_handler_ = std::move(handler);
    }

    bool start() {
        server_fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd_ == INVALID_SOCK) return false;

        int opt = 1;
        ::setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(static_cast<uint16_t>(port_));

        if (::bind(server_fd_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            CLOSE_SOCK(server_fd_);
            server_fd_ = INVALID_SOCK;
            return false;
        }

        if (::listen(server_fd_, 128) < 0) {
            CLOSE_SOCK(server_fd_);
            server_fd_ = INVALID_SOCK;
            return false;
        }

        set_nonblocking(server_fd_);

        Pid listener_pid = loop_.runtime().spawn([this](Process& p) {
            auto msg = p.receive();
            if (msg) {
                this->accept_connections();
            }
            return Status::Wait;
        });

        loop_.watch_read(static_cast<int>(server_fd_), listener_pid, atom("accept"));
        std::cout << "🚀 [GARUDA SERVER] Listening on http/ws://0.0.0.0:" << port_ << std::endl;
        return true;
    }

    void stop() {
        if (server_fd_ != INVALID_SOCK) {
            loop_.unwatch(static_cast<int>(server_fd_));
            CLOSE_SOCK(server_fd_);
            server_fd_ = INVALID_SOCK;
        }
    }

    void send_ws_text_frame(socket_t client_fd, const std::string& payload) {
        std::vector<std::uint8_t> frame;
        frame.push_back(0x81); // FIN (1) + Opcode (1 = Text)

        std::size_t len = payload.length();
        if (len <= 125) {
            frame.push_back(static_cast<std::uint8_t>(len));
        } else if (len <= 65535) {
            frame.push_back(126);
            frame.push_back(static_cast<std::uint8_t>((len >> 8) & 0xFF));
            frame.push_back(static_cast<std::uint8_t>(len & 0xFF));
        } else {
            frame.push_back(127);
            for (int i = 7; i >= 0; --i) {
                frame.push_back(static_cast<std::uint8_t>((len >> (i * 8)) & 0xFF));
            }
        }

        for (char c : payload) {
            frame.push_back(static_cast<std::uint8_t>(c));
        }

        ::send(client_fd, reinterpret_cast<const char*>(frame.data()), static_cast<int>(frame.size()), 0);
    }

private:
    void init_winsock() {
#if defined(_WIN32)
        static bool wsa_inited = false;
        if (!wsa_inited) {
            WSADATA wsaData;
            WSAStartup(MAKEWORD(2, 2), &wsaData);
            wsa_inited = true;
        }
#endif
    }

    void set_nonblocking(socket_t s) {
#if defined(_WIN32)
        u_long mode = 1;
        ioctlsocket(s, FIONBIO, &mode);
#else
        int flags = fcntl(s, F_GETFL, 0);
        fcntl(s, F_SETFL, flags | O_NONBLOCK);
#endif
    }

    void accept_connections() {
        while (true) {
            sockaddr_in client_addr{};
            socklen_t addr_len = sizeof(client_addr);
            socket_t client_fd = ::accept(server_fd_, (struct sockaddr*)&client_addr, &addr_len);
            if (client_fd == INVALID_SOCK) break;

            set_nonblocking(client_fd);
            handle_client_async(client_fd);
        }
    }

    void handle_client_async(socket_t client_fd) {
        Pid client_pid = loop_.runtime().spawn([this, client_fd](Process& p) {
            auto msg = p.receive();
            if (msg) {
                this->read_client_data(client_fd);
            }
            return Status::Wait;
        });

        loop_.watch_read(static_cast<int>(client_fd), client_pid, atom("read"));
    }

    void read_client_data(socket_t client_fd) {
        char buffer[8192];
        int bytes = ::recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) {
            ws_sockets_.erase(client_fd);
            loop_.unwatch(static_cast<int>(client_fd));
            CLOSE_SOCK(client_fd);
            return;
        }

        // 1. Check if this socket is already an established WebSocket connection
        if (ws_sockets_.count(client_fd) > 0) {
            const std::uint8_t* ubuf = reinterpret_cast<const std::uint8_t*>(buffer);
            std::uint8_t opcode = ubuf[0] & 0x0F;
            bool masked = (ubuf[1] & 0x80) != 0;
            std::size_t payload_len = ubuf[1] & 0x7F;
            std::size_t offset = 2;

            if (payload_len == 126) {
                if (bytes >= 4) {
                    payload_len = (ubuf[2] << 8) | ubuf[3];
                    offset = 4;
                }
            } else if (payload_len == 127) {
                if (bytes >= 10) {
                    offset = 10;
                }
            }

            std::uint8_t mask_key[4] = {0, 0, 0, 0};
            if (masked) {
                if (bytes >= offset + 4) {
                    for (int i = 0; i < 4; ++i) mask_key[i] = ubuf[offset++];
                }
            }

            std::string payload;
            payload.reserve(payload_len);
            for (std::size_t i = 0; i < payload_len && (offset + i) < static_cast<std::size_t>(bytes); ++i) {
                std::uint8_t b = ubuf[offset + i];
                if (masked) b ^= mask_key[i % 4];
                payload += static_cast<char>(b);
            }

            if (opcode == 0x1) { // Text Frame
                if (ws_handler_) {
                    ws_handler_(payload, [this, client_fd](const std::string& reply) {
                        this->send_ws_text_frame(client_fd, reply);
                    });
                } else {
                    send_ws_text_frame(client_fd, "Pong: " + payload);
                }
            } else if (opcode == 0x8) { // Close Frame
                ws_sockets_.erase(client_fd);
                loop_.unwatch(static_cast<int>(client_fd));
                CLOSE_SOCK(client_fd);
            }
            return;
        }

        buffer[bytes] = '\0';
        std::string raw_req(buffer, bytes);

        // 2. Simple HTTP / WS Handshake Parser
        if (raw_req.find("Sec-WebSocket-Key") != std::string::npos) {
            std::string key = extract_header_value(raw_req, "Sec-WebSocket-Key");
            std::string accept_key = compute_ws_accept(key);
            std::ostringstream response;
            response << "HTTP/1.1 101 Switching Protocols\r\n"
                     << "Upgrade: websocket\r\n"
                     << "Connection: Upgrade\r\n"
                     << "Sec-WebSocket-Accept: " << accept_key << "\r\n\r\n";
            std::string resp_str = response.str();
            ::send(client_fd, resp_str.c_str(), static_cast<int>(resp_str.length()), 0);

            // Register socket as active WebSocket connection
            ws_sockets_.insert(client_fd);
        } else {
            HttpRequest req = parse_http_request(raw_req);
            HttpResponse resp;
            if (http_handler_) {
                resp = http_handler_(req);
            } else {
                resp.status_code = 200;
                resp.body = "{\"status\":\"ok\",\"engine\":\"Garuda GVM v67.0.0\"}";
                resp.headers["Content-Type"] = "application/json";
            }
            std::string resp_str = resp.to_string();
            ::send(client_fd, resp_str.c_str(), static_cast<int>(resp_str.length()), 0);
            loop_.unwatch(static_cast<int>(client_fd));
            CLOSE_SOCK(client_fd);
        }
    }

    HttpRequest parse_http_request(const std::string& raw) {
        HttpRequest req;
        std::istringstream stream(raw);
        std::string line;
        if (std::getline(stream, line)) {
            std::istringstream line_stream(line);
            std::string method_str, full_path;
            line_stream >> method_str >> full_path;

            if (method_str == "GET") req.method = HttpMethod::Get;
            else if (method_str == "POST") req.method = HttpMethod::Post;
            else if (method_str == "PUT") req.method = HttpMethod::Put;
            else if (method_str == "DELETE") req.method = HttpMethod::Delete;
            else if (method_str == "OPTIONS") req.method = HttpMethod::Options;

            auto pos = full_path.find('?');
            if (pos != std::string::npos) {
                req.path = full_path.substr(0, pos);
                std::string q = full_path.substr(pos + 1);
                std::istringstream q_stream(q);
                std::string param;
                while (std::getline(q_stream, param, '&')) {
                    auto eq = param.find('=');
                    if (eq != std::string::npos) {
                        req.query_params[param.substr(0, eq)] = param.substr(eq + 1);
                    }
                }
            } else {
                req.path = full_path;
            }
        }

        while (std::getline(stream, line) && line != "\r" && !line.empty()) {
            auto pos = line.find(':');
            if (pos != std::string::npos) {
                std::string k = line.substr(0, pos);
                std::string v = line.substr(pos + 1);
                if (!v.empty() && v.back() == '\r') v.pop_back();
                while (!v.empty() && v.front() == ' ') v.erase(0, 1);
                req.headers[k] = v;
            }
        }

        auto body_pos = raw.find("\r\n\r\n");
        if (body_pos != std::string::npos) {
            req.body = raw.substr(body_pos + 4);
        }

        return req;
    }

    std::string extract_header_value(const std::string& raw, const std::string& key) {
        auto pos = raw.find(key + ":");
        if (pos == std::string::npos) return "";
        auto end_line = raw.find("\r\n", pos);
        std::string val = raw.substr(pos + key.length() + 1, end_line - (pos + key.length() + 1));
        while (!val.empty() && val.front() == ' ') val.erase(0, 1);
        return val;
    }

    std::string compute_ws_accept(const std::string& key) {
        return key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    }

    EventLoop& loop_;
    int port_;
    socket_t server_fd_;
    HttpHandler http_handler_;
    WsHandler ws_handler_;
    std::set<socket_t> ws_sockets_;
};

} // namespace garuda::beam
