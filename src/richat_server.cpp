// =====================================================================
// RICHAT PRODUCTION SERVER
// Menjalankan RICHAT Gateway sebagai server WebSocket yang listen
// di port 9001/9002, siap menerima koneksi dari Android/Kotlin.
// Structured JSON Logging for journalctl, Filebeat & Loki
// =====================================================================
#include <iostream>
#include <thread>
#include <chrono>
#include <csignal>
#include <atomic>
#include <cstdint>

#include "../include/garuda/sys/json_logger.hpp"
#include "../include/garuda/nusantara/nusantara_protocol.hpp"
#include "../include/garuda/richat/richat_gateway.hpp"
#include "../include/garuda/nusantara/nusantara_broker.hpp"
#include "../include/garuda/jalur_langit/jalur_langit_ultimate.hpp"
#include "../include/garuda/game/arena.hpp"

using namespace garuda::richat;
using namespace garuda::nusantara;
using namespace garuda::jalur_langit;
using garuda::sys::JsonLogger;

static std::atomic<bool> g_running{true};

void signal_handler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        JsonLogger::info("Signal " + std::to_string(signal) + " diterima, mematikan server...");
        g_running.store(false);
    }
}

int main(int argc, char** argv) {
    JsonLogger::info("RICHAT Production Server GVM Engine Starting...");

    // 1. Parse port dari argumen (default 9001)
    std::uint16_t port = 9001;
    std::string data_dir = "./richat_production_data";
    if (argc > 1) {
        try {
            int p = std::stoi(argv[1]);
            if (p > 0 && p < 65536) port = static_cast<std::uint16_t>(p);
        } catch (...) {
            JsonLogger::warn("Port tidak valid, menggunakan default 9001");
        }
    }
    if (argc > 2) {
        data_dir = argv[2];
    }

    // 2. Daftarkan signal handler untuk graceful shutdown
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    try {
        // 3. Inisialisasi Database (Jalur Langit)
        JsonLogger::info("Menginisialisasi database Jalur Langit di: " + data_dir);
        JalurLangitUltimate db(data_dir);

        // 4. Inisialisasi Gateway
        JsonLogger::info("Menginisialisasi RICHAT Gateway...");
        RichatGateway gateway(db);

        // 5. Daftarkan Gateway sebagai penerima pesan JALUR_LANGIT
        NusantaraBroker::get_instance().register_entity(
            EntityTarget::JALUR_LANGIT,
            [&gateway](const NusantaraMessage& msg) {
                gateway.handle_message(msg);
            }
        );
        JsonLogger::info("Gateway terdaftar di Nusantara Broker (entity JALUR_LANGIT)");

        // 6. Start Nusantara Worker
        NusantaraBroker::get_instance().start();
        JsonLogger::info("Nusantara worker async broker dimulai");

        // 7. Start WebSocket Listener
        if (!NusantaraBroker::get_instance().start_ws_listener(port)) {
            JsonLogger::error("GAGAL start WebSocket listener di port " + std::to_string(port) + ". Port mungkin sudah dipakai.");
            return 1;
        }
        JsonLogger::info("WebSocket Listener aktif di port " + std::to_string(port));

        // 7.5. Start Garuda Arena Game Tick Loop (30 FPS)
        std::atomic<bool> game_running{true};
        std::thread game_thread([&game_running]() {
            JsonLogger::info("Garuda Arena Game Engine Tick Loop Dimulai (30 FPS)");
            while (game_running.load()) {
                auto start = std::chrono::steady_clock::now();

                garuda::game::ArenaEngine::get_instance().tick();
                std::string state_json = garuda::game::ArenaEngine::get_instance().get_state_json();
                NusantaraBroker::get_instance().broadcast_ws_text(state_json);

                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - start
                ).count();

                if (elapsed < 33) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(33 - elapsed));
                }
            }
            JsonLogger::info("Garuda Arena Game Engine Tick Loop Dihentikan");
        });

        JsonLogger::info("SERVER SIAP! Listening ws://0.0.0.0:" + std::to_string(port));

        // 8. Loop utama
        while (g_running.load()) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        // 9. Graceful shutdown
        JsonLogger::info("Mematikan Game Loop...");
        game_running.store(false);
        if (game_thread.joinable()) game_thread.join();

        JsonLogger::info("Mematikan WebSocket listener...");
        NusantaraBroker::get_instance().stop_ws_listener();

        JsonLogger::info("Menghentikan Nusantara worker...");
        NusantaraBroker::get_instance().stop();

        JsonLogger::info("Server GVM berhenti dengan bersih.");
        return 0;

    } catch (const std::exception& e) {
        JsonLogger::error(std::string("FATAL ERROR: ") + e.what());
        return 1;
    } catch (...) {
        JsonLogger::error("FATAL ERROR: unknown exception");
        return 1;
    }
}
