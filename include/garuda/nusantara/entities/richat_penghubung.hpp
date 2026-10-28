#pragma once
#include "../nusantara_broker.hpp"

namespace garuda::nusantara::entities {

class RichatPenghubung {
public:
    void initialize() {
        NusantaraBroker::get_instance().register_entity(EntityTarget::RICHAT_APP,
            [this](const NusantaraMessage& msg) { this->handle_message(msg); });
    }

private:
    void handle_message(const NusantaraMessage& msg) {
        std::cout << "[RICHAT PENGHUBUNG] Menerima pesan dari Nusantara. Action: " << (int)msg.action << "\n";

        if (msg.action == NusantaraAction::PUSH_PATCH) {
            std::cout << "[RICHAT PENGHUBUNG] Mengirim perintah reload ke Android Studio (Gradle)...\n";
            // Logika: Kirim HTTP request ke localhost Gradle/Android Studio
        }
    }
};

} // namespace garuda::nusantara::entities
