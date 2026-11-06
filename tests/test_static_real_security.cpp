// =====================================================================
// GARUDA COMPILER — FASE 10.6 REAL STATIC TRAVERSAL SECURITY TEST SUITE (RED -> GREEN)
// =====================================================================

#include "../include/garuda/beam/tools/script_engine.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <cstdlib>

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        std::cerr << "[FAIL] " << msg << "\n"; \
        std::exit(1); \
    } \
} while(0)

static std::string run_curl_static(const std::string& cmd) {
    std::string full_cmd = "C:\\Windows\\System32\\curl.exe " + cmd + " 2>&1";
    #ifdef _WIN32
    FILE* pipe = _popen(full_cmd.c_str(), "r");
    #else
    FILE* pipe = popen(cmd.c_str(), "r");
    #endif

    if (!pipe) return "";
    char buffer[128];
    std::string result = "";
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    #ifdef _WIN32
    _pclose(pipe);
    #else
    pclose(pipe);
    #endif
    return result;
}

int main() {
    garuda::beam::ScriptEngine engine;

    std::cout << "=======================================================\n";
    std::cout << "GARUDA COMPILER — FASE 10.6 REAL STATIC TRAVERSAL SECURITY TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Start Static File Server on port 9096 with capacity 50
    std::string code1 = R"(
        modul TestSTCSec
        fungsi utama() ->
            Server.mulai_async_statis(9096, "site/", 50)
        rampung
    )";
    engine.eval(code1);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Test 1: URL-encoded traversal %2e%2e with --path-as-is must be rejected with 403 Forbidden
    std::string res1 = run_curl_static("--path-as-is -s -o NUL -w \"%{http_code}\" http://127.0.0.1:9096/%2e%2e/%2e%2e/etc/passwd");
    CHECK(res1.find("403") != std::string::npos, "Test 1 %2e%2e URL-encoded traversal must be 403");
    std::cout << "[Test 1] URL-encoded Traversal Rejection (%2e%2e -> 403) [PASS]\n";

    // Test 2: Double-encoded traversal %252e%252e must be rejected with 403 or 404
    std::string res2 = run_curl_static("--path-as-is -s -o NUL -w \"%{http_code}\" http://127.0.0.1:9096/%252e%252e/etc/passwd");
    CHECK(res2.find("403") != std::string::npos || res2.find("404") != std::string::npos, "Test 2 double-encoded traversal match");
    std::cout << "[Test 2] Double-encoded Traversal Rejection (%252e%252e -> 403/404) [PASS]\n";

    // Test 3: Path with ..%2f..%2fetc%2fpasswd must be rejected with 403
    std::string res3 = run_curl_static("--path-as-is -s -o NUL -w \"%{http_code}\" http://127.0.0.1:9096/..%2f..%2fetc%2fpasswd");
    CHECK(res3.find("403") != std::string::npos, "Test 3 ..%2f path traversal 403 match");
    std::cout << "[Test 3] Slanted Traversal Rejection (..%2f -> 403) [PASS]\n";

    // Test 4: Persistent Server - 5 Sequential Requests
    bool p_ok = true;
    for (int i = 0; i < 5; ++i) {
        std::string r = run_curl_static("-s -o NUL -w \"%{http_code}\" http://127.0.0.1:9096/index.html");
        if (r.find("200") == std::string::npos) {
            p_ok = false;
            break;
        }
    }
    CHECK(p_ok, "Test 4 persistent static server 5 sequential requests");
    std::cout << "[Test 4] Persistent Static Server (5 Sequential Requests 200 OK) [PASS]\n";

    // Test 5: MIME Types for .svg and .html
    std::string res5 = run_curl_static("-s -I http://127.0.0.1:9096/assets/favicon.svg");
    CHECK(res5.find("Content-Type: image/svg+xml") != std::string::npos, "Test 5 SVG MIME type match");
    std::cout << "[Test 5] Verified MIME Type (.svg -> image/svg+xml) [PASS]\n";

    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL 5 FASE 10.6 REAL STATIC TRAVERSAL SECURITY TESTS PASSED PERFECTLY!\n";
    std::cout << "========================================\n";
    return 0;
}
