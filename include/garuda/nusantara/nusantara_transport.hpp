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

#include "mbedtls/net_sockets.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/x509_crt.h"
#include "mbedtls/pk.h"
#include "mbedtls/error.h"

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

// === FASE D1: TLS CREDENTIALS & SOCKET LAYER ===

struct TlsCredentials {
    std::string cert_pem;
    std::string key_pem;
    bool valid = false;

    bool generate_self_signed(const std::string& common_name = "garuda-nusantara") {
        mbedtls_pk_context key;
        mbedtls_x509write_cert write_cert;
        mbedtls_entropy_context entropy;
        mbedtls_ctr_drbg_context ctr_drbg;

        mbedtls_pk_init(&key);
        mbedtls_x509write_crt_init(&write_cert);
        mbedtls_entropy_init(&entropy);
        mbedtls_ctr_drbg_init(&ctr_drbg);

        const char* pers = "garuda_gen_cert";
        int rc = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                                       reinterpret_cast<const unsigned char*>(pers), strlen(pers));
        if (rc != 0) {
            mbedtls_x509write_crt_free(&write_cert);
            mbedtls_pk_free(&key);
            mbedtls_ctr_drbg_free(&ctr_drbg);
            mbedtls_entropy_free(&entropy);
            return false;
        }

        rc = mbedtls_pk_setup(&key, mbedtls_pk_info_from_type(MBEDTLS_PK_RSA));
        if (rc != 0) {
            mbedtls_x509write_crt_free(&write_cert);
            mbedtls_pk_free(&key);
            mbedtls_ctr_drbg_free(&ctr_drbg);
            mbedtls_entropy_free(&entropy);
            return false;
        }

        rc = mbedtls_rsa_gen_key(mbedtls_pk_rsa(key), mbedtls_ctr_drbg_random, &ctr_drbg, 2048, 65537);
        if (rc != 0) {
            mbedtls_x509write_crt_free(&write_cert);
            mbedtls_pk_free(&key);
            mbedtls_ctr_drbg_free(&ctr_drbg);
            mbedtls_entropy_free(&entropy);
            return false;
        }

        std::string subject = "CN=" + common_name + ",O=Garuda";
        mbedtls_x509write_crt_set_subject_key(&write_cert, &key);
        mbedtls_x509write_crt_set_issuer_key(&write_cert, &key);
        mbedtls_x509write_crt_set_md_alg(&write_cert, MBEDTLS_MD_SHA256);
        mbedtls_x509write_crt_set_subject_name(&write_cert, subject.c_str());
        mbedtls_x509write_crt_set_issuer_name(&write_cert, subject.c_str());
        mbedtls_x509write_crt_set_validity(&write_cert, "20260101000000", "20360101000000");

        mbedtls_mpi serial;
        mbedtls_mpi_init(&serial);
        mbedtls_mpi_read_string(&serial, 10, "1");
        mbedtls_x509write_crt_set_serial(&write_cert, &serial);
        mbedtls_mpi_free(&serial);

        unsigned char cert_buf[4096];
        memset(cert_buf, 0, sizeof(cert_buf));
        rc = mbedtls_x509write_crt_pem(&write_cert, cert_buf, sizeof(cert_buf), mbedtls_ctr_drbg_random, &ctr_drbg);
        if (rc != 0) {
            mbedtls_x509write_crt_free(&write_cert);
            mbedtls_pk_free(&key);
            mbedtls_ctr_drbg_free(&ctr_drbg);
            mbedtls_entropy_free(&entropy);
            return false;
        }
        cert_pem = reinterpret_cast<char*>(cert_buf);

        unsigned char key_buf[4096];
        memset(key_buf, 0, sizeof(key_buf));
        rc = mbedtls_pk_write_key_pem(&key, key_buf, sizeof(key_buf));
        if (rc != 0) {
            mbedtls_x509write_crt_free(&write_cert);
            mbedtls_pk_free(&key);
            mbedtls_ctr_drbg_free(&ctr_drbg);
            mbedtls_entropy_free(&entropy);
            return false;
        }
        key_pem = reinterpret_cast<char*>(key_buf);

        mbedtls_x509write_crt_free(&write_cert);
        mbedtls_pk_free(&key);
        mbedtls_ctr_drbg_free(&ctr_drbg);
        mbedtls_entropy_free(&entropy);

        valid = true;
        return true;
    }
};

class TlsContext {
private:
    mbedtls_ssl_config conf_;
    mbedtls_entropy_context entropy_;
    mbedtls_ctr_drbg_context ctr_drbg_;
    mbedtls_x509_crt srvcert_;
    mbedtls_pk_context pkey_;

public:
    TlsContext() {
        mbedtls_ssl_config_init(&conf_);
        mbedtls_entropy_init(&entropy_);
        mbedtls_ctr_drbg_init(&ctr_drbg_);
        mbedtls_x509_crt_init(&srvcert_);
        mbedtls_pk_init(&pkey_);

        const char* pers = "garuda_nusantara_tls";
        mbedtls_ctr_drbg_seed(&ctr_drbg_, mbedtls_entropy_func, &entropy_,
                              reinterpret_cast<const unsigned char*>(pers), strlen(pers));
    }

