#include "../include/garuda/beam/io/http_ws_server.hpp"
#include "../include/garuda/beam/net/tls.hpp"
#include "mbedtls/x509_crt.h"
#include "mbedtls/pk.h"
#include <chrono>
#include <future>
#include <iostream>
#include <thread>
#include <vector>

using namespace garuda::beam;

#define GARUDA_CHECK(cond, msg) \
    if (!(cond)) { \
        std::cerr << "  ❌ ASSERTION FAILED: " << msg << " (" << #cond << ")\n"; \
        std::exit(1); \
    }

void run_test_with_timeout(const std::string& test_name, std::function<void()> test_fn, int timeout_seconds = 10) {
    auto future = std::async(std::launch::async, test_fn);
    if (future.wait_for(std::chrono::seconds(timeout_seconds)) == std::future_status::timeout) {
        std::cerr << "  ❌ " << test_name << " FAILED (TIMEOUT > " << timeout_seconds << "s)\n";
        std::exit(1); \
    }
}

// Generate valid self-signed certificate & private key pair using mbedTLS
bool generate_self_signed_cert(std::string& out_cert_pem, std::string& out_key_pem) {
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
    if (rc != 0) return false;

    rc = mbedtls_pk_setup(&key, mbedtls_pk_info_from_type(MBEDTLS_PK_RSA));
    if (rc != 0) return false;

    rc = mbedtls_rsa_gen_key(mbedtls_pk_rsa(key), mbedtls_ctr_drbg_random, &ctr_drbg, 2048, 65537);
    if (rc != 0) return false;

    mbedtls_x509write_crt_set_subject_key(&write_cert, &key);
    mbedtls_x509write_crt_set_issuer_key(&write_cert, &key);
    mbedtls_x509write_crt_set_md_alg(&write_cert, MBEDTLS_MD_SHA256);
    mbedtls_x509write_crt_set_subject_name(&write_cert, "CN=localhost,O=Garuda");
    mbedtls_x509write_crt_set_issuer_name(&write_cert, "CN=localhost,O=Garuda");
    mbedtls_x509write_crt_set_validity(&write_cert, "20260101000000", "20360101000000");

    mbedtls_mpi serial;
    mbedtls_mpi_init(&serial);
    mbedtls_mpi_read_string(&serial, 10, "1");
    mbedtls_x509write_crt_set_serial(&write_cert, &serial);
    mbedtls_mpi_free(&serial);

    unsigned char cert_buf[4096];
    memset(cert_buf, 0, sizeof(cert_buf));
    rc = mbedtls_x509write_crt_pem(&write_cert, cert_buf, sizeof(cert_buf), mbedtls_ctr_drbg_random, &ctr_drbg);
    if (rc != 0) return false;
    out_cert_pem = reinterpret_cast<char*>(cert_buf);

    unsigned char key_buf[4096];
    memset(key_buf, 0, sizeof(key_buf));
    rc = mbedtls_pk_write_key_pem(&key, key_buf, sizeof(key_buf));
    if (rc != 0) return false;
    out_key_pem = reinterpret_cast<char*>(key_buf);

    mbedtls_x509write_crt_free(&write_cert);
    mbedtls_pk_free(&key);
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);
    return true;
}

