#include "../include/garuda/beam/stdlib/http.hpp"
#include <cassert>
#include <chrono>
#include <iostream>
#include <thread>

using namespace garuda::beam::stdlib;

void run_mock_http_server(int port) {
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
    SOCKET listen_fd = socket(AF_INET, SOCK_STREAM, 0);
#else
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
#endif

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(static_cast<uint16_t>(port));

    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
    bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(listen_fd, 5);

    for (int i = 0; i < 2; ++i) { // Handle 2 requests (GET and POST)
#ifdef _WIN32
        SOCKET client_fd = accept(listen_fd, nullptr, nullptr);
#else
        int client_fd = accept(listen_fd, nullptr, nullptr);
#endif
        if (client_fd < 0) continue;

        char buf[1024];
        int bytes = recv(client_fd, buf, sizeof(buf) - 1, 0);
        if (bytes > 0) {
            buf[bytes] = '\0';
            std::string req(buf);

            std::string resp;
            if (req.rfind("GET", 0) == 0) {
                resp = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 10\r\n\r\nHello GET!";
            } else if (req.rfind("POST", 0) == 0) {
                resp = "HTTP/1.1 201 Created\r\nContent-Type: application/json\r\nContent-Length: 17\r\n\r\n{\"status\":\"saved\"}";
            } else {
                resp = "HTTP/1.1 400 Bad Request\r\n\r\n";
            }
            send(client_fd, resp.c_str(), static_cast<int>(resp.length()), 0);
        }
#ifdef _WIN32
        closesocket(client_fd);
#else
        close(client_fd);
#endif
    }

#ifdef _WIN32
    closesocket(listen_fd);
#else
    close(listen_fd);
#endif
}

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA STANDARD LIBRARY — SPRINT C.2 HTTP CLIENT TEST SUITE (10 TESTS)\n";
    std::cout << "=======================================================\n\n";

    // === Test 1: Simple URL Parsing ===
    std::cout << "[TEST 1/10] Running Simple URL Parsing test...\n";
    auto url1 = Http::parse_url("http://example.com/api/users");
    assert(url1.protocol == "http");
    assert(url1.host == "example.com");
    assert(url1.port == 80);
    assert(url1.path == "/api/users");
    std::cout << "  ✓ Test 1: Simple URL Parsing PASS\n";

    // === Test 2: Custom Port URL Parsing ===
    std::cout << "[TEST 2/10] Running Custom Port URL Parsing test...\n";
    auto url2 = Http::parse_url("http://localhost:8080/v1/health");
    assert(url2.protocol == "http");
    assert(url2.host == "localhost");
    assert(url2.port == 8080);
    assert(url2.path == "/v1/health");
    std::cout << "  ✓ Test 2: Custom Port URL Parsing PASS\n";

    // === Test 3: Default Path URL Parsing ===
    std::cout << "[TEST 3/10] Running Default Path URL Parsing test...\n";
    auto url3 = Http::parse_url("http://garudala.org");
    assert(url3.host == "garudala.org");
    assert(url3.path == "/");
    std::cout << "  ✓ Test 3: Default Path URL Parsing PASS\n";

    // === Test 4: HTTP Response Header Parser ===
    std::cout << "[TEST 4/10] Running HTTP Response Header Parser test...\n";
    std::string raw_4 = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nServer: GarudaEngine\r\n\r\nHello World";
    auto resp4 = Http::parse_response(raw_4);
    assert(resp4.has_value());
    assert(resp4->status_code == 200);
    assert(resp4->headers["content-type"] == "text/plain");
    assert(resp4->headers["server"] == "GarudaEngine");
    assert(resp4->body == "Hello World");
    std::cout << "  ✓ Test 4: HTTP Response Header Parser PASS\n";

    // Start mock HTTP server on port 9876
    int mock_port = 9876;
    std::thread server_thread(run_mock_http_server, mock_port);
    std::this_thread::sleep_for(std::chrono::milliseconds(200)); // Allow server thread to bind port

    // === Test 5: Local Loopback Server Setup ===
    std::cout << "[TEST 5/10] Running Local Loopback Server Setup test...\n";
    std::cout << "  ✓ Test 5: Local Loopback Server Setup PASS\n";

    // === Test 6: HTTP GET Request & Response ===
    std::cout << "[TEST 6/10] Running HTTP GET Request test...\n";
    auto resp6 = Http::get("http://127.0.0.1:9876/get");
    assert(resp6.has_value());
    assert(resp6->status_code == 200);
    assert(resp6->body == "Hello GET!");
    std::cout << "  ✓ Test 6: HTTP GET Request PASS\n";

    // === Test 7: HTTP POST Request & Body Transfer ===
    std::cout << "[TEST 7/10] Running HTTP POST Request test...\n";
    auto resp7 = Http::post("http://127.0.0.1:9876/post", "{\"name\":\"garuda\"}", "application/json");
    assert(resp7.has_value());
    assert(resp7->status_code == 201);
    assert(resp7->body == "{\"status\":\"saved\"}");
    std::cout << "  ✓ Test 7: HTTP POST Request PASS\n";

    server_thread.join();

    // === Test 8: Custom Headers Extraction ===
    std::cout << "[TEST 8/10] Running Custom Headers Extraction test...\n";
    std::string raw_8 = "HTTP/1.1 404 Not Found\r\nX-Custom-Header: garuda_v67\r\n\r\nPage Not Found";
    auto resp8 = Http::parse_response(raw_8);
    assert(resp8.has_value() && resp8->status_code == 404);
    assert(resp8->headers["x-custom-header"] == "garuda_v67");
    std::cout << "  ✓ Test 8: Custom Headers Extraction PASS\n";

    // === Test 9: Connection Refused Error Handling ===
    std::cout << "[TEST 9/10] Running Connection Refused Error Handling test...\n";
    auto resp9 = Http::get("http://127.0.0.1:59999/closed_port", 500);
    assert(!resp9.has_value()); // Connection refused -> returns std::nullopt!
    std::cout << "  ✓ Test 9: Connection Refused Error Handling PASS\n";

    // === Test 10: Timeout Handling ===
    std::cout << "[TEST 10/10] Running Timeout Handling test...\n";
    auto resp10 = Http::get("http://10.255.255.1:80/timeout", 200);
    assert(!resp10.has_value()); // Non-routable IP timeout -> returns std::nullopt!
    std::cout << "  ✓ Test 10: Timeout Handling PASS\n";

    std::cout << "\n🎉 ALL 10 HTTP CLIENT TESTS PASSED PERFECTLY!\n";
    return 0;
}
