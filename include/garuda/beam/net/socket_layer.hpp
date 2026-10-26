#pragma once
// Garuda Distributed Engine: Cross-Platform TCP Socket Transport Layer (`socket_layer.hpp`)
// Mengimplementasikan TCP Socket listener, Client Connector, Handshake Auth, dan Line-Based Protocol.

#ifdef _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #pragma comment(lib, "ws2_32.lib")
  using socket_t = SOCKET;
  #define IS_INVALID_SOCKET(s) ((s) == INVALID_SOCKET)
  #define CLOSE_SOCKET(s) closesocket(s)
#else
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <unistd.h>
  #include <fcntl.h>
  using socket_t = int;
  #define IS_INVALID_SOCKET(s) ((s) < 0)
  #define CLOSE_SOCKET(s) ::close(s)
#endif

#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <iostream>
#include <map>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace garuda::beam {

class SocketLayer {
public:
    using MessageCallback = std::function<void(const std::string& from_node, const std::string& msg)>;

    SocketLayer(std::string node_id, std::uint16_t listen_port)
        : node_id_(std::move(node_id)), listen_port_(listen_port) {
#ifdef _WIN32
        WSADATA wsa_data;
        WSAStartup(MAKEWORD(2, 2), &wsa_data);
#endif
    }

    ~SocketLayer() {
        stop();
#ifdef _WIN32
        WSACleanup();
#endif
    }

    bool start() {
        if (running_) return true;

        listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
        if (IS_INVALID_SOCKET(listen_fd_)) return false;

        int opt = 1;
#ifdef _WIN32
        setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt));
#else
        setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 127.0.0.1
        addr.sin_port = htons(listen_port_);

        if (bind(listen_fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
            CLOSE_SOCKET(listen_fd_);
            return false;
        }

        if (listen(listen_fd_, 10) < 0) {
            CLOSE_SOCKET(listen_fd_);
            return false;
        }

        running_ = true;
        accept_thread_ = std::thread(&SocketLayer::accept_loop, this);

        std::cout << "🌐 [GARUDA SOCKET] Node " << node_id_ << " listening on TCP 127.0.0.1:" << listen_port_ << std::endl;
        return true;
    }

    bool connect_to_peer(const std::string& peer_id, const std::string& host, std::uint16_t port) {
        socket_t client_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (IS_INVALID_SOCKET(client_fd)) return false;

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        inet_pton(AF_INET, host.c_str(), &addr.sin_addr);

        if (connect(client_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
            CLOSE_SOCKET(client_fd);
            return false;
        }

        // Send Handshake
        std::string handshake = "HELLO " + node_id_ + " garuda_secret_2026\n";
        ::send(client_fd, handshake.data(), static_cast<int>(handshake.length()), 0);

        {
            std::lock_guard<std::mutex> lock(mu_);
            peer_fds_[peer_id] = client_fd;
        }

        reader_threads_.push_back(std::thread(&SocketLayer::read_loop, this, client_fd, peer_id));
        std::cout << "🔗 [GARUDA SOCKET] Connected to peer " << peer_id << " at " << host << ":" << port << std::endl;
        return true;
    }

    void send(const std::string& peer_id, const std::string& msg) {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = peer_fds_.find(peer_id);
        if (it != peer_fds_.end()) {
            std::string payload = msg + "\n";
            ::send(it->second, payload.data(), static_cast<int>(payload.length()), 0);
        }
    }

    void set_message_callback(MessageCallback cb) {
        std::lock_guard<std::mutex> lock(mu_);
        msg_cb_ = std::move(cb);
    }

    void stop() {
        if (!running_) return;
        running_ = false;

        if (!IS_INVALID_SOCKET(listen_fd_)) {
            CLOSE_SOCKET(listen_fd_);
            listen_fd_ = static_cast<socket_t>(-1);
        }

        {
            std::lock_guard<std::mutex> lock(mu_);
            for (auto& [peer, fd] : peer_fds_) {
                if (!IS_INVALID_SOCKET(fd)) CLOSE_SOCKET(fd);
            }
            peer_fds_.clear();
        }

        if (accept_thread_.joinable()) accept_thread_.join();
        for (auto& t : reader_threads_) {
            if (t.joinable()) t.join();
        }
        reader_threads_.clear();
    }

private:
    void accept_loop() {
        while (running_) {
            sockaddr_in client_addr{};
            socklen_t addr_len = sizeof(client_addr);
            socket_t conn_fd = accept(listen_fd_, reinterpret_cast<sockaddr*>(&client_addr), &addr_len);
            if (IS_INVALID_SOCKET(conn_fd)) {
                if (!running_) break;
                continue;
            }

            // Read Handshake
            char buf[256] = {0};
            int bytes = recv(conn_fd, buf, sizeof(buf) - 1, 0);
            if (bytes <= 0) {
                CLOSE_SOCKET(conn_fd);
                continue;
            }

            std::string hs_line(buf);
            std::istringstream ss(hs_line);
            std::string tag, peer_id, secret;
            ss >> tag >> peer_id >> secret;

            if (tag == "HELLO" && secret == "garuda_secret_2026") {
                {
                    std::lock_guard<std::mutex> lock(mu_);
                    peer_fds_[peer_id] = conn_fd;
                }
                reader_threads_.push_back(std::thread(&SocketLayer::read_loop, this, conn_fd, peer_id));
                std::cout << "🤝 [GARUDA SOCKET] Accepted authenticated peer " << peer_id << std::endl;
            } else {
                CLOSE_SOCKET(conn_fd);
            }
        }
    }

    void read_loop(socket_t fd, std::string peer_id) {
        std::string buffer;
        char chunk[1024];

        while (running_) {
            int bytes = recv(fd, chunk, sizeof(chunk) - 1, 0);
            if (bytes <= 0) break;

            chunk[bytes] = '\0';
            buffer += chunk;

            std::size_t pos;
            while ((pos = buffer.find('\n')) != std::string::npos) {
                std::string line = buffer.substr(0, pos);
                buffer.erase(0, pos + 1);

                if (!line.empty()) {
                    MessageCallback cb;
                    {
                        std::lock_guard<std::mutex> lock(mu_);
                        cb = msg_cb_;
                    }
                    if (cb) cb(peer_id, line);
                }
            }
        }
    }

    std::string node_id_;
    std::uint16_t listen_port_;
    socket_t listen_fd_ = static_cast<socket_t>(-1);
    std::atomic<bool> running_{false};
    std::thread accept_thread_;
    mutable std::mutex mu_;
    std::map<std::string, socket_t> peer_fds_;
    std::vector<std::thread> reader_threads_;
    MessageCallback msg_cb_;
};

} // namespace garuda::beam
