#include "../include/garuda/beam/stdlib/http_server.hpp"
#include "../include/garuda/beam/stdlib/http.hpp"
#include <atomic>
#include <chrono>
#include <future>
#include <iostream>
#include <thread>
#include <vector>

using namespace garuda::beam::stdlib;

#define GARUDA_CHECK(cond, msg) \
    if (!(cond)) { \
        std::cerr << "  ❌ ASSERTION FAILED: " << msg << " (" << #cond << ")\n"; \
        std::exit(1); \
    }

void run_test_with_timeout(const std::string& test_name, std::function<void()> test_fn, int timeout_seconds = 10) {
    auto future = std::async(std::launch::async, test_fn);
    if (future.wait_for(std::chrono::seconds(timeout_seconds)) == std::future_status::timeout) {
        std::cerr << "  ❌ " << test_name << " FAILED (TIMEOUT > " << timeout_seconds << "s)\n";
        std::exit(1);
    }
}

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA HTTP SERVER END-TO-END SUITE\n";
    std::cout << "=======================================================\n\n";

    int port = 8099;
    Router router;

    router.get("/hello", [](const HttpRequest&) {
        return HttpServer::make_json("{\"message\":\"Hello Garuda E2E\"}");
    });

    HttpServer server;
    bool started = server.start(port, router, 32);
    GARUDA_CHECK(started && server.is_running(), "HttpServer must start on port 8099");

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // === Test 1: GET Request 200 OK & Body Verification ===
    run_test_with_timeout("Test 1: HTTP GET Request 200 OK", [&]() {
        auto resp = Http::get("http://127.0.0.1:8099/hello");
        GARUDA_CHECK(resp.has_value(), "HTTP request must receive a response");
        GARUDA_CHECK(resp->status_code == 200, "Response status must be 200 OK");
        GARUDA_CHECK(resp->body.find("Hello Garuda E2E") != std::string::npos, "Body must contain 'Hello Garuda E2E'");
        std::cout << "  ✓ Test 1: HTTP GET Request 200 OK PASS\n";
    }, 5);

    // === Test 2: Unmatched Route 404 Response ===
    run_test_with_timeout("Test 2: HTTP GET Unmatched Route 404", [&]() {
        auto resp = Http::get("http://127.0.0.1:8099/notfound");
        GARUDA_CHECK(resp.has_value(), "HTTP request must receive a response");
        GARUDA_CHECK(resp->status_code == 404, "Response status must be 404 Not Found");
        std::cout << "  ✓ Test 2: HTTP GET Unmatched Route 404 PASS\n";
    }, 5);

    // === Test 3: 100 Parallel Requests High-Concurrency Verification ===
    run_test_with_timeout("Test 3: 100 Parallel HTTP Requests", [&]() {
        std::vector<std::thread> threads;
        std::atomic<int> success_count{0};

        for (int t = 0; t < 10; ++t) {
            threads.emplace_back([&success_count]() {
                for (int i = 0; i < 10; ++i) {
                    auto res = Http::get("http://127.0.0.1:8099/hello");
                    if (res.has_value() && res->status_code == 200) {
                        success_count++;
                    }
                }
            });
        }

        for (auto& th : threads) {
            if (th.joinable()) th.join();
        }

        GARUDA_CHECK(success_count == 100, "All 100 parallel HTTP requests must succeed with 200 OK");
        std::cout << "  ✓ Test 3: 100 Parallel HTTP Requests PASS\n";
    }, 5);

    // === Test 4: Graceful Server Teardown ===
    run_test_with_timeout("Test 4: Graceful Server Teardown", [&]() {
        server.stop();
        GARUDA_CHECK(!server.is_running(), "Server must no longer be running after stop()");
        auto resp = Http::get("http://127.0.0.1:8099/hello", 200);
        GARUDA_CHECK(!resp.has_value(), "Request must fail after server is stopped");
        std::cout << "  ✓ Test 4: Graceful Server Teardown PASS\n";
    }, 5);

    std::cout << "\n🎉 ALL HTTP SERVER END-TO-END TESTS PASSED PERFECTLY!\n";
    return 0;
}
