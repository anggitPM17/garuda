#pragma once
// =====================================================================
// R1 - RICHAT GATEWAY (WebSocket Handler)
// Mengimplementasikan kontrak API R0 menggunakan Jalur Langit.
// =====================================================================
#include "richat_message_store.hpp"
#include "richat_metrics.hpp"
#include "richat_tracer.hpp"
#include "../nusantara/nusantara_broker.hpp"
#include <iostream>
#include <regex>
#include <chrono>

namespace garuda::richat {

using namespace garuda::nusantara;

class RichatGateway {
private:
    RichatMessageStore store_;
    RichatMetrics metrics_;
    RichatTracer tracer_;

    // Helper: Validasi session token
    bool validate_session(const std::string& session_token, std::string& out_user_id) {
        auto sess = store_.get_session(session_token);
        if (!sess.has_value()) return false;
        out_user_id = std::get<std::string>(sess->columns["user_id"]);
        return true;
    }

    // Helper: Kirim response error
    void send_error(EntityTarget receiver, int code, const std::string& message) {
        NusantaraMessage resp;
        resp.message_id = "resp_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
        resp.sender = EntityTarget::JALUR_LANGIT;
        resp.receiver = receiver;
        resp.action = NusantaraAction::BROADCAST_MSG;
        resp.payload["status"] = std::to_string(code);
        resp.payload["success"] = "false";
        resp.payload["message"] = message;
        NusantaraBroker::get_instance().send_message(resp);
    }

public:
    explicit RichatGateway(JalurLangitUltimate& db) : store_(db) {}

    std::string get_metrics() { return metrics_.render_prometheus(); }
    std::string get_traces() { return tracer_.export_json(); }

    void handle_message(const NusantaraMessage& msg) {
        auto span = tracer_.start_span(to_string(msg.action));
        auto start_time = std::chrono::steady_clock::now();
        bool success = true;

        try {
            // Validasi aksi
            if (msg.action == NusantaraAction::RICHAT_LOGIN) {
                success = handle_login(msg);
            } else if (msg.action == NusantaraAction::RICHAT_SEND_MESSAGE) {
                success = handle_send_message(msg);
            } else if (msg.action == NusantaraAction::RICHAT_GET_MESSAGES) {
                success = handle_get_messages(msg);
            } else if (msg.action == NusantaraAction::RICHAT_ACK_DELIVERED) {
                success = handle_ack_delivered(msg);
            } else if (msg.action == NusantaraAction::RICHAT_CREATE_ROOM) {
                success = handle_create_room(msg);
            } else if (msg.action == NusantaraAction::RICHAT_JOIN_ROOM) {
                success = handle_join_room(msg);
            } else if (msg.action == NusantaraAction::RICHAT_SEND_ROOM_MESSAGE) {
                success = handle_send_room_message(msg);
            } else if (msg.action == NusantaraAction::RICHAT_LEAVE_ROOM) {
                success = handle_leave_room(msg);
            } else {
                send_error(msg.sender, 403, "Action not allowed for RICHAT Gateway");
                success = false;
            }
        } catch (const std::exception& e) {
            send_error(msg.sender, 500, std::string("Internal Error: ") + e.what());
            success = false;
        }

        auto end_time = std::chrono::steady_clock::now();
        auto latency = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

        tracer_.end_span(span);
        metrics_.record_request(to_string(msg.action), latency, success);
    }

private:
    bool handle_login(const NusantaraMessage& msg) {
        auto auth_it = msg.payload.find("auth_token");
        auto dev_it = msg.payload.find("device_id");

        if (auth_it == msg.payload.end() || auth_it->second.empty() ||
            dev_it == msg.payload.end() || dev_it->second.empty()) {
            send_error(msg.sender, 400, "Missing auth_token or device_id");
            return false;
        }

        // Simulasi: anggap auth_token valid, buat user & session
        std::string user_id = "usr_" + auth_it->second.substr(0, 6);
        std::string session_token = "sess_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());

        store_.save_user(user_id, "user_" + user_id, auth_it->second, dev_it->second);
        store_.save_session(session_token, user_id);

        NusantaraMessage resp;
        resp.message_id = "resp_login";
        resp.sender = EntityTarget::JALUR_LANGIT;
        resp.receiver = msg.sender;
        resp.action = NusantaraAction::BROADCAST_MSG;
        resp.payload["status"] = "200";
        resp.payload["success"] = "true";
        resp.payload["user_id"] = user_id;
        resp.payload["session_token"] = session_token;
        NusantaraBroker::get_instance().send_message(resp);
        return true;
    }