    ~TlsContext() {
        mbedtls_x509_crt_free(&srvcert_);
        mbedtls_pk_free(&pkey_);
        mbedtls_ssl_config_free(&conf_);
        mbedtls_ctr_drbg_free(&ctr_drbg_);
        mbedtls_entropy_free(&entropy_);
    }

    TlsContext(const TlsContext&) = delete;
    TlsContext& operator=(const TlsContext&) = delete;

    static std::shared_ptr<TlsContext> create_server(const std::string& cert_pem, const std::string& key_pem) {
        auto ctx = std::make_shared<TlsContext>();
        int rc = mbedtls_ssl_config_defaults(&ctx->conf_,
                                             MBEDTLS_SSL_IS_SERVER,
                                             MBEDTLS_SSL_TRANSPORT_STREAM,
                                             MBEDTLS_SSL_PRESET_DEFAULT);
        if (rc != 0) return nullptr;

        mbedtls_ssl_conf_rng(&ctx->conf_, mbedtls_ctr_drbg_random, &ctx->ctr_drbg_);

        rc = mbedtls_x509_crt_parse(&ctx->srvcert_,
                                    reinterpret_cast<const unsigned char*>(cert_pem.c_str()),
                                    cert_pem.length() + 1);
        if (rc != 0) return nullptr;

        rc = mbedtls_pk_parse_key(&ctx->pkey_,
                                  reinterpret_cast<const unsigned char*>(key_pem.c_str()),
                                  key_pem.length() + 1,
                                  nullptr, 0,
                                  mbedtls_ctr_drbg_random, &ctx->ctr_drbg_);
        if (rc != 0) return nullptr;

        rc = mbedtls_ssl_conf_own_cert(&ctx->conf_, &ctx->srvcert_, &ctx->pkey_);
        if (rc != 0) return nullptr;

        return ctx;
    }

    static std::shared_ptr<TlsContext> create_client(const std::string& ca_cert_path = "") {
        auto ctx = std::make_shared<TlsContext>();
        int rc = mbedtls_ssl_config_defaults(&ctx->conf_,
                                             MBEDTLS_SSL_IS_CLIENT,
                                             MBEDTLS_SSL_TRANSPORT_STREAM,
                                             MBEDTLS_SSL_PRESET_DEFAULT);
        if (rc != 0) return nullptr;

        mbedtls_ssl_conf_rng(&ctx->conf_, mbedtls_ctr_drbg_random, &ctx->ctr_drbg_);

        if (!ca_cert_path.empty()) {
            rc = mbedtls_x509_crt_parse_file(&ctx->srvcert_, ca_cert_path.c_str());
            if (rc == 0) {
                mbedtls_ssl_conf_ca_chain(&ctx->conf_, &ctx->srvcert_, nullptr);
                mbedtls_ssl_conf_authmode(&ctx->conf_, MBEDTLS_SSL_VERIFY_REQUIRED);
            } else {
                mbedtls_ssl_conf_authmode(&ctx->conf_, MBEDTLS_SSL_VERIFY_NONE);
            }
        } else {
            mbedtls_ssl_conf_authmode(&ctx->conf_, MBEDTLS_SSL_VERIFY_NONE);
        }

        return ctx;
    }

    mbedtls_ssl_config* get_config() { return &conf_; }
};

class TlsSocket {
private:
    struct Impl {
        std::shared_ptr<TlsContext> ctx;
        Socket raw_sock;
        mbedtls_net_context net_ctx;
        mbedtls_ssl_context ssl;
        bool ssl_initialized = false;
        bool closed = false;
        std::string recv_buf;

        Impl(std::shared_ptr<TlsContext> c, Socket r) : ctx(std::move(c)), raw_sock(std::move(r)) {
            if (ctx && raw_sock.valid()) {
                mbedtls_ssl_init(&ssl);
                mbedtls_net_init(&net_ctx);
                net_ctx.fd = static_cast<int>(raw_sock.fd());

                if (mbedtls_ssl_setup(&ssl, ctx->get_config()) == 0) {
                    mbedtls_ssl_set_bio(&ssl, &net_ctx, mbedtls_net_send, mbedtls_net_recv, nullptr);
                    ssl_initialized = true;
                }
            }
        }

        ~Impl() {
            close();
            if (ssl_initialized) {
                mbedtls_ssl_free(&ssl);
            }
        }

        void close() {
            if (!closed) {
                if (ssl_initialized) {
                    mbedtls_ssl_close_notify(&ssl);
                }
                raw_sock.close();
                closed = true;
            }
            recv_buf.clear();
        }
    };

