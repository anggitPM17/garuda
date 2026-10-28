// =====================================================================
// FASE G5.1 - GARUDA WEBRTC SIGNALING TESTS
// Memverifikasi webrtc_signaling.hpp
// =====================================================================
#include "../include/garuda/beam/webrtc/webrtc_signaling.hpp"
#include <iostream>
#include <string>
#include <cstdlib>

using namespace garuda::beam;

#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

int main() {
    std::cout << "========================================\n";
    std::cout << "FASE G5.1 - GARUDA WEBRTC SIGNALING TESTS\n";
    std::cout << "========================================\n\n";

    WebRTCSignaling ws;

    // Test 1: Call Offer
    std::cout << "[Test 1] Call Offer...\n";
    {
        auto call_opt = ws.call_offer("alice", "bob", true);
        CHECK(call_opt.has_value(), "Call offer harus mengembalikan CallSession");
        CHECK(call_opt->state == CallState::RINGING, "State panggilan awal harus RINGING");
        CHECK(call_opt->video_enabled == true, "Video harus enabled");
        CHECK(ws.active_call_count() == 1, "Active call count harus 1");
    }
    std::cout << "PASS\n\n";

    // Test 2: Call Answer
    std::cout << "[Test 2] Call Answer...\n";
    {
        auto call_opt = ws.call_offer("alice", "charlie");
        std::string call_id = call_opt->call_id;

        bool answered = ws.call_answer(call_id, "charlie", "v=0\r\nsdp_answer_data");
        CHECK(answered, "Call answer harus return true");

        auto call = ws.get_call(call_id);
        CHECK(call->state == CallState::ACTIVE, "State panggilan setelah answer harus ACTIVE");
        CHECK(call->sdp_answer == "v=0\r\nsdp_answer_data", "SDP answer harus cocok");
    }
    std::cout << "PASS\n\n";

    // Test 3: Call Reject
    std::cout << "[Test 3] Call Reject...\n";
    {
        auto call_opt = ws.call_offer("david", "eve");
        std::string call_id = call_opt->call_id;

        bool rejected = ws.call_reject(call_id, "eve");
        CHECK(rejected, "Call reject harus return true");

        auto call = ws.get_call(call_id);
        CHECK(call->state == CallState::REJECTED, "State setelah reject harus REJECTED");
    }
    std::cout << "PASS\n\n";

    // Test 4: Call End
    std::cout << "[Test 4] Call End...\n";
    {
        auto call_opt = ws.call_offer("alice", "bob");
        std::string call_id = call_opt->call_id;
        ws.call_answer(call_id, "bob", "answer");

        bool ended = ws.call_end(call_id, "alice");
        CHECK(ended, "Call end harus return true");

        auto call = ws.get_call(call_id);
        CHECK(call->state == CallState::ENDED, "State setelah end harus ENDED");
    }
    std::cout << "PASS\n\n";

    // Test 5: ICE Candidates Propagation
    std::cout << "[Test 5] ICE Candidates Propagation...\n";
    {
        auto call_opt = ws.call_offer("user1", "user2");
        std::string call_id = call_opt->call_id;

        ws.add_ice_candidate(call_id, "candidate:1 1 UDP 2122260223 192.168.1.5 50000 typ host");
        ws.add_ice_candidate(call_id, "candidate:2 1 UDP 2122260223 192.168.1.6 50001 typ host");

        auto call = ws.get_call(call_id);
        CHECK(call->ice_candidates.size() == 2, "ICE candidates count harus 2");
    }
    std::cout << "PASS\n\n";

    std::cout << "========================================\n";
    std::cout << "FASE G5.1 - WEBRTC SIGNALING SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
