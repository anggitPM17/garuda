#pragma once
// =====================================================================
// FASE G5.1 - WebRTC Signaling: Call offer/answer, ICE, reject, end
// =====================================================================
#include <string>
#include <map>
#include <vector>
#include <mutex>
#include <optional>
#include <chrono>
#include <cstddef>

namespace garuda::beam {

enum class CallState { IDLE, RINGING, ACTIVE, ENDED, REJECTED };

struct CallSession {
    std::string call_id;
    std::string caller_id;
    std::string callee_id;
    CallState state = CallState::IDLE;
    std::string sdp_offer;
    std::string sdp_answer;
    std::vector<std::string> ice_candidates;
    std::chrono::steady_clock::time_point started_at;
    bool video_enabled = false;
};

class WebRTCSignaling {
private:
    std::map<std::string, CallSession> calls_;  // call_id -> CallSession
    mutable std::mutex mu_;

public:
    std::optional<CallSession> call_offer(const std::string& caller_id,
                                          const std::string& callee_id,
                                          bool video = false) {
        if (caller_id.empty() || callee_id.empty()) return std::nullopt;

        std::lock_guard<std::mutex> lock(mu_);
        std::string call_id = "call_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());

        CallSession session;
        session.call_id = call_id;
        session.caller_id = caller_id;
        session.callee_id = callee_id;
        session.state = CallState::RINGING;
        session.sdp_offer = "v=0\r\no=- " + call_id + " IN IP4 127.0.0.1\r\ns=GarudaCall\r\n";
        session.started_at = std::chrono::steady_clock::now();
        session.video_enabled = video;

        calls_[call_id] = session;
        return session;
    }

    bool call_answer(const std::string& call_id,
                     const std::string& callee_id,
                     const std::string& sdp_answer) {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = calls_.find(call_id);
        if (it == calls_.end()) return false;

        auto& call = it->second;
        if (call.callee_id != callee_id) return false;

        call.sdp_answer = sdp_answer;
        call.state = CallState::ACTIVE;
        return true;
    }

    bool call_reject(const std::string& call_id, const std::string& user_id) {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = calls_.find(call_id);
        if (it == calls_.end()) return false;

        auto& call = it->second;
        if (call.caller_id != user_id && call.callee_id != user_id) return false;

        call.state = CallState::REJECTED;
        return true;
    }

    bool call_end(const std::string& call_id, const std::string& user_id) {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = calls_.find(call_id);
        if (it == calls_.end()) return false;

        auto& call = it->second;
        if (call.caller_id != user_id && call.callee_id != user_id) return false;

        call.state = CallState::ENDED;
        return true;
    }

    bool add_ice_candidate(const std::string& call_id, const std::string& candidate) {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = calls_.find(call_id);
        if (it == calls_.end()) return false;

        it->second.ice_candidates.push_back(candidate);
        return true;
    }

    std::optional<CallSession> get_call(const std::string& call_id) const {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = calls_.find(call_id);
        if (it != calls_.end()) return it->second;
        return std::nullopt;
    }

    std::vector<CallSession> get_active_calls(const std::string& user_id) const {
        std::lock_guard<std::mutex> lock(mu_);
        std::vector<CallSession> out;
        for (const auto& [id, call] : calls_) {
            if ((call.caller_id == user_id || call.callee_id == user_id) &&
                (call.state == CallState::RINGING || call.state == CallState::ACTIVE)) {
                out.push_back(call);
            }
        }
        return out;
    }

    std::size_t active_call_count() const {
        std::lock_guard<std::mutex> lock(mu_);
        std::size_t count = 0;
        for (const auto& [id, call] : calls_) {
            if (call.state == CallState::RINGING || call.state == CallState::ACTIVE) {
                count++;
            }
        }
        return count;
    }
};

} // namespace garuda::beam
