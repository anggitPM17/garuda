#pragma once
// Garuda GVM: Cowboy-Grade WebSocket Handler untuk RICHAT
// Menangani frame WebSocket real-time: chat_msg, contact_invite, contact_accept, webrtc_signal
#include "cowboy_router.hpp"
#include "../core/runtime.hpp"
#include "../crypto/crypto.hpp"

#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace garuda::beam {

struct WsClientSession {
    std::string user_id;
    std::string device_uuid;
    Pid process_pid;
    socket_t socket_fd;
    std::uint64_t connected_at = 0;
};

class RichatWsHandler {
public:
    explicit RichatWsHandler(Runtime& rt) : rt_(rt) {}

    void register_session(const std::string& user_id, const std::string& device_uuid, Pid pid, socket_t fd) {
        std::lock_guard<std::mutex> lock(mu_);
        WsClientSession sess;
        sess.user_id = user_id;
        sess.device_uuid = device_uuid;
        sess.process_pid = pid;
        sess.socket_fd = fd;
        sess.connected_at = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();

        active_sessions_[user_id] = sess;
        std::cout << "💬 [GARUDA WS RICHAT] User connected: " << user_id << " (PID: " << to_string(Term(pid)) << ")" << std::endl;
    }

    void unregister_session(const std::string& user_id) {
        std::lock_guard<std::mutex> lock(mu_);
        active_sessions_.erase(user_id);
        std::cout << "👋 [GARUDA WS RICHAT] User disconnected: " << user_id << std::endl;
    }

    bool send_to_user(const std::string& target_user_id, const std::string& json_frame) {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = active_sessions_.find(target_user_id);
        if (it != active_sessions_.end()) {
            rt_.send(Pid{}, it->second.process_pid, tuple({atom("ws_send"), Term(json_frame)}));
            return true;
        }
        return false; // User offline, store in Mnesia offline_message
    }

    std::size_t online_count() const {
        std::lock_guard<std::mutex> lock(mu_);
        return active_sessions_.size();
    }

    // Parsing Frame WebSocket RICHAT (chat_msg, contact_invite, webrtc_signal, etc)
    std::optional<Term> parse_richat_frame(const std::string& raw_json) {
        if (raw_json.find("\"type\":\"chat_msg\"") != std::string::npos) {
            return tuple({atom("chat_msg"), Term(raw_json)});
        }
        if (raw_json.find("\"type\":\"contact_invite\"") != std::string::npos) {
            return tuple({atom("contact_invite"), Term(raw_json)});
        }
        if (raw_json.find("\"type\":\"webrtc_signal\"") != std::string::npos) {
            return tuple({atom("webrtc_signal"), Term(raw_json)});
        }
        return tuple({atom("unknown"), Term(raw_json)});
    }

private:
    Runtime& rt_;
    mutable std::mutex mu_;
    std::map<std::string, WsClientSession> active_sessions_;
};

} // namespace garuda::beam
