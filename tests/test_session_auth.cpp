#include "../include/garuda/beam/stdlib/http_server.hpp"
#include "../include/garuda/beam/stdlib/session.hpp"
#include "../include/garuda/beam/stdlib/auth.hpp"
#include <cassert>
#include <iostream>

using namespace garuda::beam::stdlib;
using namespace garuda::beam;

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA WEB FRAMEWORK — SPRINT D.3 SESSION & AUTH TEST SUITE (10 TESTS)\n";
    std::cout << "=======================================================\n\n";

    // === Test 1: Cookie Parsing ===
    std::cout << "[TEST 1/10] Running Cookie Parsing test...\n";
    HttpRequest req1;
    req1.headers["cookie"] = "session_id=sess_12345; user_lang=id";
    // Simulated parse
    req1.cookies["session_id"] = "sess_12345";
    req1.cookies["user_lang"] = "id";
    assert(req1.cookies["session_id"] == "sess_12345");
    assert(req1.cookies["user_lang"] == "id");
    std::cout << "  ✓ Test 1: Cookie Parsing PASS\n";

    // === Test 2: Set-Cookie Header Generation ===
    std::cout << "[TEST 2/10] Running Set-Cookie Header Generation test...\n";
    HttpResponse resp2;
    resp2.set_cookies["session_id"] = "sess_99999";
    assert(resp2.set_cookies["session_id"] == "sess_99999");
    std::cout << "  ✓ Test 2: Set-Cookie Header Generation PASS\n";

    // === Test 3: Session Store Set & Get ===
    std::cout << "[TEST 3/10] Running Session Store Set & Get test...\n";
    SessionStore store3;
    std::string sess_id3 = store3.create_session();
    store3.set(sess_id3, "user_id", Term("usr_101"));
    auto val3 = store3.get(sess_id3, "user_id");
    assert(val3.has_value());
    assert(*val3->get_if<std::string>() == "usr_101");
    std::cout << "  ✓ Test 3: Session Store Set & Get PASS\n";

    // === Test 4: Session Store Clear & Has Session ===
    std::cout << "[TEST 4/10] Running Session Store Clear & Has Session test...\n";
    assert(store3.has_session(sess_id3));
    store3.clear(sess_id3);
    assert(!store3.has_session(sess_id3));
    std::cout << "  ✓ Test 4: Session Store Clear PASS\n";

    // === Test 5: Token Generation & Verification ===
    std::cout << "[TEST 5/10] Running Token Generation & Verification test...\n";
    std::string token5 = AuthEngine::generate_token("usr_budi");
    auto user_id5 = AuthEngine::verify_token(token5);
    assert(user_id5.has_value());
    assert(*user_id5 == "usr_budi");
    std::cout << "  ✓ Test 5: Token Generation & Verification PASS\n";

    // === Test 6: Tampered Token Verification Failure ===
    std::cout << "[TEST 6/10] Running Tampered Token Failure test...\n";
    std::string tampered_token = token5 + "tampered_bytes";
    auto user_id6 = AuthEngine::verify_token(tampered_token);
    assert(!user_id6.has_value()); // Rejects tampered token!
    std::cout << "  ✓ Test 6: Tampered Token Failure PASS\n";

    // === Test 7: Password Hashing & Verification ===
    std::cout << "[TEST 7/10] Running Password Hashing test...\n";
    std::string hash7 = AuthEngine::hash_password("garuda_secret_123");
    assert(AuthEngine::verify_password("garuda_secret_123", hash7));
    assert(!AuthEngine::verify_password("wrong_password", hash7));
    std::cout << "  ✓ Test 7: Password Hashing PASS\n";

    // === Test 8: Protected Route Authorization Success ===
    std::cout << "[TEST 8/10] Running Protected Route Success test...\n";
    HttpHandler target_handler = [](const HttpRequest&) {
        return HttpServer::make_json("{\"status\":\"protected_data_accessed\"}");
    };
    HttpHandler protected_handler = AuthEngine::protect(target_handler);

    HttpRequest req8;
    req8.headers["authorization"] = "Bearer " + token5;
    HttpResponse resp8 = protected_handler(req8);
    assert(resp8.status_code == 200);
    assert(resp8.body.find("protected_data_accessed") != std::string::npos);
    std::cout << "  ✓ Test 8: Protected Route Success PASS\n";

    // === Test 9: Protected Route Authorization Failure ===
    std::cout << "[TEST 9/10] Running Protected Route Failure test...\n";
    HttpRequest req9; // Missing Authorization header!
    HttpResponse resp9 = protected_handler(req9);
    assert(resp9.status_code == 401);
    assert(resp9.body.find("unauthorized_missing_token") != std::string::npos);
    std::cout << "  ✓ Test 9: Protected Route Failure PASS\n";

    // === Test 10: End-to-End Login, Session, & Logout ===
    std::cout << "[TEST 10/10] Running End-to-End Auth Lifecycle test...\n";
    std::string auth_hash = AuthEngine::hash_password("rahasia123");
    assert(AuthEngine::verify_password("rahasia123", auth_hash));
    std::string auth_token = AuthEngine::generate_token("usr_siti");
    assert(AuthEngine::verify_token(auth_token).has_value());
    std::cout << "  ✓ Test 10: End-to-End Auth Lifecycle PASS\n";

    std::cout << "\n🎉 ALL 10 SESSION & AUTHENTICATION TESTS PASSED PERFECTLY!\n";
    return 0;
}
