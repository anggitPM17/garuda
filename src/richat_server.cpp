// =====================================================================
// RICHAT PRODUCTION SERVER
// Menjalankan RICHAT Gateway sebagai server WebSocket yang listen
// di port 9001, siap menerima koneksi dari Android/Kotlin.
// =====================================================================
#include "../include/garuda/richat/richat_gateway.hpp"
#include "../include/garuda/nusantara/nusantara_broker.hpp"
#include "../include/garuda/jalur_langit/jalur_langit_ultimate.hpp"

#include <iostream>
#include <thread>
#include <chrono>
#include <csignal>
#include <atomic>
#include <cstdint>

using namespace garuda::richat;
using namespace garuda::nusantara;
using namespace garuda::jalur_langit;

static std::atomic<bool> g_running{true};

void signal_handler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        std::cout << "\n[RICHAT SERVER] Menerima signal " << signal << ", mematikan...\n";
        g_running.store(false);
    }
}

int main(int argc, char** argv) {
    std::cout << "========================================\n";
    std::cout << "  RICHAT PRODUCTION SERVER\n";
    std::cout << "  Jalur Langit Database Engine v7.0\n";
    std::cout << "========================================\n\n";

    // 1. Parse port dari argumen (default 9001)
    std::uint16_t port = 9001;
    std::string data_dir = "./richat_production_data";
    if (argc > 1) {
        try {
            int p = std::stoi(argv[1]);
            if (p > 0 && p < 65536) port = static_cast<std::uint16_t>(p);
        } catch (...) {
            std::cerr << "[RICHAT SERVER] Port tidak valid, pakai default 9001\n";
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
        std::cout << "[RICHAT SERVER] Menginisialisasi database di: " << data_dir << "\n";
        JalurLangitUltimate db(data_dir);

        // 4. Inisialisasi Gateway
        std::cout << "[RICHAT SERVER] Menginisialisasi RICHAT Gateway...\n";
        RichatGateway gateway(db);

        // 5. Daftarkan Gateway sebagai penerima pesan JALUR_LANGIT
        NusantaraBroker::get_instance().register_entity(
            EntityTarget::JALUR_LANGIT,
            [&gateway](const NusantaraMessage& msg) {
                gateway.handle_message(msg);
            }
        );
        std::cout << "[RICHAT SERVER] Gateway terdaftar di Nusantara (entity JALUR_LANGIT)\n";

        // 6. Start Nusantara Worker (untuk proses antrean pesan)
        NusantaraBroker::get_instance().start();
        std::cout << "[RICHAT SERVER] Nusantara worker dimulai\n";

        // 7. Start WebSocket Listener di port 9001
        if (!NusantaraBroker::get_instance().start_ws_listener(port)) {
            std::cerr << "[RICHAT SERVER] GAGAL start WebSocket listener di port "
                      << port << "\n";
            std::cerr << "[RICHAT SERVER] Kemungkinan port sudah dipakai proses lain.\n";
            return 1;
        }

        std::cout << "\n";
        std::cout << "[RICHAT SERVER] ================================\n";
        std::cout << "[RICHAT SERVER] SERVER SIAP!\n";
        std::cout << "[RICHAT SERVER] ================================\n";
        std::cout << "[RICHAT SERVER] WebSocket: ws://0.0.0.0:" << port << "\n";
        std::cout << "[RICHAT SERVER] Emulator:  ws://10.0.2.2:" << port << "\n";
        std::cout << "[RICHAT SERVER] HP fisik:  ws://<IP-LAPTOP>:" << port << "\n";
        std::cout << "[RICHAT SERVER] Data dir:  " << data_dir << "\n";
        std::cout << "[RICHAT SERVER] ================================\n";
        std::cout << "[RICHAT SERVER] Tekan Ctrl+C untuk berhenti.\n\n";

        // 8. Loop utama (tunggu sampai signal)
        while (g_running.load()) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        // 9. Graceful shutdown
        std::cout << "\n[RICHAT SERVER] Mematikan WebSocket listener...\n";
        NusantaraBroker::get_instance().stop_ws_listener();

        std::cout << "[RICHAT SERVER] Menghentikan Nusantara worker...\n";
        NusantaraBroker::get_instance().stop();

        std::cout << "[RICHAT SERVER] Server berhenti dengan bersih.\n";
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "[RICHAT SERVER] FATAL ERROR: " << e.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "[RICHAT SERVER] FATAL ERROR: unknown exception\n";
        return 1;
    }
}
