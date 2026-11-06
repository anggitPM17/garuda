// =====================================================================
// GARUDA COMPILER — FASE 10.6 REAL ROUTER REGISTRY TEST SUITE (RED -> GREEN)
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
    std::cout << "GARUDA COMPILER — FASE 10.6 REAL ROUTER REGISTRY TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test 1: Register unhardcoded route /api/rahasia
    std::string code1 = R"(
        modul TestRTRReal1
        fungsi utama() ->
            R = Router.baru()
            R2 = Router.get(R, "/api/rahasia", "handler_rahasia")
            Req = %{"path" => "/api/rahasia"}
            Router.dispatch(R2, Req)
        rampung
    )";
    auto res1 = engine.eval(code1);
    CHECK(res1.has_value(), "Test 1 must return dispatch response");
    auto* s1 = res1->get_if<std::string>();
    CHECK(s1 != nullptr && s1->find("handler_rahasia") != std::string::npos, "Test 1 unhardcoded route /api/rahasia match");
    std::cout << "[Test 1] Unhardcoded Route /api/rahasia Match: " << *s1 << " [PASS]\n";

    // Test 2: Dispatch unregistered route /api/lain -> 404
    std::string code2 = R"(
        modul TestRTRReal2
        fungsi utama() ->
            R = Router.baru()
            R2 = Router.get(R, "/api/rahasia", "handler_rahasia")
            Req = %{"path" => "/api/lain"}
            Router.dispatch(R2, Req)
        rampung
    )";
    auto res2 = engine.eval(code2);
    CHECK(res2.has_value(), "Test 2 must return 404 response");
    auto* s2 = res2->get_if<std::string>();
    CHECK(s2 != nullptr && s2->find("404 Route Not Found") != std::string::npos, "Test 2 unregistered route /api/lain 404 match");
    std::cout << "[Test 2] Dispatch Unregistered Route -> 404 Not Found [PASS]\n";

    // Test 3: Multiple routes /a and /b
    std::string code3 = R"(
        modul TestRTRReal3
        fungsi utama() ->
            R = Router.baru()
            R2 = Router.get(R, "/a", "handler_a")
            R3 = Router.get(R2, "/b", "handler_b")
            ReqA = %{"path" => "/a"}
            ReqB = %{"path" => "/b"}
            ResA = Router.dispatch(R3, ReqA)
            ResB = Router.dispatch(R3, ReqB)
            ResA ++ "_" ++ ResB
        rampung
    )";
    auto res3 = engine.eval(code3);
    CHECK(res3.has_value(), "Test 3 must return multi-route response");
    auto* s3 = res3->get_if<std::string>();
    CHECK(s3 != nullptr && s3->find("handler_a") != std::string::npos && s3->find("handler_b") != std::string::npos, "Test 3 multi-route match");
    std::cout << "[Test 3] Multiple Routes /a and /b Match [PASS]\n";

    // Test 4: Middleware conditional header (X-Logged ONLY when logging middleware added)
    std::string code4 = R"(
        modul TestRTRReal4
        fungsi utama() ->
            R = Router.baru()
            Req = %{"path" => "/a"}
            ResNoMw = Router.dispatch(R, Req)

            R2 = Router.middleware(R, "logging_mw")
            ResMw = Router.dispatch(R2, Req)

            ResNoMw ++ "|||" ++ ResMw
        rampung
    )";
    auto res4 = engine.eval(code4);
    CHECK(res4.has_value(), "Test 4 must return conditional middleware response");
    auto* s4 = res4->get_if<std::string>();
    std::size_t sep = s4 ? s4->find("|||") : std::string::npos;
    CHECK(sep != std::string::npos, "Test 4 separator match");
    std::string part_no = s4->substr(0, sep);
    std::string part_mw = s4->substr(sep + 3);
    CHECK(part_no.find("X-Logged") == std::string::npos, "Test 4 X-Logged must NOT be in un-middlewared response");
    CHECK(part_mw.find("X-Logged") != std::string::npos, "Test 4 X-Logged MUST be in middlewared response");
    std::cout << "[Test 4] Conditional Middleware Execution (X-Logged only when registered) [PASS]\n";

    // Test 5: Complex path params /users/:id/posts/:post_id
    std::string code5 = R"(
        modul TestRTRReal5
        fungsi utama() ->
            R = Router.baru()
            R2 = Router.get(R, "/users/:id/posts/:post_id", "handler_post")
            Req = %{"path" => "/users/42/posts/99"}
            Router.dispatch(R2, Req)
        rampung
    )";
    auto res5 = engine.eval(code5);
    CHECK(res5.has_value(), "Test 5 must return response with 42 and 99");
    auto* s5 = res5->get_if<std::string>();
    CHECK(s5 != nullptr && s5->find("42") != std::string::npos && s5->find("99") != std::string::npos, "Test 5 multi-param extraction match");
    std::cout << "[Test 5] Complex Path Params Extraction (/users/42/posts/99) [PASS]\n";

    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL 5 FASE 10.6 REAL ROUTER REGISTRY TESTS PASSED PERFECTLY!\n";
    std::cout << "========================================\n";
    return 0;
}
