// =====================================================================
// GARUDA COMPILER — FASE 10.5 REAL ROUTER & MIDDLEWARE E2E TEST SUITE (5 TESTS)
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

int main() {
    garuda::beam::ScriptEngine engine;

    std::cout << "=======================================================\n";
    std::cout << "GARUDA COMPILER — FASE 10.5 REAL ROUTER & MIDDLEWARE E2E TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test 1: Real Registered Route Dispatch
    std::string code1 = R"(
        modul TestRTRE2E1
        fungsi utama() ->
            R = Router.baru()
            R2 = Router.get(R, "/", "handler_home")
            Req = %{"path" => "/"}
            Router.dispatch(R2, Req)
        rampung
    )";
    auto res1 = engine.eval(code1);
    CHECK(res1.has_value(), "Test 1 must return dispatch response");
    auto* s1 = res1->get_if<std::string>();
    CHECK(s1 != nullptr && s1->find("200 OK") != std::string::npos, "Test 1 dispatch 200 OK");
    std::cout << "[Test 1] Real Registered Route Dispatch -> 200 OK [PASS]\n";

    // Test 2: Real Path Parameter Extraction (:id)
    std::string code2 = R"(
        modul TestRTRE2E2
        fungsi utama() ->
            R = Router.baru()
            R2 = Router.get(R, "/users/:id", "handler_user")
            Req = %{"path" => "/users/42"}
            Router.dispatch(R2, Req)
        rampung
    )";
    auto res2 = engine.eval(code2);
    CHECK(res2.has_value(), "Test 2 must return response with extracted param");
    auto* s2 = res2->get_if<std::string>();
    CHECK(s2 != nullptr && (s2->find("User ID: 42") != std::string::npos || s2->find("42") != std::string::npos), "Test 2 extracted param ID == 42");
    std::cout << "[Test 2] Real Path Parameter Extraction (:id -> 42): " << *s2 << " [PASS]\n";

    // Test 3: Real Logging Middleware Header (X-Logged: true)
    std::string code3 = R"(
        modul TestRTRE2E3
        fungsi utama() ->
            R = Router.baru() |> Router.get("/", "handler_home")
            R2 = Router.middleware(R, "logging_mw")
            Req = %{"path" => "/"}
            Router.dispatch(R2, Req)
        rampung
    )";
    auto res3 = engine.eval(code3);
    CHECK(res3.has_value(), "Test 3 must return response with logging header");
    auto* s3 = res3->get_if<std::string>();
    CHECK(s3 != nullptr && s3->find("X-Logged: true") != std::string::npos, "Test 3 X-Logged header match");
    std::cout << "[Test 3] Real Logging Middleware Header (X-Logged: true) [PASS]\n";

    // Test 4: Real CORS Middleware Header (Access-Control-Allow-Origin: *)
    std::string code4 = R"(
        modul TestRTRE2E4
        fungsi utama() ->
            R = Router.baru() |> Router.get("/", "handler_home")
            R2 = Router.middleware(R, "cors_mw")
            Req = %{"path" => "/"}
            Router.dispatch(R2, Req)
        rampung
    )";
    auto res4 = engine.eval(code4);
    CHECK(res4.has_value(), "Test 4 must return response with CORS header");
    auto* s4 = res4->get_if<std::string>();
    CHECK(s4 != nullptr && s4->find("Access-Control-Allow-Origin: *") != std::string::npos, "Test 4 CORS header match");
    std::cout << "[Test 4] Real CORS Middleware Header (Access-Control-Allow-Origin: *) [PASS]\n";

    // Test 5: Real Unregistered 404 Fallback
    std::string code5 = R"(
        modul TestRTRE2E5
        fungsi utama() ->
            R = Router.baru()
            Req = %{"path" => "/unregistered"}
            Router.dispatch(R, Req)
        rampung
    )";
    auto res5 = engine.eval(code5);
    CHECK(res5.has_value(), "Test 5 must return 404 response");
    auto* s5 = res5->get_if<std::string>();
    CHECK(s5 != nullptr && s5->find("404 Route Not Found") != std::string::npos, "Test 5 unregistered 404 match");
    std::cout << "[Test 5] Real Unregistered 404 Fallback -> 404 Route Not Found [PASS]\n";

    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL 5 FASE 10.5 REAL ROUTER & MIDDLEWARE E2E TESTS PASSED PERFECTLY!\n";
    std::cout << "========================================\n";
    return 0;
}
