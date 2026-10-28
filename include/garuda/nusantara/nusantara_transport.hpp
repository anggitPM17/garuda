#pragma once
#include <string>
#include <vector>
#include <optional>
#include <memory>
#include <iostream>
#include <cstdint>
#include <cstring>

#ifdef _WIN32
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #pragma comment(lib, "ws2_32.lib")
  using socket_t = SOCKET;
  constexpr socket_t INVALID_SOCK = INVALID_SOCKET;
#else
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <netdb.h>
  #include <unistd.h>
  #include <fcntl.h>
  using socket_t = int;
  constexpr socket_t INVALID_SOCK = -1;
#endif

namespace garuda::nusantara::transport {

inline void init_sockets() {
#ifdef _WIN32
    static struct WinsockInit {
        WinsockInit() {
            WSADATA wsaData;
            WSAStartup(MAKEWORD(2, 2), &wsaData);
        }
        ~WinsockInit() {
            WSACleanup();
        }
    } init;
#endif
}

inline void close_fd(socket_t fd) {
    if (fd != INVALID_SOCK) {
#ifdef _WIN32
        closesocket(fd);
#else
        ::close(fd);
#endif
    }
}

class Socket {
private:
    socket_t fd_ = INVALID_SOCK;
    std::string recv_buf_;

public:
    Socket() = default;
    explicit Socket(socket_t fd) : fd_(fd) { init_sockets(); }
    ~Socket() { close(); }

    Socket(Socket&& other) noexcept : fd_(other.fd_), recv_buf_(std::move(other.recv_buf_)) {
        other.fd_ = INVALID_SOCK;
    }

    Socket& operator=(Socket&& other) noexcept {
        if (this != &other) {
            close();
            fd_ = other.fd_;
            recv_buf_ = std::move(other.recv_buf_);
            other.fd_ = INVALID_SOCK;
        }
        return *this;
    }

    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    socket_t fd() const { return fd_; }
    bool valid() const { return fd_ != INVALID_SOCK; }

    void close() {
        if (fd_ != INVALID_SOCK) {
            close_fd(fd_);
            fd_ = INVALID_SOCK;
        }
        recv_buf_.clear();
    }

    bool send_all(const void* data, std::size_t len) {
        if (!valid()) return false;
        const char* ptr = static_cast<const char*>(data);
        std::size_t remaining = len;

        while (remaining > 0) {
#ifdef _WIN32
            int sent = ::send(fd_, ptr, static_cast<int>(remaining), 0);
#else
            ssize_t sent = ::send(fd_, ptr, remaining, 0);
#endif
            if (sent <= 0) {
                return false;
            }
            ptr += sent;
            remaining -= static_cast<std::size_t>(sent);
        }
        return true;
    }

    bool send_all(const std::string& s) {
        return send_all(s.data(), s.size());
    }

    std::optional<std::string> recv_until(char delim, std::size_t max_size = 16 * 1024 * 1024) {
        if (!valid()) return std::nullopt;

        while (true) {
            auto pos = recv_buf_.find(delim);
            if (pos != std::string::npos) {
                std::string result = recv_buf_.substr(0, pos);
                recv_buf_.erase(0, pos + 1);
                return result;
            }

            if (recv_buf_.size() >= max_size) {
                return std::nullopt;
            }

            char chunk[4096];
#ifdef _WIN32
            int n = ::recv(fd_, chunk, sizeof(chunk), 0);
#else
            ssize_t n = ::recv(fd_, chunk, sizeof(chunk), 0);
#endif
            if (n <= 0) {
                if (!recv_buf_.empty()) {
                    std::string result = std::move(recv_buf_);
                    recv_buf_.clear();
                    return result;
                }
                return std::nullopt;
            }
            recv_buf_.append(chunk, static_cast<std::size_t>(n));
        }
    }
};

class Listener {
private:
    socket_t fd_ = INVALID_SOCK;
    std::uint16_t port_ = 0;

public:
    Listener() = default;
    ~Listener() { close(); }

    Listener(Listener&& other) noexcept : fd_(other.fd_), port_(other.port_) {
        other.fd_ = INVALID_SOCK;
        other.port_ = 0;
    }

    Listener& operator=(Listener&& other) noexcept {
        if (this != &other) {
            close();
            fd_ = other.fd_;
            port_ = other.port_;
            other.fd_ = INVALID_SOCK;
            other.port_ = 0;
        }
        return *this;
    }

    Listener(const Listener&) = delete;
    Listener& operator=(const Listener&) = delete;

    bool listen(const std::string& host, std::uint16_t port) {
        init_sockets();
        close();

        fd_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (fd_ == INVALID_SOCK) return false;

        int opt = 1;
#ifdef _WIN32
        ::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt));
#else
        ::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

        struct sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);

        if (host == "0.0.0.0" || host.empty()) {
            addr.sin_addr.s_addr = INADDR_ANY;
        } else {
            inet_pton(AF_INET, host.c_str(), &addr.sin_addr);
        }

        if (::bind(fd_, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) != 0) {
            close();
            return false;
        }

        if (::listen(fd_, SOMAXCONN) != 0) {
            close();
            return false;
        }

        struct sockaddr_in bound_addr{};
        socklen_t len = sizeof(bound_addr);
        if (::getsockname(fd_, reinterpret_cast<struct sockaddr*>(&bound_addr), &len) == 0) {
            port_ = ntohs(bound_addr.sin_port);
        } else {
            port_ = port;
        }

        return true;
    }

    Socket accept() {
        if (!valid()) return Socket();
        struct sockaddr_in client_addr{};
        socklen_t len = sizeof(client_addr);
        socket_t client_fd = ::accept(fd_, reinterpret_cast<struct sockaddr*>(&client_addr), &len);
        if (client_fd == INVALID_SOCK) {
            return Socket();
        }
        return Socket(client_fd);
    }

    void close() {
        if (fd_ != INVALID_SOCK) {
            close_fd(fd_);
            fd_ = INVALID_SOCK;
        }
        port_ = 0;
    }

    bool valid() const { return fd_ != INVALID_SOCK; }
    socket_t fd() const { return fd_; }
    std::uint16_t port() const { return port_; }
};

inline std::optional<Socket> connect_to(const std::string& host, std::uint16_t port, int /*timeout_ms*/ = 5000) {
    init_sockets();

    struct addrinfo hints{}, *res = nullptr;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    std::string port_str = std::to_string(port);
    if (getaddrinfo(host.c_str(), port_str.c_str(), &hints, &res) != 0 || !res) {
        return std::nullopt;
    }

    socket_t fd = ::socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd == INVALID_SOCK) {
        freeaddrinfo(res);
        return std::nullopt;
    }

    if (::connect(fd, res->ai_addr, static_cast<int>(res->ai_addrlen)) != 0) {
        close_fd(fd);
        freeaddrinfo(res);
        return std::nullopt;
    }

    freeaddrinfo(res);
    return Socket(fd);
}

} // namespace garuda::nusantara::transport
