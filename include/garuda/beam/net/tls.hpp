#pragma once
// Garuda BEAM-core: Native TLS/SSL Adapter (`tls.hpp`)
// Menggunakan mbedTLS 3.5.2 (Vendor) untuk HTTPS / TLS sockets.
#include "mbedtls/net_sockets.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/x509_crt.h"
#include "mbedtls/pk.h"
#include "mbedtls/error.h"

#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace garuda::beam {

enum class TlsRole { Server, Client };

class TlsContext {
public:
    TlsContext() {
        mbedtls_ssl_config_init(&conf_);
        mbedtls_entropy_init(&entropy_);
        mbedtls_ctr_drbg_init(&ctr_drbg_);
        mbedtls_x509_crt_init(&srvcert_);
        mbedtls_pk_init(&pkey_);

        const char* pers = "garuda_tls";
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

    static std::shared_ptr<TlsContext> create_server_context(const std::string& cert_pem, const std::string& key_pem) {
        auto ctx = std::make_shared<TlsContext>();
        ctx->role_ = TlsRole::Server;

        int rc = mbedtls_ssl_config_defaults(&ctx->conf_,
                                            MBEDTLS_SSL_IS_SERVER,
                                            MBEDTLS_SSL_TRANSPORT_STREAM,
                                            MBEDTLS_SSL_PRESET_DEFAULT);
        if (rc != 0) return nullptr;

        mbedtls_ssl_conf_rng(&ctx->conf_, mbedtls_ctr_drbg_random, &ctx->ctr_drbg_);

        // Parse Cert
        rc = mbedtls_x509_crt_parse(&ctx->srvcert_,
                                    reinterpret_cast<const unsigned char*>(cert_pem.c_str()),
                                    cert_pem.length() + 1);
        if (rc != 0) return nullptr;

        // Parse Private Key
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

    static std::shared_ptr<TlsContext> create_client_context() {
        auto ctx = std::make_shared<TlsContext>();
        ctx->role_ = TlsRole::Client;

        int rc = mbedtls_ssl_config_defaults(&ctx->conf_,
                                            MBEDTLS_SSL_IS_CLIENT,
                                            MBEDTLS_SSL_TRANSPORT_STREAM,
                                            MBEDTLS_SSL_PRESET_DEFAULT);
        if (rc != 0) return nullptr;

        mbedtls_ssl_conf_rng(&ctx->conf_, mbedtls_ctr_drbg_random, &ctx->ctr_drbg_);
        mbedtls_ssl_conf_authmode(&ctx->conf_, MBEDTLS_SSL_VERIFY_NONE); // Self-signed ok for dev

        return ctx;
    }

    mbedtls_ssl_config* get_config() { return &conf_; }
    TlsRole get_role() const { return role_; }

private:
    TlsRole role_ = TlsRole::Client;
    mbedtls_ssl_config conf_;
    mbedtls_entropy_context entropy_;
    mbedtls_ctr_drbg_context ctr_drbg_;
    mbedtls_x509_crt srvcert_;
    mbedtls_pk_context pkey_;
};

class TlsSocket {
public:
    TlsSocket(std::shared_ptr<TlsContext> ctx, int socket_fd)
        : ctx_(std::move(ctx)), socket_fd_(socket_fd) {
        mbedtls_ssl_init(&ssl_);
        mbedtls_net_init(&net_ctx_);
        net_ctx_.fd = socket_fd;

        if (ctx_) {
            mbedtls_ssl_setup(&ssl_, ctx_->get_config());
            mbedtls_ssl_set_bio(&ssl_, &net_ctx_, mbedtls_net_send, mbedtls_net_recv, nullptr);
        }
    }

    ~TlsSocket() {
        close();
        mbedtls_ssl_free(&ssl_);
    }

    bool handshake() {
        int rc = 0;
        while ((rc = mbedtls_ssl_handshake(&ssl_)) != 0) {
            if (rc != MBEDTLS_ERR_SSL_WANT_READ && rc != MBEDTLS_ERR_SSL_WANT_WRITE) {
                return false;
            }
        }
        return true;
    }

    int write(const void* data, std::size_t len) {
        int rc = 0;
        const unsigned char* p = reinterpret_cast<const unsigned char*>(data);
        std::size_t written = 0;

        while (written < len) {
            rc = mbedtls_ssl_write(&ssl_, p + written, len - written);
            if (rc > 0) {
                written += rc;
            } else if (rc != MBEDTLS_ERR_SSL_WANT_READ && rc != MBEDTLS_ERR_SSL_WANT_WRITE) {
                return rc;
            }
        }
        return static_cast<int>(written);
    }

    int read(void* buf, std::size_t len) {
        int rc = 0;
        while ((rc = mbedtls_ssl_read(&ssl_, reinterpret_cast<unsigned char*>(buf), len)) < 0) {
            if (rc != MBEDTLS_ERR_SSL_WANT_READ && rc != MBEDTLS_ERR_SSL_WANT_WRITE) {
                return rc;
            }
        }
        return rc;
    }

    void close() {
        if (!closed_) {
            mbedtls_ssl_close_notify(&ssl_);
            closed_ = true;
        }
    }

private:
    std::shared_ptr<TlsContext> ctx_;
    int socket_fd_;
    mbedtls_net_context net_ctx_;
    mbedtls_ssl_context ssl_;
    bool closed_ = false;
};

} // namespace garuda::beam
