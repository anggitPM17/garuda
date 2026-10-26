#include "../include/garuda/beam/stdlib/live_view.hpp"
#include "../include/garuda/beam/stdlib/http_server.hpp"
#include <cassert>
#include <iostream>

using namespace garuda::beam::stdlib;
using namespace garuda::beam;

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA WEB FRAMEWORK — SPRINT D.5 REAL-TIME LIVEVIEW TEST SUITE (10 TESTS)\n";
    std::cout << "=======================================================\n\n";

    LiveViewEngine lv;

    // === Test 1: LiveView Mount & Initial State Setup ===
    std::cout << "[TEST 1/10] Running LiveView Mount test...\n";
    Term state1 = list_of({tuple_of({Term("count"), Term("0")})});
    std::string html1 = lv.mount("sess_counter", "<div id=\"counter\">Count: {{ count }}</div>", state1);
    assert(html1 == "<div id=\"counter\">Count: 0</div>");
    assert(lv.get_state("sess_counter").has_value());
    std::cout << "  ✓ Test 1: LiveView Mount PASS\n";

    // === Test 2: LiveView Initial Rendered HTML Verification ===
    std::cout << "[TEST 2/10] Running Initial Rendered HTML Verification test...\n";
    auto rendered2 = lv.get_rendered_html("sess_counter");
    assert(rendered2.has_value());
    assert(*rendered2 == "<div id=\"counter\">Count: 0</div>");
    std::cout << "  ✓ Test 2: Initial Rendered HTML Verification PASS\n";

    // === Test 3: WebSocket Upgrade Response Generation ===
    std::cout << "[TEST 3/10] Running WebSocket Upgrade Response test...\n";
    HttpResponse ws_resp3 = HttpServer::make_websocket_upgrade_response("dGhlIHNhbXBsZSBub25jZQ==");
    assert(ws_resp3.status_code == 101);
    assert(ws_resp3.headers["Upgrade"] == "websocket");
    assert(ws_resp3.headers.find("Sec-WebSocket-Accept") != ws_resp3.headers.end());
    std::cout << "  ✓ Test 3: WebSocket Upgrade Response PASS\n";

    // === Test 4: Event Handling Pipeline (click:inc) ===
    std::cout << "[TEST 4/10] Running Event Handling Pipeline test...\n";
    EventHandler inc_handler = [](const std::string& evt, const Term&, const Term& current_state) -> Term {
        if (evt == "inc") {
            return list_of({tuple_of({Term("count"), Term("1")})});
        }
        return current_state;
    };

    auto diff4_opt = lv.push_event("sess_counter", "inc", Term(), inc_handler);
    assert(diff4_opt.has_value());
    assert(diff4_opt->find("\"event\": \"inc\"") != std::string::npos || diff4_opt->find("inc") != std::string::npos);
    assert(diff4_opt->find("Count: 1") != std::string::npos);
    std::cout << "  ✓ Test 4: Event Handling Pipeline PASS\n";

    // === Test 5: DOM Diff Engine Generation ===
    std::cout << "[TEST 5/10] Running DOM Diff Engine Generation test...\n";
    std::string diff_patch5 = LiveViewEngine::compute_dom_diff("<div id=\"counter\">Count: 0</div>", "<div id=\"counter\">Count: 1</div>");
    assert(diff_patch5.find("\"diff\":\"updated\"") != std::string::npos);
    std::cout << "  ✓ Test 5: DOM Diff Engine Generation PASS\n";

    // === Test 6: State Preservation Across Event Pipeline ===
    std::cout << "[TEST 6/10] Running State Preservation Across Event test...\n";
    auto current_st6 = lv.get_state("sess_counter");
    assert(current_st6.has_value());
    assert(lv.get_rendered_html("sess_counter") == "<div id=\"counter\">Count: 1</div>");
    std::cout << "  ✓ Test 6: State Preservation Across Event PASS\n";

    // === Test 7: Multiple Event Sequence (inc -> inc -> dec) ===
    std::cout << "[TEST 7/10] Running Multiple Event Sequence test...\n";
    EventHandler multi_handler = [](const std::string& evt, const Term&, const Term& current_state) -> Term {
        std::int64_t current_count = 1;
        if (auto* list_val = current_state.get_if<List>()) {
            for (const auto& item : seq_items(list_val->items)) {
                if (auto* tup = item.get_if<Tuple>()) {
                    const auto& pair = seq_items(tup->items);
                    if (pair.size() == 2 && pair[0].get_if<std::string>() && *pair[0].get_if<std::string>() == "count") {
                        if (pair[1].get_if<std::string>()) current_count = std::stoll(*pair[1].get_if<std::string>());
                    }
                }
            }
        }
        if (evt == "inc") current_count++;
        if (evt == "dec") current_count--;
        return list_of({tuple_of({Term("count"), Term(std::to_string(current_count))})});
    };

    lv.push_event("sess_counter", "inc", Term(), multi_handler); // count -> 2
    lv.push_event("sess_counter", "dec", Term(), multi_handler); // count -> 1
    assert(lv.get_rendered_html("sess_counter") == "<div id=\"counter\">Count: 1</div>");
    std::cout << "  ✓ Test 7: Multiple Event Sequence PASS\n";

    // === Test 8: Concurrent LiveView Sessions Isolation ===
    std::cout << "[TEST 8/10] Running Concurrent LiveView Sessions Isolation test...\n";
    Term state8_A = list_of({tuple_of({Term("count"), Term("100")})});
    Term state8_B = list_of({tuple_of({Term("count"), Term("200")})});

    lv.mount("sess_user_A", "<p>{{ count }}</p>", state8_A);
    lv.mount("sess_user_B", "<p>{{ count }}</p>", state8_B);

    assert(lv.get_rendered_html("sess_user_A") == "<p>100</p>");
    assert(lv.get_rendered_html("sess_user_B") == "<p>200</p>");
    std::cout << "  ✓ Test 8: Concurrent LiveView Sessions Isolation PASS\n";

    // === Test 9: LiveView + ViewEngine Integration ===
    std::cout << "[TEST 9/10] Running LiveView + ViewEngine Integration test...\n";
    Term state9 = list_of({tuple_of({Term("user"), Term("Aura")})});
    std::string html9 = lv.mount("sess_view", "<h2>Hello {{ user }}</h2>", state9);
    assert(html9 == "<h2>Hello Aura</h2>");
    std::cout << "  ✓ Test 9: LiveView + ViewEngine Integration PASS\n";

    // === Test 10: End-to-End LiveView Session Lifecycle & Unmount ===
    std::cout << "[TEST 10/10] Running End-to-End Session Unmount test...\n";
    lv.unmount("sess_counter");
    assert(!lv.get_state("sess_counter").has_value());
    assert(!lv.get_rendered_html("sess_counter").has_value());
    std::cout << "  ✓ Test 10: End-to-End Session Unmount PASS\n";

    std::cout << "\n🎉 ALL 10 REAL-TIME LIVEVIEW ENGINE TESTS PASSED PERFECTLY!\n";
    return 0;
}
