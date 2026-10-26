#include "../include/garuda/beam/stdlib/view.hpp"
#include "../include/garuda/beam/stdlib/web.hpp"
#include <cassert>
#include <iostream>

using namespace garuda::beam::stdlib;
using namespace garuda::beam;

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA WEB FRAMEWORK — SPRINT D.2 VIEW ENGINE TEST SUITE (10 TESTS)\n";
    std::cout << "=======================================================\n\n";

    // === Test 1: Simple Variable Substitution ===
    std::cout << "[TEST 1/10] Running Simple Variable Substitution test...\n";
    Term data1 = list_of({tuple_of({Term("nama"), Term("Budi")})});
    std::string res1 = ViewEngine::render_template("<h1>Halo {{ nama }}</h1>", data1);
    assert(res1 == "<h1>Halo Budi</h1>");
    std::cout << "  ✓ Test 1: Simple Variable Substitution PASS\n";

    // === Test 2: Numeric Variable Substitution ===
    std::cout << "[TEST 2/10] Running Numeric Variable Substitution test...\n";
    Term data2 = list_of({tuple_of({Term("umur"), Term(42LL)})});
    std::string res2 = ViewEngine::render_template("<p>Umur: {{ umur }}</p>", data2);
    assert(res2 == "<p>Umur: 42</p>");
    std::cout << "  ✓ Test 2: Numeric Variable Substitution PASS\n";

    // === Test 3: Conditional Block True ===
    std::cout << "[TEST 3/10] Running Conditional Block True test...\n";
    Term data3 = list_of({tuple_of({Term("is_admin"), Term("true")})});
    std::string res3 = ViewEngine::render_template("{% if is_admin %}Panel Admin{% endif %}", data3);
    assert(res3 == "Panel Admin");
    std::cout << "  ✓ Test 3: Conditional Block True PASS\n";

    // === Test 4: Conditional Block False with Else ===
    std::cout << "[TEST 4/10] Running Conditional Block False with Else test...\n";
    Term data4 = list_of({tuple_of({Term("is_admin"), Term("false")})});
    std::string res4 = ViewEngine::render_template("{% if is_admin %}Admin{% else %}Pengguna Biasa{% endif %}", data4);
    assert(res4 == "Pengguna Biasa");
    std::cout << "  ✓ Test 4: Conditional Block False with Else PASS\n";

    // === Test 5: Loop Block ===
    std::cout << "[TEST 5/10] Running Loop Block test...\n";
    Term data5 = list_of({tuple_of({Term("items"), list_of({Term("Aura"), Term("Budi"), Term("Candra")})})});
    std::string res5 = ViewEngine::render_template("<ul>{% for item in items %}<li>{{ item }}</li>{% endfor %}</ul>", data5);
    assert(res5 == "<ul><li>Aura</li><li>Budi</li><li>Candra</li></ul>");
    std::cout << "  ✓ Test 5: Loop Block PASS\n";

    // === Test 6: Layout Inheritance ===
    std::cout << "[TEST 6/10] Running Layout Inheritance test...\n";
    std::string layout6 = "<html><body>{{ content }}</body></html>";
    std::string tpl6 = "<h2>Halo {{ nama }}</h2>";
    Term data6 = list_of({tuple_of({Term("nama"), Term("Siti")})});
    std::string res6 = ViewEngine::render_with_layout(layout6, tpl6, data6);
    assert(res6 == "<html><body><h2>Halo Siti</h2></body></html>");
    std::cout << "  ✓ Test 6: Layout Inheritance PASS\n";

    // === Test 7: Multi-Variable Substitution ===
    std::cout << "[TEST 7/10] Running Multi-Variable Substitution test...\n";
    Term data7 = list_of({tuple_of({Term("a"), Term("10")}), tuple_of({Term("b"), Term("20")})});
    std::string res7 = ViewEngine::render_template("{{ a }} + {{ b }}", data7);
    assert(res7 == "10 + 20");
    std::cout << "  ✓ Test 7: Multi-Variable Substitution PASS\n";

    // === Test 8: View Engine + WebFramework Integration ===
    std::cout << "[TEST 8/10] Running WebFramework Integration test...\n";
    HttpResponse resp8 = WebFramework::render_with_layout("<html>{{ content }}</html>", "<h1>{{ title }}</h1>", list_of({tuple_of({Term("title"), Term("Garuda Web")})}));
    assert(resp8.status_code == 200);
    assert(resp8.headers["content-type"] == "text/html");
    assert(resp8.body == "<html><h1>Garuda Web</h1></html>");
    std::cout << "  ✓ Test 8: WebFramework Integration PASS\n";

    // === Test 9: HTML View Formatting ===
    std::cout << "[TEST 9/10] Running HTML View Formatting test...\n";
    std::string res9 = ViewEngine::render_template("<div class=\"user\">{{ name }}</div>", list_of({tuple_of({Term("name"), Term("Eko")})}));
    assert(res9 == "<div class=\"user\">Eko</div>");
    std::cout << "  ✓ Test 9: HTML View Formatting PASS\n";

    // === Test 10: Graceful Handling for Missing Variables ===
    std::cout << "[TEST 10/10] Running Missing Variables Graceful Handling test...\n";
    std::string res10 = ViewEngine::render_template("<p>User: {{ missing_var }}</p>", list_of({}));
    assert(res10 == "<p>User: </p>"); // Gracefully resolves to empty string!
    std::cout << "  ✓ Test 10: Missing Variables Graceful Handling PASS\n";

    std::cout << "\n🎉 ALL 10 VIEW ENGINE TESTS PASSED PERFECTLY!\n";
    return 0;
}
