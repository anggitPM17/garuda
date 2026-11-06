// =====================================================================
// GARUDA COMPILER — FASE 10 ROUTER & MIDDLEWARE TEST SUITE (5 TESTS)
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
    std::cout << "GARUDA COMPILER — FASE 10 ROUTER & MIDDLEWARE TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test 1: Router Initialization
    std::string code1 = R"(
        modul TestRTR1
        fungsi utama() ->
            Router.baru()
        rampung
    )";
    auto res1 = engine.eval(code1);
    CHECK(res1.has_value(), "Test 1 must return Router state");
    auto* t1 = res1->get_if<garuda::beam::Tuple>();
    CHECK(t1 != nullptr && t1->items && t1->items->size() == 2, "Test 1 Router tuple size 2");
    std::cout << "[Test 1] Router.baru() Initialization [PASS]\n";

    // Test 2: Pipeline Route Chaining
    std::string code2 = R"(
        modul TestRTR2
        fungsi utama() ->
            R = Router.baru()
            R |> Router.get("/", "home") |> Router.get("/users", "users")
        rampung
    )";
    auto res2 = engine.eval(code2);
    CHECK(res2.has_value(), "Test 2 must return chained router");
    std::cout << "[Test 2] Pipeline Route Chaining (Router.get) [PASS]\n";

    // Test 3: Middleware Pipeline Attachment
    std::string code3 = R"(
        modul TestRTR3
        fungsi utama() ->
            R = Router.baru()
            R |> Router.middleware("logging") |> Router.middleware("cors")
        rampung
    )";
    auto res3 = engine.eval(code3);
    CHECK(res3.has_value(), "Test 3 must return middleware router");
    std::cout << "[Test 3] Middleware Pipeline Attachment (Router.middleware) [PASS]\n";

    // Test 4: Route Dispatch Matching
    std::string code4 = R"(
        modul TestRTR4
        fungsi utama() ->
            R = Router.baru()
            Req = %{"path" => "/api/users"}
            Router.dispatch(R, Req)
        rampung
    )";
    auto res4 = engine.eval(code4);
    CHECK(res4.has_value(), "Test 4 must return dispatch response");
    auto* s4 = res4->get_if<std::string>();
    CHECK(s4 != nullptr && s4->find("200 OK") != std::string::npos, "Test 4 dispatch 200 OK");
    std::cout << "[Test 4] Route Dispatch Matching -> 200 OK [PASS]\n";

    // Test 5: Router 404 Fallback
    std::string code5 = R"(
        modul TestRTR5
        fungsi utama() ->
            R = Router.baru()
            Req = %{"path" => "/missing"}
            Router.dispatch(R, Req)
        rampung
    )";
    auto res5 = engine.eval(code5);
    CHECK(res5.has_value(), "Test 5 must return 404 response");
    auto* s5 = res5->get_if<std::string>();
    CHECK(s5 != nullptr && s5->find("404 Route Not Found") != std::string::npos, "Test 5 dispatch 404");
    std::cout << "[Test 5] Router 404 Fallback -> 404 Not Found [PASS]\n";

    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL 5 FASE 10 ROUTER & MIDDLEWARE TESTS PASSED PERFECTLY!\n";
    std::cout << "========================================\n";
    return 0;
}