    std::shared_ptr<Impl> impl_;

public:
    TlsSocket() = default;

    TlsSocket(std::shared_ptr<TlsContext> ctx, Socket raw_sock)
        : impl_(std::make_shared<Impl>(std::move(ctx), std::move(raw_sock))) {}

    bool valid() const { return impl_ && impl_->ssl_initialized && impl_->raw_sock.valid(); }

    bool handshake() {
        if (!valid()) return false;
        int rc = 0;
        while ((rc = mbedtls_ssl_handshake(&impl_->ssl)) != 0) {
            if (rc != MBEDTLS_ERR_SSL_WANT_READ && rc != MBEDTLS_ERR_SSL_WANT_WRITE) {
                return false;
            }
        }
        return true;
    }

    bool send_all(const void* data, std::size_t len) {
        if (!valid()) return false;
        const unsigned char* p = static_cast<const unsigned char*>(data);
        std::size_t written = 0;

        while (written < len) {
            int rc = mbedtls_ssl_write(&impl_->ssl, p + written, len - written);
            if (rc > 0) {
                written += static_cast<std::size_t>(rc);
            } else if (rc != MBEDTLS_ERR_SSL_WANT_READ && rc != MBEDTLS_ERR_SSL_WANT_WRITE) {
                return false;
            }
        }
        return true;
    }

    bool send_all(const std::string& s) {
        return send_all(s.data(), s.size());
    }

    std::optional<std::string> recv_until(char delim, std::size_t max_size = 16 * 1024 * 1024) {
        if (!valid()) return std::nullopt;

        while (true) {
            auto pos = impl_->recv_buf.find(delim);
            if (pos != std::string::npos) {
                std::string result = impl_->recv_buf.substr(0, pos);
                impl_->recv_buf.erase(0, pos + 1);
                return result;
            }

            if (impl_->recv_buf.size() >= max_size) {
                return std::nullopt;
            }

            unsigned char chunk[4096];
            int rc = mbedtls_ssl_read(&impl_->ssl, chunk, sizeof(chunk));
            if (rc > 0) {
                impl_->recv_buf.append(reinterpret_cast<char*>(chunk), static_cast<std::size_t>(rc));
            } else if (rc == MBEDTLS_ERR_SSL_WANT_READ || rc == MBEDTLS_ERR_SSL_WANT_WRITE) {
                continue;
            } else {
                if (!impl_->recv_buf.empty()) {
                    std::string result = std::move(impl_->recv_buf);
                    impl_->recv_buf.clear();
                    return result;
                }
                return std::nullopt;
            }
        }
    }

    void close() {
        if (impl_) impl_->close();
    }
};

class TlsListener {
private:
    std::shared_ptr<TlsContext> ctx_;
    Listener raw_listener_;

public:
    TlsListener() = default;
    ~TlsListener() { close(); }

    TlsListener(TlsListener&& other) noexcept
        : ctx_(std::move(other.ctx_)), raw_listener_(std::move(other.raw_listener_)) {}

    TlsListener& operator=(TlsListener&& other) noexcept {
        if (this != &other) {
            close();
            ctx_ = std::move(other.ctx_);
            raw_listener_ = std::move(other.raw_listener_);
        }
        return *this;
    }

    TlsListener(const TlsListener&) = delete;
    TlsListener& operator=(const TlsListener&) = delete;

    bool listen(const std::string& host, std::uint16_t port, const std::string& cert_pem, const std::string& key_pem) {
        ctx_ = TlsContext::create_server(cert_pem, key_pem);
        if (!ctx_) return false;
        return raw_listener_.listen(host, port);
    }

    TlsSocket accept() {
        if (!valid()) return TlsSocket();
        auto raw_conn = raw_listener_.accept();
        if (!raw_conn.valid()) return TlsSocket();

        TlsSocket tls_sock(ctx_, std::move(raw_conn));
        if (!tls_sock.handshake()) {
            tls_sock.close();
            return TlsSocket();
        }
        return tls_sock;
    }

    void close() {
        raw_listener_.close();
        ctx_.reset();
    }

    bool valid() const { return raw_listener_.valid() && ctx_ != nullptr; }
    std::uint16_t port() const { return raw_listener_.port(); }
};

inline std::optional<TlsSocket> connect_tls_to(const std::string& host, std::uint16_t port, const std::string& ca_cert_path = "", int timeout_ms = 5000) {
    auto raw_sock = connect_to(host, port, timeout_ms);
    if (!raw_sock.has_value()) return std::nullopt;

    auto ctx = TlsContext::create_client(ca_cert_path);
    if (!ctx) return std::nullopt;

    TlsSocket tls_sock(ctx, std::move(*raw_sock));
    if (!tls_sock.handshake()) {
        return std::nullopt;
    }
    return tls_sock;
}

} // namespace garuda::nusantara::transport
