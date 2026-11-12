#pragma once
// =====================================================================
// FASE G5.1 - WebRTC Signaling: Call offer/answer, ICE, reject, end
// Optimized with Sharded Lock-Striping Concurrent Hash Map for 10M+ Scale
// =====================================================================
#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <optional>
#include <chrono>
#include <cstddef>
#include <atomic>
#include <functional>

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
    static constexpr std::size_t NUM_BUCKETS = 64;

    struct Bucket {
        std::unordered_map<std::string, CallSession> calls;
        mutable std::mutex mu;
    };

    Bucket buckets_[NUM_BUCKETS];
    std::atomic<std::size_t> active_count_{0};
    std::atomic<std::uint64_t> id_counter_{0};

    std::size_t get_bucket_index(const std::string& key) const {
        return std::hash<std::string>{}(key) % NUM_BUCKETS;
    }

public:
    WebRTCSignaling() = default;

    std::optional<CallSession> call_offer(const std::string& caller_id,
                                          const std::string& callee_id,
                                          bool video = false) {
        if (caller_id.empty() || callee_id.empty()) return std::nullopt;

        std::uint64_t seq = ++id_counter_;
        std::string call_id = "call_" + std::to_string(seq) + "_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());

        CallSession session;
        session.call_id = call_id;
        session.caller_id = caller_id;
        session.callee_id = callee_id;
        session.state = CallState::RINGING;
        session.sdp_offer = "v=0\r\no=- " + call_id + " IN IP4 127.0.0.1\r\ns=GarudaCall\r\n";
        session.started_at = std::chrono::steady_clock::now();
        session.video_enabled = video;

        std::size_t b = get_bucket_index(call_id);
        {
            std::lock_guard<std::mutex> lock(buckets_[b].mu);
            buckets_[b].calls[call_id] = session;
        }

        active_count_.fetch_add(1, std::memory_order_relaxed);
        return session;
    }

    bool call_answer(const std::string& call_id,
                     const std::string& callee_id,
                     const std::string& sdp_answer) {
        std::size_t b = get_bucket_index(call_id);
        std::lock_guard<std::mutex> lock(buckets_[b].mu);
        auto it = buckets_[b].calls.find(call_id);
        if (it == buckets_[b].calls.end()) return false;

        auto& call = it->second;
        if (call.callee_id != callee_id) return false;

        call.sdp_answer = sdp_answer;
        call.state = CallState::ACTIVE;
        return true;
    }

    bool call_reject(const std::string& call_id, const std::string& user_id) {
        std::size_t b = get_bucket_index(call_id);
        std::lock_guard<std::mutex> lock(buckets_[b].mu);
        auto it = buckets_[b].calls.find(call_id);
        if (it == buckets_[b].calls.end()) return false;

        auto& call = it->second;
        if (call.caller_id != user_id && call.callee_id != user_id) return false;

        if (call.state == CallState::RINGING || call.state == CallState::ACTIVE) {
            active_count_.fetch_sub(1, std::memory_order_relaxed);
        }
        call.state = CallState::REJECTED;
        return true;
    }

    bool call_end(const std::string& call_id, const std::string& user_id) {
        std::size_t b = get_bucket_index(call_id);
        std::lock_guard<std::mutex> lock(buckets_[b].mu);
        auto it = buckets_[b].calls.find(call_id);
        if (it == buckets_[b].calls.end()) return false;

        auto& call = it->second;
        if (call.caller_id != user_id && call.callee_id != user_id) return false;

        if (call.state == CallState::RINGING || call.state == CallState::ACTIVE) {
            active_count_.fetch_sub(1, std::memory_order_relaxed);
        }
        call.state = CallState::ENDED;
        return true;
    }

    bool add_ice_candidate(const std::string& call_id, const std::string& candidate) {
        std::size_t b = get_bucket_index(call_id);
        std::lock_guard<std::mutex> lock(buckets_[b].mu);
        auto it = buckets_[b].calls.find(call_id);
        if (it == buckets_[b].calls.end()) return false;

        it->second.ice_candidates.push_back(candidate);
        return true;
    }

    std::optional<CallSession> get_call(const std::string& call_id) const {
        std::size_t b = get_bucket_index(call_id);
        std::lock_guard<std::mutex> lock(buckets_[b].mu);
        auto it = buckets_[b].calls.find(call_id);
        if (it != buckets_[b].calls.end()) return it->second;
        return std::nullopt;
    }

    std::vector<CallSession> get_active_calls(const std::string& user_id) const {
        std::vector<CallSession> out;
        for (std::size_t b = 0; b < NUM_BUCKETS; ++b) {
            std::lock_guard<std::mutex> lock(buckets_[b].mu);
            for (const auto& [id, call] : buckets_[b].calls) {
                if ((call.caller_id == user_id || call.callee_id == user_id) &&
                    (call.state == CallState::RINGING || call.state == CallState::ACTIVE)) {
                    out.push_back(call);
                }
            }
        }
        return out;
    }

    std::size_t active_call_count() const {
        return active_count_.load(std::memory_order_relaxed);
    }
};

} // namespace garuda::beam