int main() {
#if defined(_WIN32)
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    std::cout << "=======================================================\n";
    std::cout << "GARUDA NATIVE MBEDTLS TLS/SSL SUITE\n";
    std::cout << "=======================================================\n\n";

    std::string cert_pem;
    std::string key_pem;
    bool cert_gen_ok = generate_self_signed_cert(cert_pem, key_pem);
    GARUDA_CHECK(cert_gen_ok, "Self-signed certificate generation must succeed");

    // === Test 1: Self-Signed Certificate Generation ===
    run_test_with_timeout("Test 1: Self-Signed Cert & Key Generation", [cert_pem, key_pem]() {
        GARUDA_CHECK(cert_pem.find("BEGIN CERTIFICATE") != std::string::npos, "Cert PEM must contain BEGIN CERTIFICATE");
        GARUDA_CHECK(key_pem.find("BEGIN RSA PRIVATE KEY") != std::string::npos || key_pem.find("BEGIN PRIVATE KEY") != std::string::npos, "Key PEM must contain PRIVATE KEY");
        std::cout << "  ✓ Test 1: Self-Signed Cert & Key Generation PASS\n";
    }, 5);

    // === Test 2: Server & Client TLS Context Creation ===
    run_test_with_timeout("Test 2: TLS Server & Client Context Initialization", [cert_pem, key_pem]() {
        auto srv_ctx = TlsContext::create_server_context(cert_pem, key_pem);
        GARUDA_CHECK(srv_ctx != nullptr, "TlsContext::create_server_context must succeed");

        auto cli_ctx = TlsContext::create_client_context();
        GARUDA_CHECK(cli_ctx != nullptr, "TlsContext::create_client_context must succeed");

        std::cout << "  ✓ Test 2: TLS Server & Client Context Initialization PASS\n";
    }, 5);

    // === Test 3: Real TLS Socket Handshake & Data Roundtrip ===
    run_test_with_timeout("Test 3: TLS Socket Handshake & Data Roundtrip over TCP", [cert_pem, key_pem]() {
        auto srv_ctx = TlsContext::create_server_context(cert_pem, key_pem);
        auto cli_ctx = TlsContext::create_client_context();

        // 1. Create Server Listening Socket
        socket_t listener_fd = ::socket(AF_INET, SOCK_STREAM, 0);
        GARUDA_CHECK(listener_fd != INVALID_SOCK, "Listener socket creation must succeed");

        sockaddr_in server_addr{};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = 0; // Bind to ephemeral OS port
        inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

        int bind_rc = ::bind(listener_fd, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
        GARUDA_CHECK(bind_rc == 0, "Socket bind must succeed");

        socklen_t addr_len = sizeof(server_addr);
        ::getsockname(listener_fd, reinterpret_cast<sockaddr*>(&server_addr), &addr_len);
        int port = ntohs(server_addr.sin_port);

        ::listen(listener_fd, 1);

        // 2. Client Thread
        std::thread client_thread([cli_ctx, port]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            socket_t cli_fd = ::socket(AF_INET, SOCK_STREAM, 0);
            sockaddr_in client_addr{};
            client_addr.sin_family = AF_INET;
            client_addr.sin_port = htons(static_cast<u_short>(port));
            inet_pton(AF_INET, "127.0.0.1", &client_addr.sin_addr);

            int conn_rc = ::connect(cli_fd, reinterpret_cast<sockaddr*>(&client_addr), sizeof(client_addr));
            if (conn_rc != 0) return;

            TlsSocket cli_sock(cli_ctx, static_cast<int>(cli_fd));
            if (!cli_sock.handshake()) return;

            // Send HTTPS GET Request over TLS
            std::string req = "GET /status HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";
            cli_sock.write(req.data(), req.length());

            char buf[1024];
            int r = cli_sock.read(buf, sizeof(buf) - 1);
            if (r > 0) {
                buf[r] = '\0';
                GARUDA_CHECK(std::string(buf).find("200 OK") != std::string::npos, "Client must receive 200 OK over TLS");
            }
            cli_sock.close();
            CLOSE_SOCK(cli_fd);
        });

        // 3. Server Accept Thread
        socket_t srv_conn = ::accept(listener_fd, nullptr, nullptr);
        GARUDA_CHECK(srv_conn != INVALID_SOCK, "Server accept must succeed");

        TlsSocket srv_sock(srv_ctx, static_cast<int>(srv_conn));
        bool hs_ok = srv_sock.handshake();
        GARUDA_CHECK(hs_ok, "Server TLS handshake must succeed");

        char req_buf[1024];
        int bytes = srv_sock.read(req_buf, sizeof(req_buf) - 1);
        GARUDA_CHECK(bytes > 0, "Server must read HTTP request over TLS");
        req_buf[bytes] = '\0';
        GARUDA_CHECK(std::string(req_buf).find("GET /status") != std::string::npos, "Request must contain GET /status");

        std::string http_resp = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 15\r\n\r\nGaruda TLS PASS";
        srv_sock.write(http_resp.data(), http_resp.length());

        if (client_thread.joinable()) client_thread.join();

        srv_sock.close();
        CLOSE_SOCK(srv_conn);
        CLOSE_SOCK(listener_fd);

        std::cout << "  ✓ Test 3: TLS Socket Handshake & Data Roundtrip PASS\n";
    }, 10);

    std::cout << "\n🎉 ALL NATIVE MBEDTLS TLS/SSL TESTS PASSED PERFECTLY!\n";
    return 0;
}
