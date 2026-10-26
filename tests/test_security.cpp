#include "../include/garuda/beam/stdlib/crypto.hpp"
#include "../include/garuda/beam/stdlib/csrf.hpp"
#include "../include/garuda/beam/stdlib/auth.hpp"
#include "../include/garuda/beam/stdlib/view.hpp"
#include <cassert>
#include <iostream>

using namespace garuda::beam::stdlib;
using namespace garuda::beam;

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA SECURITY ENGINE — SPRINT E.2 SECURITY HARDENING TEST SUITE (15 TESTS)\n";
    std::cout << "=======================================================\n\n";

    // === Test 1: SHA-256 String Hashing ===
    std::cout << "[TEST 1/15] Running SHA-256 String Hashing test...\n";
    std::string h1 = Sha256::hash("hello");
    assert(h1 == "2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824");
    std::cout << "  ✓ Test 1: SHA-256 String Hashing PASS\n";

    // === Test 2: Salted Password Hash Generation ===
    std::cout << "[TEST 2/15] Running Salted Password Hash Generation test...\n";
    std::string salted_hash2 = AuthEngine::hash_password("garuda_2026");
    assert(salted_hash2.find(':') != std::string::npos);
    std::cout << "  ✓ Test 2: Salted Password Hash Generation PASS\n";

    // === Test 3: Salted Password Verification Success ===
    std::cout << "[TEST 3/15] Running Password Verification Success test...\n";
    assert(AuthEngine::verify_password("garuda_2026", salted_hash2));
    std::cout << "  ✓ Test 3: Password Verification Success PASS\n";

    // === Test 4: Salted Password Verification Failure ===
    std::cout << "[TEST 4/15] Running Password Verification Failure test...\n";
    assert(!AuthEngine::verify_password("wrong_password", salted_hash2));
    std::cout << "  ✓ Test 4: Password Verification Failure PASS\n";

    // === Test 5: Salt Randomness Check ===
    std::cout << "[TEST 5/15] Running Salt Randomness Check test...\n";
    std::string salted_hash5_a = AuthEngine::hash_password("garuda_2026");
    std::string salted_hash5_b = AuthEngine::hash_password("garuda_2026");
    assert(salted_hash5_a != salted_hash5_b); // Unique salts produced different hashes!
    assert(AuthEngine::verify_password("garuda_2026", salted_hash5_a));
    assert(AuthEngine::verify_password("garuda_2026", salted_hash5_b));
    std::cout << "  ✓ Test 5: Salt Randomness Check PASS\n";

    // === Test 6: CSRF Token Generation ===
    std::cout << "[TEST 6/15] Running CSRF Token Generation test...\n";
    std::string csrf_tok6 = CsrfEngine::generate_token();
    assert(csrf_tok6.rfind("csrf_", 0) == 0);
    assert(csrf_tok6.length() > 20);
    std::cout << "  ✓ Test 6: CSRF Token Generation PASS\n";

    // === Test 7: CSRF Protection Middleware Success ===
    std::cout << "[TEST 7/15] Running CSRF Protection Success test...\n";
    HttpHandler base_handler = [](const HttpRequest&) {
        return HttpServer::make_json("{\"status\":\"ok_csrf_passed\"}");
    };
    HttpHandler protected_csrf = CsrfEngine::protect(base_handler, csrf_tok6);

    HttpRequest req7;
    req7.method = "POST";
    req7.headers["x-csrf-token"] = csrf_tok6;
    HttpResponse resp7 = protected_csrf(req7);
    assert(resp7.status_code == 200);
    assert(resp7.body.find("ok_csrf_passed") != std::string::npos);
    std::cout << "  ✓ Test 7: CSRF Protection Success PASS\n";

    // === Test 8: CSRF Protection Failure (Missing Token) ===
    std::cout << "[TEST 8/15] Running CSRF Protection Missing Token Failure test...\n";
    HttpRequest req8;
    req8.method = "POST"; // Missing X-CSRF-Token header!
    HttpResponse resp8 = protected_csrf(req8);
    assert(resp8.status_code == 403);
    assert(resp8.body.find("csrf_token_invalid_or_missing") != std::string::npos);
    std::cout << "  ✓ Test 8: CSRF Protection Missing Token Failure PASS\n";

    // === Test 9: CSRF Protection Failure (Invalid Token) ===
    std::cout << "[TEST 9/15] Running CSRF Protection Invalid Token Failure test...\n";
    HttpRequest req9;
    req9.method = "POST";
    req9.headers["x-csrf-token"] = "invalid_csrf_token_value";
    HttpResponse resp9 = protected_csrf(req9);
    assert(resp9.status_code == 403);
    std::cout << "  ✓ Test 9: CSRF Protection Invalid Token Failure PASS\n";

    // === Test 10: XSS HTML Escaping (<script>) ===
    std::cout << "[TEST 10/15] Running XSS HTML Escaping (<script>) test...\n";
    std::string xss10 = ViewEngine::escape_html("<script>alert('xss')</script>");
    assert(xss10 == "&lt;script&gt;alert(&#39;xss&#39;)&lt;/script&gt;");
    std::cout << "  ✓ Test 10: XSS HTML Escaping (<script>) PASS\n";

    // === Test 11: XSS Quote & Ampersand Escaping ===
    std::cout << "[TEST 11/15] Running XSS Quote & Ampersand Escaping test...\n";
    std::string xss11 = ViewEngine::escape_html("\"hello & world\"");
    assert(xss11 == "&quot;hello &amp; world&quot;");
    std::cout << "  ✓ Test 11: XSS Quote & Ampersand Escaping PASS\n";

    // === Test 12: XSS View Engine Variable Auto-Escaping ===
    std::cout << "[TEST 12/15] Running XSS Auto-Escaping in View Engine test...\n";
    Term data12 = list_of({tuple_of({Term("user_input"), Term("<img src=x onerror=alert(1)>")})});
    std::string res12 = ViewEngine::render_template("<div>{{ user_input }}</div>", data12);
    assert(res12 == "<div>&lt;img src=x onerror=alert(1)&gt;</div>");
    std::cout << "  ✓ Test 12: XSS Auto-Escaping in View Engine PASS\n";

    // === Test 13: Raw Unescaped View Substitution ===
    std::cout << "[TEST 13/15] Running Raw Unescaped View Substitution test...\n";
    Term data13 = list_of({tuple_of({Term("raw_html"), Term("<b>Bold Text</b>")})});
    std::string res13 = ViewEngine::render_template("<div>{{ raw_html }}</div>", data13);
    assert(res13 == "<div><b>Bold Text</b></div>"); // Unescaped raw substitution!
    std::cout << "  ✓ Test 13: Raw Unescaped View Substitution PASS\n";

    // === Test 14: Auth Engine SHA-256 Integration Check ===
    std::cout << "[TEST 14/15] Running Auth Engine SHA-256 Integration test...\n";
    std::string token14 = AuthEngine::generate_token("usr_sec_101");
    auto user14 = AuthEngine::verify_token(token14);
    assert(user14.has_value() && *user14 == "usr_sec_101");
    std::cout << "  ✓ Test 14: Auth Engine SHA-256 Integration PASS\n";

    // === Test 15: End-to-End Security Hardening Lifecycle ===
    std::cout << "[TEST 15/15] Running End-to-End Security Hardening Lifecycle test...\n";
    std::string pass_hash15 = AuthEngine::hash_password("SuperSecretPass123!");
    assert(AuthEngine::verify_password("SuperSecretPass123!", pass_hash15));
    std::cout << "  ✓ Test 15: End-to-End Security Hardening Lifecycle PASS\n";

    std::cout << "\n🎉 ALL 15 SECURITY HARDENING TESTS PASSED PERFECTLY!\n";
    return 0;
}
