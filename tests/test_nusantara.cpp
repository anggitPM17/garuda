#include "../include/garuda/nusantara/nusantara_broker.hpp"
#include "../include/garuda/nusantara/entities/richat_penghubung.hpp"
#include "../include/garuda/nusantara/entities/gvm_penghubung.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>

using namespace garuda::nusantara;
using namespace garuda::nusantara::entities;

#define GARUDA_CHECK(cond, msg) \
    if (!(cond)) { \
        std::cerr << "  ❌ ASSERTION FAILED: " << msg << " (" << #cond << ")\n"; \
        std::exit(1); \
    }

int main() {
    std::cout << "=== MEMULAI EKOSISTEM GARUDA NUSANTARA ===\n\n";

    // 1. Inisialisasi Entitas
    RichatPenghubung richat;
    GvmPenghubung gvm;

    richat.initialize();
    gvm.initialize();

    // 2. Autopilot AI mengirim perbaikan ke RICHAT
    std::cout << "\n[SCENARIO] Autopilot AI mendeteksi bug di RICHAT UI...\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    NusantaraMessage ai_patch_msg;
    ai_patch_msg.message_id = "patch-001";
    ai_patch_msg.sender = EntityTarget::AUTOPILOT_AI;
    ai_patch_msg.receiver = EntityTarget::RICHAT_APP;
    ai_patch_msg.action = NusantaraAction::PUSH_PATCH;
    ai_patch_msg.payload["file"] = "LoginActivity.kt";

    NusantaraBroker::get_instance().send_message(ai_patch_msg);

    // 3. Autopilot AI mengirim update logika ke GVM
    std::cout << "\n[SCENARIO] Autopilot AI mengirim update logika ke GVM...\n";
    NusantaraMessage ai_gvm_msg;
    ai_gvm_msg.message_id = "gvm-001";
    ai_gvm_msg.sender = EntityTarget::AUTOPILOT_AI;
    ai_gvm_msg.receiver = EntityTarget::GARUDA_GVM;
    ai_gvm_msg.action = NusantaraAction::UPDATE_CODE;
    ai_gvm_msg.payload["function"] = "hitung_skor";

    NusantaraBroker::get_instance().send_message(ai_gvm_msg);

    // 4. Proses antrean pesan (Sync)
    std::cout << "\n=== MEMPROSES ANTREAN GARUDA NUSANTARA (SYNC) ===\n";
    NusantaraBroker::get_instance().process_queue();

    // === Test A: JSON Roundtrip ===
    std::cout << "\n=== TEST A: JSON ROUNDTRIP ===\n";
    {
        NusantaraMessage msg;
        msg.message_id = "msg-001";
        msg.sender = EntityTarget::AUTOPILOT_AI;
        msg.receiver = EntityTarget::RICHAT_APP;
        msg.action = NusantaraAction::PUSH_PATCH;
        msg.payload["file"] = "LoginActivity.kt";
        msg.payload["patch"] = "quote\" backslash\\ newline\n tab\t";

        std::string json = msg.to_json();
        std::cout << "JSON Output: " << json << "\n";

        auto parsed = NusantaraMessage::from_json(json);
        GARUDA_CHECK(parsed.has_value(), "from_json must parse to_json output");
        GARUDA_CHECK(parsed->message_id == msg.message_id, "message_id roundtrip");
        GARUDA_CHECK(parsed->sender == msg.sender, "sender roundtrip");
        GARUDA_CHECK(parsed->receiver == msg.receiver, "receiver roundtrip");
        GARUDA_CHECK(parsed->action == msg.action, "action roundtrip");
        GARUDA_CHECK(parsed->payload == msg.payload, "payload roundtrip (with escapes)");
        std::cout << "  ✓ Test A: JSON Roundtrip PASS\n";
    }

    // === Test B: JSON Parsing Error Handling ===
    std::cout << "\n=== TEST B: JSON ERROR HANDLING ===\n";
    {
        GARUDA_CHECK(!NusantaraMessage::from_json("{invalid").has_value(), "malformed JSON must return nullopt");
        GARUDA_CHECK(!NusantaraMessage::from_json("").has_value(), "empty JSON must return nullopt");
        GARUDA_CHECK(!NusantaraMessage::from_json("{\"unknown\": 1}").has_value(), "missing required fields must return nullopt");
        std::cout << "  ✓ Test B: JSON Error Handling PASS\n";
    }

    // === Test C: Async Broker ===
    std::cout << "\n=== TEST C: ASYNC BROKER WORKER ===\n";
    {
        auto& broker = NusantaraBroker::get_instance();
        std::atomic<int> received{0};
        broker.register_entity(EntityTarget::STADION_UI, [&](const NusantaraMessage&) {
            received.fetch_add(1);
        });

        broker.start();

        for (int i = 0; i < 10; ++i) {
            NusantaraMessage m;
            m.message_id = "async-" + std::to_string(i);
            m.sender = EntityTarget::AUTOPILOT_AI;
            m.receiver = EntityTarget::STADION_UI;
            m.action = NusantaraAction::BROADCAST_MSG;
            broker.send_message(m);
        }

        auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
        while (received.load() < 10 && std::chrono::steady_clock::now() < deadline) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        GARUDA_CHECK(received.load() == 10, "worker must process all 10 messages");
        broker.stop();
        std::cout << "  ✓ Test C: Async Broker PASS (" << received.load() << "/10 messages received)\n";
    }

    // === Test D: stop() Idempotency & Restart ===
    std::cout << "\n=== TEST D: STOP IDEMPOTENCY & RESTART ===\n";
    {
        auto& broker = NusantaraBroker::get_instance();
        broker.stop();
        broker.stop(); // aman dipanggil dua kali
        broker.start();
        broker.stop();
        std::cout << "  ✓ Test D: stop() Idempotency & Restart PASS\n";
    }

    // === Test E: TCP Transport ===
    std::cout << "\n=== TEST E: TCP TRANSPORT ===\n";
    {
        auto& broker = NusantaraBroker::get_instance();

        std::atomic<int> received{0};
        std::atomic<bool> payload_ok{false};
        broker.register_entity(EntityTarget::STADION_UI, [&](const NusantaraMessage& m) {
            if (m.payload.count("text") && m.payload.at("text") == "hello via tcp") {
                payload_ok.store(true);
            }
            received.fetch_add(1);
        });

        const std::uint16_t port = 47823;
        GARUDA_CHECK(broker.start_listener(port), "start_listener must succeed");
        broker.start();

        NusantaraMessage msg;
        msg.message_id = "tcp-001";
        msg.sender = EntityTarget::AUTOPILOT_AI;
        msg.receiver = EntityTarget::STADION_UI;
        msg.action = NusantaraAction::BROADCAST_MSG;
        msg.payload["text"] = "hello via tcp";

        GARUDA_CHECK(broker.send_remote("127.0.0.1", port, msg), "send_remote must succeed");

        auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
        while (received.load() < 1 && std::chrono::steady_clock::now() < deadline) {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }

        GARUDA_CHECK(received.load() == 1, "listener must receive 1 message via TCP");
        GARUDA_CHECK(payload_ok.load(), "payload must be preserved across TCP");

        broker.stop();
        broker.stop_listener();
        std::cout << "  ✓ Test E: TCP Transport PASS\n";
    }

    // === Test F: Malformed JSON over TCP ===
    std::cout << "\n=== TEST F: MALFORMED JSON OVER TCP ===\n";
    {
        auto& broker = NusantaraBroker::get_instance();
        const std::uint16_t port = 47824;
        GARUDA_CHECK(broker.start_listener(port), "start_listener must succeed");
        broker.start();

        auto sock = transport::connect_to("127.0.0.1", port);
        GARUDA_CHECK(sock.has_value(), "connect_to must succeed");
        GARUDA_CHECK(sock->send_all("{not valid json\n"), "send_all must succeed");
        sock->close();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        std::atomic<int> received{0};
        broker.register_entity(EntityTarget::ANDROID_STUDIO, [&](const NusantaraMessage&) {
            received.fetch_add(1);
        });

        NusantaraMessage msg;
        msg.message_id = "tcp-002";
        msg.sender = EntityTarget::AUTOPILOT_AI;
        msg.receiver = EntityTarget::ANDROID_STUDIO;
        msg.action = NusantaraAction::BROADCAST_MSG;
        GARUDA_CHECK(broker.send_remote("127.0.0.1", port, msg), "send_remote must succeed after malformed input");

        auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
        while (received.load() < 1 && std::chrono::steady_clock::now() < deadline) {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
        GARUDA_CHECK(received.load() == 1, "broker must survive malformed JSON and process subsequent message");

        broker.stop();
        broker.stop_listener();
        std::cout << "  ✓ Test F: Malformed JSON over TCP PASS\n";
    }

    // === Test G: stop_listener() Idempotency ===
    std::cout << "\n=== TEST G: STOP LISTENER IDEMPOTENCY ===\n";
    {
        auto& broker = NusantaraBroker::get_instance();
        broker.stop_listener();
        broker.stop_listener();
        std::cout << "  ✓ Test G: stop_listener() Idempotency PASS\n";
    }

    std::cout << "\n=== EKOSISTEM GARUDA NUSANTARA FASE C BERJALAN SUKSES ===\n";
    return 0;
}