    bool handle_send_message(const NusantaraMessage& msg) {
        auto sess_it = msg.payload.find("session_token");
        auto rec_it = msg.payload.find("receiver_id");
        auto cnt_it = msg.payload.find("content");
        auto cmsg_it = msg.payload.find("client_msg_id");

        if (sess_it == msg.payload.end() || rec_it == msg.payload.end() ||
            cnt_it == msg.payload.end() || cmsg_it == msg.payload.end()) {
            send_error(msg.sender, 400, "Missing required fields");
            return false;
        }

        std::string sender_id;
        if (!validate_session(sess_it->second, sender_id)) {
            send_error(msg.sender, 401, "Invalid or expired session_token");
            return false;
        }

        if (cnt_it->second.empty() || cnt_it->second.size() > 4096) {
            send_error(msg.sender, 413, "Content empty or too large (max 4096)");
            return false;
        }

        std::string message_id = store_.save_message(sender_id, rec_it->second, cnt_it->second, cmsg_it->second);

        NusantaraMessage resp;
        resp.message_id = "resp_send";
        resp.sender = EntityTarget::JALUR_LANGIT;
        resp.receiver = msg.sender;
        resp.action = NusantaraAction::BROADCAST_MSG;
        resp.payload["status"] = "200";
        resp.payload["success"] = "true";
        resp.payload["message_id"] = message_id;
        NusantaraBroker::get_instance().send_message(resp);
        return true;
    }

    bool handle_get_messages(const NusantaraMessage& msg) {
        auto sess_it = msg.payload.find("session_token");
        auto with_it = msg.payload.find("with_user_id");

        if (sess_it == msg.payload.end() || with_it == msg.payload.end()) {
            send_error(msg.sender, 400, "Missing session_token or with_user_id");
            return false;
        }

        std::string user_id;
        if (!validate_session(sess_it->second, user_id)) {
            send_error(msg.sender, 401, "Invalid session_token");
            return false;
        }

        auto messages = store_.get_messages(user_id, with_it->second);

        NusantaraMessage resp;
        resp.message_id = "resp_get";
        resp.sender = EntityTarget::JALUR_LANGIT;
        resp.receiver = msg.sender;
        resp.action = NusantaraAction::BROADCAST_MSG;
        resp.payload["status"] = "200";
        resp.payload["success"] = "true";
        resp.payload["count"] = std::to_string(messages.size());
        NusantaraBroker::get_instance().send_message(resp);
        return true;
    }

    bool handle_ack_delivered(const NusantaraMessage& msg) {
        auto sess_it = msg.payload.find("session_token");
        auto msg_it = msg.payload.find("message_id");

        if (sess_it == msg.payload.end() || msg_it == msg.payload.end()) {
            send_error(msg.sender, 400, "Missing session_token or message_id");
            return false;
        }

        std::string user_id;
        if (!validate_session(sess_it->second, user_id)) {
            send_error(msg.sender, 401, "Invalid session_token");
            return false;
        }

        store_.update_message_status(msg_it->second, "delivered");

        NusantaraMessage resp;
        resp.message_id = "resp_ack";
        resp.sender = EntityTarget::JALUR_LANGIT;
        resp.receiver = msg.sender;
        resp.action = NusantaraAction::BROADCAST_MSG;
        resp.payload["status"] = "200";
        resp.payload["success"] = "true";
        resp.payload["message"] = "Acknowledged";
        NusantaraBroker::get_instance().send_message(resp);
        return true;
    }

