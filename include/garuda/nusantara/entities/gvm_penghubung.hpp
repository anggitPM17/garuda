#pragma once
#include "../nusantara_broker.hpp"

namespace garuda::nusantara::entities {

class GvmPenghubung {
public:
    void initialize() {
        NusantaraBroker::get_instance().register_entity(EntityTarget::GARUDA_GVM,
            [this](const NusantaraMessage& msg) { this->handle_message(msg); });
    }

private:
    void handle_message(const NusantaraMessage& msg) {
        std::cout << "[GVM PENGHUBUNG] Menerima perintah dari Nusantara.\n";
        if (msg.action == NusantaraAction::UPDATE_CODE) {
            std::cout << "[GVM PENGHUBUNG] Melakukan Hot-Reload Bytecode di runtime...\n";
            // Panggil API Runtime GVM untuk hot-reload
        }
    }
};

} // namespace garuda::nusantara::entities
