// =====================================================================
// GARUDA COMPILER — FASE 10.7 GENERAL ROUTER PATTERN MATCHING TEST SUITE (RED -> GREEN)
// =====================================================================

#include "../include/garuda/beam/tools/script_engine.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <cstdlib>

int main() {
    garuda::beam::ScriptEngine engine;
    int failures = 0;

    std::cout << "=======================================================\n";
    std::cout << "GARUDA COMPILER — FASE 10.7 GENERAL ROUTER PATTERN TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test 1: Route /produk/:kode
    std::string code1 = R"(
        modul TestGenRTR1
        fungsi utama() ->
            R = Router.baru()
            R2 = Router.get(R, "/produk/:kode", "handler_produk")
            Req = %{"path" => "/produk/ABC-123"}
            Router.dispatch(R2, Req)
        rampung
    )";
    auto res1 = engine.eval(code1);
    std::string s1 = (res1.has_value() && res1->get_if<std::string>()) ? *res1->get_if<std::string>() : "";
    if (s1.find("ABC-123") != std::string::npos) {
        std::cout << "[Test 1] Route /produk/:kode -> 'ABC-123' [PASS]\n";
    } else {
        std::cout << "[Test 1] Route /produk/:kode -> [FAIL] Expected 'ABC-123', got: " << s1 << "\n";
        failures++;
    }

    // Test 2: Route /api/v1/items/:id
    std::string code2 = R"(
        modul TestGenRTR2
        fungsi utama() ->
            R = Router.baru()
            R2 = Router.get(R, "/api/v1/items/:id", "handler_item")
            Req = %{"path" => "/api/v1/items/999"}
            Router.dispatch(R2, Req)
        rampung
    )";
    auto res2 = engine.eval(code2);
    std::string s2 = (res2.has_value() && res2->get_if<std::string>()) ? *res2->get_if<std::string>() : "";
    if (s2.find("999") != std::string::npos) {
        std::cout << "[Test 2] Route /api/v1/items/:id -> '999' [PASS]\n";
    } else {
        std::cout << "[Test 2] Route /api/v1/items/:id -> [FAIL] Expected '999', got: " << s2 << "\n";
        failures++;
    }

    // Test 3: Route /files/:category/:name
    std::string code3 = R"(
        modul TestGenRTR3
        fungsi utama() ->
            R = Router.baru()
            R2 = Router.get(R, "/files/:category/:name", "handler_file")
            Req = %{"path" => "/files/image/logo.png"}
            Router.dispatch(R2, Req)
        rampung
    )";
    auto res3 = engine.eval(code3);
    std::string s3 = (res3.has_value() && res3->get_if<std::string>()) ? *res3->get_if<std::string>() : "";
    if (s3.find("image") != std::string::npos && s3.find("logo.png") != std::string::npos) {
        std::cout << "[Test 3] Route /files/:category/:name -> 'image' & 'logo.png' [PASS]\n";
    } else {
        std::cout << "[Test 3] Route /files/:category/:name -> [FAIL] Expected 'image' and 'logo.png', got: " << s3 << "\n";
        failures++;
    }

    // Test 4: Route /a/:x/b/:y/c/:z (3 params)
    std::string code4 = R"(
        modul TestGenRTR4
        fungsi utama() ->
            R = Router.baru()
            R2 = Router.get(R, "/a/:x/b/:y/c/:z", "handler_xyz")
            Req = %{"path" => "/a/1/b/2/c/3"}
            Router.dispatch(R2, Req)
        rampung
    )";
    auto res4 = engine.eval(code4);
    std::string s4 = (res4.has_value() && res4->get_if<std::string>()) ? *res4->get_if<std::string>() : "";
    if (s4.find("1") != std::string::npos && s4.find("2") != std::string::npos && s4.find("3") != std::string::npos) {
        std::cout << "[Test 4] Route /a/:x/b/:y/c/:z -> '1', '2', '3' [PASS]\n";
    } else {
        std::cout << "[Test 4] Route /a/:x/b/:y/c/:z -> [FAIL] Expected '1', '2', '3', got: " << s4 << "\n";
        failures++;
    }

    // Test 5: Pattern mismatch /produk/:kode vs /produk/ABC/extra -> 404 Not Found
    std::string code5 = R"(
        modul TestGenRTR5
        fungsi utama() ->
            R = Router.baru()
            R2 = Router.get(R, "/produk/:kode", "handler_produk")
            Req = %{"path" => "/produk/ABC/extra"}
            Router.dispatch(R2, Req)
        rampung
    )";
    auto res5 = engine.eval(code5);
    std::string s5 = (res5.has_value() && res5->get_if<std::string>()) ? *res5->get_if<std::string>() : "";
    if (s5.find("404 Route Not Found") != std::string::npos) {
        std::cout << "[Test 5] Pattern Mismatch -> 404 Route Not Found [PASS]\n";
    } else {
        std::cout << "[Test 5] Pattern Mismatch -> [FAIL] Expected 404, got: " << s5 << "\n";
        failures++;
    }

    if (failures > 0) {
        std::cout << "\n❌ " << failures << "/5 TESTS FAILED ON STUB CODE!\n";
        return 1;
    }

    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL 5 FASE 10.7 GENERAL ROUTER TESTS PASSED PERFECTLY!\n";
    std::cout << "========================================\n";
    return 0;
}