    bool handle_create_room(const NusantaraMessage& msg) {
        auto sess_it = msg.payload.find("session_token");
        auto name_it = msg.payload.find("room_name");

        if (sess_it == msg.payload.end() || name_it == msg.payload.end()) {
            send_error(msg.sender, 400, "Missing session_token or room_name");
            return false;
        }

        std::string user_id;
        if (!validate_session(sess_it->second, user_id)) {
            send_error(msg.sender, 401, "Invalid session_token");
            return false;
        }

        std::string room_id = store_.create_room(user_id, name_it->second);

        NusantaraMessage resp;
        resp.message_id = "resp_create_room";
        resp.sender = EntityTarget::JALUR_LANGIT;
        resp.receiver = msg.sender;
        resp.action = NusantaraAction::BROADCAST_MSG;
        resp.payload["status"] = "200";
        resp.payload["success"] = "true";
        resp.payload["room_id"] = room_id;
        NusantaraBroker::get_instance().send_message(resp);
        return true;
    }

    bool handle_join_room(const NusantaraMessage& msg) {
        auto sess_it = msg.payload.find("session_token");
        auto room_it = msg.payload.find("room_id");

        if (sess_it == msg.payload.end() || room_it == msg.payload.end()) {
            send_error(msg.sender, 400, "Missing session_token or room_id");
            return false;
        }

        std::string user_id;
        if (!validate_session(sess_it->second, user_id)) {
            send_error(msg.sender, 401, "Invalid session_token");
            return false;
        }

        store_.join_room(user_id, room_it->second);

        NusantaraMessage resp;
        resp.message_id = "resp_join_room";
        resp.sender = EntityTarget::JALUR_LANGIT;
        resp.receiver = msg.sender;
        resp.action = NusantaraAction::BROADCAST_MSG;
        resp.payload["status"] = "200";
        resp.payload["success"] = "true";
        resp.payload["message"] = "Joined room";
        NusantaraBroker::get_instance().send_message(resp);
        return true;
    }

    bool handle_leave_room(const NusantaraMessage& msg) {
        auto sess_it = msg.payload.find("session_token");
        auto room_it = msg.payload.find("room_id");

        if (sess_it == msg.payload.end() || room_it == msg.payload.end()) {
            send_error(msg.sender, 400, "Missing session_token or room_id");
            return false;
        }

        std::string user_id;
        if (!validate_session(sess_it->second, user_id)) {
            send_error(msg.sender, 401, "Invalid session_token");
            return false;
        }

        store_.leave_room(user_id, room_it->second);

        NusantaraMessage resp;
        resp.message_id = "resp_leave_room";
        resp.sender = EntityTarget::JALUR_LANGIT;
        resp.receiver = msg.sender;
        resp.action = NusantaraAction::BROADCAST_MSG;
        resp.payload["status"] = "200";
        resp.payload["success"] = "true";
        resp.payload["message"] = "Left room";
        NusantaraBroker::get_instance().send_message(resp);
        return true;
    }

    bool handle_send_room_message(const NusantaraMessage& msg) {
        auto sess_it = msg.payload.find("session_token");
        auto room_it = msg.payload.find("room_id");
        auto cnt_it = msg.payload.find("content");
        auto cmsg_it = msg.payload.find("client_msg_id");

        if (sess_it == msg.payload.end() || room_it == msg.payload.end() ||
            cnt_it == msg.payload.end() || cmsg_it == msg.payload.end()) {
            send_error(msg.sender, 400, "Missing required fields");
            return false;
        }

        std::string sender_id;
        if (!validate_session(sess_it->second, sender_id)) {
            send_error(msg.sender, 401, "Invalid session_token");
            return false;
        }

        if (cnt_it->second.empty() || cnt_it->second.size() > 4096) {
            send_error(msg.sender, 413, "Content empty or too large");
            return false;
        }

        std::string message_id = store_.save_room_message(sender_id, room_it->second, cnt_it->second, cmsg_it->second);

        NusantaraMessage resp;
        resp.message_id = "resp_room_msg";
        resp.sender = EntityTarget::JALUR_LANGIT;
        resp.receiver = msg.sender;
        resp.action = NusantaraAction::BROADCAST_MSG;
        resp.payload["status"] = "200";
        resp.payload["success"] = "true";
        resp.payload["message_id"] = message_id;
        NusantaraBroker::get_instance().send_message(resp);
        return true;
    }
};

} // namespace garuda::richat
