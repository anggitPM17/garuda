// =====================================================================
// GARUDA COMPILER — FASE 10.6 REAL HTTPS TLS TEST SUITE (RED -> GREEN)
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

static std::string run_curl_command(const std::string& cmd) {
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
    std::cout << "GARUDA COMPILER — FASE 10.6 REAL HTTPS TLS TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Start HTTPS Server
    std::string code1 = R"(
        modul TestREALHTTPS
        fungsi utama() ->
            Server.mulai_https(8443, "cert.pem", "key.pem", nil, 10)
        rampung
    )";
    engine.eval(code1);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Test 1: curl -k -s https://127.0.0.1:8443/
    std::string res1 = run_curl_command("-k -s https://127.0.0.1:8443/");
    CHECK(res1.find("HTTPS OK") != std::string::npos, "Test 1 curl -k https://127.0.0.1:8443/ body match");
    std::cout << "[Test 1] Real TLS curl -k https://127.0.0.1:8443/ -> 'HTTPS OK' [PASS]\n";

    // Test 2: curl -k -s https://127.0.0.1:8443/health
    std::string res2 = run_curl_command("-k -s https://127.0.0.1:8443/health");
    CHECK(res2.find("{\"https\":\"ok\"}") != std::string::npos, "Test 2 curl -k https://127.0.0.1:8443/health JSON match");
    std::cout << "[Test 2] Real TLS curl -k https://127.0.0.1:8443/health -> JSON [PASS]\n";

    // Test 3: curl -k -s -o NUL -w "%{http_code}" https://127.0.0.1:8443/missing
    std::string res3 = run_curl_command("-k -s -o NUL -w \"%{http_code}\" https://127.0.0.1:8443/missing");
    CHECK(res3.find("404") != std::string::npos, "Test 3 curl -k https://127.0.0.1:8443/missing 404 code match");
    std::cout << "[Test 3] Real TLS curl -k https://127.0.0.1:8443/missing -> Status 404 [PASS]\n";

    // Test 4: curl https://127.0.0.1:8443/ (without -k -> MUST FAIL DUE TO SSL CERTIFICATE VERIFICATION)
    std::string res4 = run_curl_command("https://127.0.0.1:8443/");
    CHECK(res4.find("SSL") != std::string::npos || res4.find("certificate") != std::string::npos || res4.find("PKIX") != std::string::npos, "Test 4 curl without -k must fail certificate verify");
    std::cout << "[Test 4] Real TLS Certificate Verification Failure (curl without -k) [PASS]\n";

    // Test 5: curl -k -v https://127.0.0.1:8443/ 2>&1 (Verbose TLS Handshake Output)
    std::string res5 = run_curl_command("-k -v https://127.0.0.1:8443/");
    CHECK(res5.find("TLS") != std::string::npos || res5.find("SSL") != std::string::npos || res5.find("200 OK") != std::string::npos, "Test 5 curl -k -v verbose TLS handshake check");
    std::cout << "[Test 5] Real Verbose TLS Handshake Verification [PASS]\n";

    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL 5 FASE 10.6 REAL HTTPS TLS TESTS PASSED PERFECTLY!\n";
    std::cout << "========================================\n";
    return 0;
}
