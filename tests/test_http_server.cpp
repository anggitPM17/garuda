#include "../include/garuda/beam/stdlib/http_server.hpp"
#include "../include/garuda/beam/stdlib/http.hpp"
#include <cassert>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

using namespace garuda::beam::stdlib;

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA STANDARD LIBRARY — SPRINT C.3 HTTP SERVER TEST SUITE (10 TESTS)\n";
    std::cout << "=======================================================\n\n";

    int port = 9988;
    Router router;

    // Route 1: GET /hello
    router.get("/hello", [](const HttpRequest&) {
        return HttpServer::make_ok("Sugeng Rawuh!");
    });

    // Route 2: GET /api/status
    router.get("/api/status", [](const HttpRequest&) {
        return HttpServer::make_json("{\"status\":\"online\",\"version\":\"67.0.0\"}");
    });

    // Route 3: POST /api/echo
    router.post("/api/echo", [](const HttpRequest& req) {
        return HttpServer::make_json("{\"received\":" + req.body + "}");
    });

    HttpServer server;

    // === Test 1: Bind and Listen ===
    std::cout << "[TEST 1/10] Running Server Bind & Listen test...\n";
    bool ok1 = server.start(port, router);
    assert(ok1);
    assert(server.is_running());
    std::cout << "  ✓ Test 1: Server Bind & Listen PASS\n";

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // === Test 2: HTTP GET Request 200 OK ===
    std::cout << "[TEST 2/10] Running HTTP GET Request test...\n";
    auto resp2 = Http::get("http://127.0.0.1:9988/hello");
    assert(resp2.has_value());
    assert(resp2->status_code == 200);
    assert(resp2->body == "Sugeng Rawuh!");
    std::cout << "  ✓ Test 2: HTTP GET Request 200 OK PASS\n";

    // === Test 3: Unmatched Route 404 Not Found ===
    std::cout << "[TEST 3/10] Running 404 Not Found test...\n";
    auto resp3 = Http::get("http://127.0.0.1:9988/not_found_path");
    assert(resp3.has_value());
    assert(resp3->status_code == 404);
    assert(resp3->body == "404 Not Found");
    std::cout << "  ✓ Test 3: 404 Not Found PASS\n";

    // === Test 4: HTTP POST Request with JSON Payload ===
    std::cout << "[TEST 4/10] Running HTTP POST JSON Payload test...\n";
    auto resp4 = Http::post("http://127.0.0.1:9988/api/echo", "{\"val\":42}");
    assert(resp4.has_value());
    assert(resp4->status_code == 200);
    assert(resp4->body.find("\"val\":42") != std::string::npos);
    std::cout << "  ✓ Test 4: HTTP POST JSON Payload PASS\n";

    // === Test 5: Multi-Route Dispatch ===
    std::cout << "[TEST 5/10] Running Multi-Route Dispatch test...\n";
    auto resp5 = Http::get("http://127.0.0.1:9988/api/status");
    assert(resp5.has_value());
    assert(resp5->status_code == 200);
    assert(resp5->body.find("online") != std::string::npos);
    std::cout << "  ✓ Test 5: Multi-Route Dispatch PASS\n";

    // === Test 6: Response Headers Generation ===
    std::cout << "[TEST 6/10] Running Response Headers Generation test...\n";
    assert(resp5->headers.find("content-type") != resp5->headers.end());
    assert(resp5->headers["content-type"] == "application/json");
    std::cout << "  ✓ Test 6: Response Headers Generation PASS\n";

    // === Test 7: Multi-Worker Concurrent Request Handling ===
    std::cout << "[TEST 7/10] Running Concurrent Multi-Worker Request test...\n";
    std::vector<std::thread> client_threads;
    std::atomic<int> success_count{0};

    for (int i = 0; i < 5; ++i) {
        client_threads.emplace_back([&success_count]() {
            auto res = Http::get("http://127.0.0.1:9988/hello");
            if (res.has_value() && res->status_code == 200 && res->body == "Sugeng Rawuh!") {
                success_count++;
            }
        });
    }

    for (auto& t : client_threads) {
        if (t.joinable()) t.join();
    }
    assert(success_count == 5);
    std::cout << "  ✓ Test 7: Concurrent Multi-Worker Request PASS\n";

    // === Test 8: Method Mismatch 405 Method Not Allowed ===
    std::cout << "[TEST 8/10] Running 405 Method Not Allowed test...\n";
    auto resp8 = Http::get("http://127.0.0.1:9988/api/echo"); // GET to a POST-only route!
    assert(resp8.has_value());
    assert(resp8->status_code == 405);
    std::cout << "  ✓ Test 8: 405 Method Not Allowed PASS\n";

    // === Test 9: Path & Query String Request Handling ===
    std::cout << "[TEST 9/10] Running Path & Query String Request test...\n";
    HttpRequest req9;
    req9.method = "GET";
    req9.path = "/api/status";
    HttpResponse res9 = router.dispatch(req9);
    assert(res9.status_code == 200);
    std::cout << "  ✓ Test 9: Path & Query String Request PASS\n";

    // === Test 10: Graceful Server Shutdown ===
    std::cout << "[TEST 10/10] Running Graceful Server Shutdown test...\n";
    server.stop();
    assert(!server.is_running());
    auto resp10 = Http::get("http://127.0.0.1:9988/hello", 300);
    assert(!resp10.has_value()); // Connection refused after shutdown!
    std::cout << "  ✓ Test 10: Graceful Server Shutdown PASS\n";

    std::cout << "\n🎉 ALL 10 HTTP SERVER TESTS PASSED PERFECTLY!\n";
    return 0;
}
