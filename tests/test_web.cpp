#include "../include/garuda/beam/stdlib/web.hpp"
#include "../include/garuda/beam/stdlib/http.hpp"
#include "../include/garuda/beam/stdlib/db.hpp"
#include <cassert>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <thread>

using namespace garuda::beam::stdlib;
using namespace garuda::beam;
namespace fs = std::filesystem;

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA WEB FRAMEWORK — SPRINT D.1 CORE WEB FRAMEWORK TEST SUITE (10 TESTS)\n";
    std::cout << "=======================================================\n\n";

    fs::path test_db_file = fs::temp_directory_path() / "garuda_web_app.db";
    fs::remove(test_db_file);

    Database db(test_db_file.string());
    db.insert("users", "usr_101", Term("Budi Developer"));

    WebFramework app;

    // Route 1: Static GET /
    app.get("/", [](const HttpRequest&) {
        return WebFramework::json_response("{\"app\":\"Garuda Web Framework\",\"status\":\"ok\"}");
    });

    // Route 2: Dynamic GET /users/:id
    app.get("/users/:id", [&db](const HttpRequest& req) {
        auto it = req.path_params.find("id");
        if (it != req.path_params.end()) {
            auto rec = db.get("users", it->second);
            if (rec.has_value()) {
                std::string user_name = rec->get_if<std::string>() ? *rec->get_if<std::string>() : to_string(*rec);
                return WebFramework::json_response("{\"id\":\"" + it->second + "\",\"nama\":\"" + user_name + "\"}");
            }
        }
        return WebFramework::not_found_response("{\"error\":\"user_not_found\"}");
    });

    // Route 3: Nested Dynamic GET /posts/:post_id/comments/:comment_id
    app.get("/posts/:post_id/comments/:comment_id", [](const HttpRequest& req) {
        std::string p_id = req.path_params.at("post_id");
        std::string c_id = req.path_params.at("comment_id");
        return WebFramework::json_response("{\"post\":\"" + p_id + "\",\"comment\":\"" + c_id + "\"}");
    });

    // Route 4: POST /users
    app.post("/users", [&db](const HttpRequest& req) {
        std::string new_id = "usr_102";
        db.insert("users", new_id, Term(req.body));
        return WebFramework::json_response("{\"status\":\"created\",\"id\":\"" + new_id + "\"}", 201);
    });

    // Route 5: GET /html
    app.get("/html", [](const HttpRequest&) {
        return WebFramework::html_response("<h1>Halo Garuda Web Framework</h1>");
    });

    int port = 8090;

    // === Test 1: Start Server ===
    std::cout << "[TEST 1/10] Running Web Framework Server Start test...\n";
    bool ok1 = app.start(port);
    assert(ok1);
    assert(app.is_running());
    std::cout << "  ✓ Test 1: Web Framework Server Start PASS\n";

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // === Test 2: Static Route Matching (GET /) ===
    std::cout << "[TEST 2/10] Running Static Route Matching test...\n";
    auto resp2 = Http::get("http://127.0.0.1:8090/");
    assert(resp2.has_value() && resp2->status_code == 200);
    assert(resp2->body.find("Garuda Web Framework") != std::string::npos);
    std::cout << "  ✓ Test 2: Static Route Matching PASS\n";

    // === Test 3: Dynamic Path Parameter Extraction (/users/:id) ===
    std::cout << "[TEST 3/10] Running Dynamic Path Parameter Extraction test...\n";
    auto resp3 = Http::get("http://127.0.0.1:8090/users/usr_101");
    assert(resp3.has_value() && resp3->status_code == 200);
    assert(resp3->body.find("usr_101") != std::string::npos);
    assert(resp3->body.find("Budi Developer") != std::string::npos);
    std::cout << "  ✓ Test 3: Dynamic Path Parameter Extraction PASS\n";

    // === Test 4: Nested Dynamic Path Parameter Extraction ===
    std::cout << "[TEST 4/10] Running Nested Dynamic Path Parameter test...\n";
    auto resp4 = Http::get("http://127.0.0.1:8090/posts/p99/comments/c42");
    assert(resp4.has_value() && resp4->status_code == 200);
    assert(resp4->body.find("\"post\":\"p99\"") != std::string::npos);
    assert(resp4->body.find("\"comment\":\"c42\"") != std::string::npos);
    std::cout << "  ✓ Test 4: Nested Dynamic Path Parameter PASS\n";

    // === Test 5: Controller POST Request ===
    std::cout << "[TEST 5/10] Running Controller POST Request test...\n";
    auto resp5 = Http::post("http://127.0.0.1:8090/users", "Siti Developer");
    assert(resp5.has_value() && resp5->status_code == 201);
    assert(resp5->body.find("usr_102") != std::string::npos);
    std::cout << "  ✓ Test 5: Controller POST Request PASS\n";

    // === Test 6: Unmatched Route 404 Handling ===
    std::cout << "[TEST 6/10] Running Unmatched Route 404 Handling test...\n";
    auto resp6 = Http::get("http://127.0.0.1:8090/users/non_existent_usr");
    assert(resp6.has_value() && resp6->status_code == 404);
    assert(resp6->body.find("user_not_found") != std::string::npos);
    std::cout << "  ✓ Test 6: Unmatched Route 404 Handling PASS\n";

    // === Test 7: HTML Response Helper ===
    std::cout << "[TEST 7/10] Running HTML Response Helper test...\n";
    auto resp7 = Http::get("http://127.0.0.1:8090/html");
    assert(resp7.has_value() && resp7->status_code == 200);
    assert(resp7->headers["content-type"] == "text/html");
    assert(resp7->body.find("<h1>Halo Garuda Web Framework</h1>") != std::string::npos);
    std::cout << "  ✓ Test 7: HTML Response Helper PASS\n";

    // === Test 8: Controller + Database Query Integration ===
    std::cout << "[TEST 8/10] Running Controller + Database Query Integration test...\n";
    auto resp8 = Http::get("http://127.0.0.1:8090/users/usr_102");
    assert(resp8.has_value() && resp8->status_code == 200);
    assert(resp8->body.find("Siti Developer") != std::string::npos);
    std::cout << "  ✓ Test 8: Controller + Database Query Integration PASS\n";

    // === Test 9: Redirect Response Helper ===
    std::cout << "[TEST 9/10] Running Redirect Response Helper test...\n";
    HttpResponse red = WebFramework::redirect_response("http://localhost:8090/");
    assert(red.status_code == 302);
    assert(red.headers["Location"] == "http://localhost:8090/");
    std::cout << "  ✓ Test 9: Redirect Response Helper PASS\n";

    // === Test 10: Graceful Web Framework Shutdown ===
    std::cout << "[TEST 10/10] Running Graceful Web Framework Shutdown test...\n";
    app.stop();
    assert(!app.is_running());
    fs::remove(test_db_file);
    std::cout << "  ✓ Test 10: Graceful Web Framework Shutdown PASS\n";

    std::cout << "\n🎉 ALL 10 WEB FRAMEWORK CORE TESTS PASSED PERFECTLY!\n";
    return 0;
}
