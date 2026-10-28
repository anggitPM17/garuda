#pragma once
#include "nusantara_protocol.hpp"
#include <mutex>
#include <queue>
#include <functional>
#include <iostream>
#include <memory>
#include <map>

namespace garuda::nusantara {

class NusantaraBroker {
public:
    using EntityReceiver = std::function<void(const NusantaraMessage&)>;

private:
    std::map<EntityTarget, EntityReceiver> receivers_;
    std::queue<NusantaraMessage> message_queue_;
    std::mutex mutex_;

    NusantaraBroker() = default; // Singleton

public:
    static NusantaraBroker& get_instance() {
        static NusantaraBroker instance;
        return instance;
    }

    // Mendaftarkan entitas (misal: Autopilot, Richat, GVM)
    void register_entity(EntityTarget target, EntityReceiver receiver) {
        std::lock_guard<std::mutex> lock(mutex_);
        receivers_[target] = receiver;
        std::cout << "[GARUDA NUSANTARA] Entitas terdaftar: " << (int)target << "\n";
    }

    // Mengirim pesan (Bisa dari mana saja)
    void send_message(const NusantaraMessage& msg) {
        std::lock_guard<std::mutex> lock(mutex_);
        message_queue_.push(msg);
        std::cout << "[GARUDA NUSANTARA] Pesan masuk: Action=" << (int)msg.action
                  << ", Dari=" << (int)msg.sender
                  << ", Ke=" << (int)msg.receiver << "\n";
    }

    // Memproses antrean pesan
    void process_queue() {
        std::lock_guard<std::mutex> lock(mutex_);
        while (!message_queue_.empty()) {
            NusantaraMessage msg = message_queue_.front();
            message_queue_.pop();

            auto it = receivers_.find(msg.receiver);
            if (it != receivers_.end()) {
                try {
                    it->second(msg);
                } catch (const std::exception& e) {
                    std::cerr << "[GARUDA NUSANTARA ERROR] Gagal memproses pesan: " << e.what() << "\n";
                }
            } else {
                std::cerr << "[GARUDA NUSANTARA] Peringatan: Penerima tidak ditemukan!\n";
            }
        }
    }
};

} // namespace garuda::nusantara
