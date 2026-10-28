#include "../include/garuda/beam/io/health_check.hpp"
#include "../include/garuda/beam/io/http_ws_server.hpp"
#include "../include/garuda/beam/core/runtime.hpp"
#if !defined(_WIN32)
#include <arpa/inet.h>
#endif
#include <chrono>
#include <future>
#include <iostream>
#include <thread>

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

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA HEALTH CHECK & GRACEFUL SHUTDOWN SUITE\n";
    std::cout << "=======================================================\n\n";

    // === Test 1: HealthChecker Status Transition (OK -> Error) ===
    run_test_with_timeout("Test 1: HealthChecker Status Transition", []() {
        auto& hc = HealthChecker::instance();
        hc.clear_checks();
        hc.set_version("67.0.0");

        bool db_healthy = true;
        hc.register_check("memory", []() { return true; });
        hc.register_check("database", [&db_healthy]() { return db_healthy; });

        // 1. Healthy State
        std::string json_ok = hc.get_health_json();
        GARUDA_CHECK(json_ok.find("\"status\":\"ok\"") != std::string::npos, "JSON status must be 'ok'");
        GARUDA_CHECK(json_ok.find("\"database\":\"ok\"") != std::string::npos, "Database check must be 'ok'");

        // 2. Unhealthy Transition
        db_healthy = false;
        std::string json_err = hc.get_health_json();
        GARUDA_CHECK(json_err.find("\"status\":\"error\"") != std::string::npos, "JSON status must be 'error'");
        GARUDA_CHECK(json_err.find("\"database\":\"failed\"") != std::string::npos, "Database check must be 'failed'");

        std::cout << "  ✓ Test 1: HealthChecker Status Transition PASS\n";
    }, 5);

    // === Test 2: HTTP GET /health Endpoint Roundtrip & Graceful Teardown ===
    run_test_with_timeout("Test 2: HTTP GET /health Roundtrip & Graceful Teardown", []() {
        Runtime rt;
        EventLoop loop(rt);
        int port = 8105;

        auto& hc = HealthChecker::instance();
        hc.clear_checks();
        hc.register_check("system", []() { return true; });

        HttpWsServer server(loop, port);
        server.set_http_handler([&hc](const HttpRequest& req) {
            HttpResponse resp;
            if (req.path == "/health") {
                resp.status_code = 200;
                resp.body = hc.get_health_json();
                resp.headers["Content-Type"] = "application/json";
            } else {
                resp.status_code = 404;
                resp.body = "Not Found";
            }
            return resp;
        });

        bool started = server.start();
        GARUDA_CHECK(started, "HttpWsServer must start on port 8105");

        std::thread loop_thread([&loop]() {
            loop.run();
        });

        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        // Connect Client Socket
        socket_t client_fd = ::socket(AF_INET, SOCK_STREAM, 0);
        GARUDA_CHECK(client_fd != INVALID_SOCK, "Client socket creation must succeed");

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(static_cast<u_short>(port));
        inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

        int conn_rc = ::connect(client_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
        GARUDA_CHECK(conn_rc == 0, "Socket connect to 127.0.0.1:8105 must succeed");

        std::string req = "GET /health HTTP/1.1\r\nHost: 127.0.0.1:8105\r\nConnection: close\r\n\r\n";
        ::send(client_fd, req.c_str(), static_cast<int>(req.length()), 0);

        char buf[1024];
        int bytes = ::recv(client_fd, buf, sizeof(buf) - 1, 0);
        GARUDA_CHECK(bytes > 0, "Must receive HTTP response from /health");
        buf[bytes] = '\0';

        std::string resp_str(buf, bytes);
        GARUDA_CHECK(resp_str.find("200 OK") != std::string::npos, "Health endpoint must return HTTP 200 OK");
        GARUDA_CHECK(resp_str.find("\"status\":\"ok\"") != std::string::npos, "Health endpoint response body must contain '\"status\":\"ok\"'");

        CLOSE_SOCK(client_fd);

        // Graceful Teardown
        server.stop();
        loop.stop();

        if (loop_thread.joinable()) loop_thread.join();

        std::cout << "  ✓ Test 2: HTTP GET /health Roundtrip & Graceful Teardown PASS\n";
    }, 5);

    std::cout << "\n🎉 ALL HEALTH CHECK & GRACEFUL SHUTDOWN TESTS PASSED PERFECTLY!\n";
    return 0;
}
