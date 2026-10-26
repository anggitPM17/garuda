#include "../include/garuda/beam/stdlib/fullstack_bridge.hpp"
#include "../include/garuda/beam/stdlib/http.hpp"
#include <cassert>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <thread>

using namespace garuda::beam::stdlib;
namespace fs = std::filesystem;

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA FULLSTACK — SPRINT C.5 INTEGRATED WEB APP TEST SUITE (5 TESTS)\n";
    std::cout << "=======================================================\n\n";

    fs::path test_db_path = fs::temp_directory_path() / "garuda_fullstack_test.db";
    fs::remove(test_db_path);

    int port = 8085;
    FullStackApp app(test_db_path.string());

    // === Test 1: Start Fullstack App ===
    std::cout << "[TEST 1/5] Running Start Fullstack App test...\n";
    bool ok1 = app.start(port);
    assert(ok1);
    assert(app.is_running());
    std::cout << "  ✓ Test 1: Start Fullstack App PASS\n";

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // === Test 2: GET / Healthcheck ===
    std::cout << "[TEST 2/5] Running GET / Healthcheck test...\n";
    auto resp2 = Http::get("http://127.0.0.1:8085/");
    assert(resp2.has_value());
    assert(resp2->status_code == 200);
    assert(resp2->body.find("Garuda Fullstack") != std::string::npos);
    assert(resp2->body.find("\"status\":\"ok\"") != std::string::npos);
    std::cout << "  ✓ Test 2: GET / Healthcheck PASS\n";

    // === Test 3: POST /users Create User ===
    std::cout << "[TEST 3/5] Running POST /users Create User test...\n";
    std::string user_payload = "{\"nama\":\"Budi\",\"umur\":42}";
    auto resp3 = Http::post("http://127.0.0.1:8085/users", user_payload, "application/json");
    assert(resp3.has_value());
    assert(resp3->status_code == 201);
    assert(resp3->body.find("\"id\":\"usr_101\"") != std::string::npos);
    assert(resp3->body.find("\"status\":\"created\"") != std::string::npos);
    std::cout << "  ✓ Test 3: POST /users Create User PASS\n";

    // === Test 4: GET /users/usr_101 Fetch User from DB ===
    std::cout << "[TEST 4/5] Running GET /users/usr_101 Fetch User test...\n";
    auto resp4 = Http::get("http://127.0.0.1:8085/users/usr_101");
    assert(resp4.has_value());
    assert(resp4->status_code == 200);
    assert(resp4->body.find("Budi") != std::string::npos);
    assert(resp4->body.find("42") != std::string::npos);
    std::cout << "  ✓ Test 4: GET /users/usr_101 Fetch User PASS\n";

    // === Test 5: GET /users/usr_999 Non-Existent User 404 ===
    std::cout << "[TEST 5/5] Running GET /users/usr_999 Non-Existent User test...\n";
    auto resp5 = Http::get("http://127.0.0.1:8085/users/usr_999");
    assert(resp5.has_value());
    assert(resp5->status_code == 404);
    assert(resp5->body.find("not_found") != std::string::npos);
    std::cout << "  ✓ Test 5: GET /users/usr_999 Non-Existent User 404 PASS\n";

    // Stop App and Cleanup
    app.stop();
    assert(!app.is_running());
    fs::remove(test_db_path);

    std::cout << "\n🎉 ALL 5 FULLSTACK INTEGRATED APP TESTS PASSED PERFECTLY!\n";
    return 0;
}
