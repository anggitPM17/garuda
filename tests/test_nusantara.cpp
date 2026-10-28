#include "../include/garuda/nusantara/nusantara_broker.hpp"
#include "../include/garuda/nusantara/entities/richat_penghubung.hpp"
#include "../include/garuda/nusantara/entities/gvm_penghubung.hpp"
#include <iostream>
#include <thread>
#include <chrono>

using namespace garuda::nusantara;
using namespace garuda::nusantara::entities;

int main() {
    std::cout << "=== MEMULAI EKOSISTEM GARUDA NUSANTARA ===\n\n";

    // 1. Inisialisasi Entitas
    RichatPenghubung richat;
    GvmPenghubung gvm;

    richat.initialize();
    gvm.initialize();

    // 2. Autopilot AI mengirim perbaikan ke RICHAT
    std::cout << "\n[SCENARIO] Autopilot AI mendeteksi bug di RICHAT UI...\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));

    NusantaraMessage ai_patch_msg;
    ai_patch_msg.sender = EntityTarget::AUTOPILOT_AI;
    ai_patch_msg.receiver = EntityTarget::RICHAT_APP;
    ai_patch_msg.action = NusantaraAction::PUSH_PATCH;
    ai_patch_msg.payload["file"] = "LoginActivity.kt";

    NusantaraBroker::get_instance().send_message(ai_patch_msg);

    // 3. Autopilot AI mengirim update logika ke GVM
    std::cout << "\n[SCENARIO] Autopilot AI mengirim update logika ke GVM...\n";
    NusantaraMessage ai_gvm_msg;
    ai_gvm_msg.sender = EntityTarget::AUTOPILOT_AI;
    ai_gvm_msg.receiver = EntityTarget::GARUDA_GVM;
    ai_gvm_msg.action = NusantaraAction::UPDATE_CODE;
    ai_gvm_msg.payload["function"] = "hitung_skor";

    NusantaraBroker::get_instance().send_message(ai_gvm_msg);

    // 4. Proses antrean pesan
    std::cout << "\n=== MEMPROSES ANTREAN GARUDA NUSANTARA ===\n";
    NusantaraBroker::get_instance().process_queue();

    std::cout << "\n=== EKOSISTEM GARUDA NUSANTARA BERJALAN SUKSES ===\n";
    return 0;
}
