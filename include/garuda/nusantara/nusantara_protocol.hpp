#pragma once
#include <string>
#include <vector>
#include <map>

namespace garuda::nusantara {

// Entitas yang terhubung dalam ekosistem Garuda Nusantara
enum class EntityTarget {
    RICHAT_APP,
    GARUDA_GVM,
    STADION_UI,
    ANDROID_STUDIO,
    AUTOPILOT_AI
};

// Aksi yang bisa dilakukan antar entitas
enum class NusantaraAction {
    UPDATE_CODE,      // Perintah update kode / hot-reload
    RELOAD_ASSETS,    // Perintah memuat ulang aset UI
    RUN_TEST,         // Perintah menjalankan pengujian
    GET_METRICS,      // Permintaan data telemetri / kesehatan
    PUSH_PATCH,       // Mengirim patch kode hasil perbaikan AI
    BROADCAST_MSG     // Pesan teks biasa
};

struct NusantaraMessage {
    std::string message_id;
    EntityTarget sender;
    EntityTarget receiver;
    NusantaraAction action;
    std::map<std::string, std::string> payload;

    std::string to_json() const {
        // Implementasi serialisasi JSON diletakkan di sini
        return "{}";
    }
};

} // namespace garuda::nusantara
