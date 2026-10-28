#pragma once
#include "nusantara_protocol.hpp"
#include <atomic>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>

namespace garuda::nusantara {

class NusantaraBroker {
public:
    using EntityReceiver = std::function<void(const NusantaraMessage&)>;

private:
    std::map<EntityTarget, EntityReceiver> receivers_;
    std::queue<NusantaraMessage> message_queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::atomic<bool> running_{false};
    std::thread worker_;

    NusantaraBroker() = default; // Singleton

    void worker_loop() {
        while (true) {
            NusantaraMessage msg;
            EntityReceiver receiver = nullptr;
            bool has_msg = false;

            {
                std::unique_lock<std::mutex> lock(mutex_);
                cv_.wait(lock, [this]() { return !running_.load() || !message_queue_.empty(); });

                if (!running_.load() && message_queue_.empty()) {
                    break;
                }

                if (!message_queue_.empty()) {
                    msg = message_queue_.front();
                    message_queue_.pop();
                    has_msg = true;

                    auto it = receivers_.find(msg.receiver);
                    if (it != receivers_.end()) {
                        receiver = it->second;
                    }
                }
            }

            if (has_msg) {
                if (receiver) {
                    try {
                        receiver(msg);
                    } catch (const std::exception& e) {
                        std::cerr << "[GARUDA NUSANTARA ERROR] Gagal memproses pesan: " << e.what() << "\n";
                    }
                } else {
                    std::cerr << "[GARUDA NUSANTARA] Peringatan: Penerima tidak ditemukan!\n";
                }
            }
        }
    }

public:
    static NusantaraBroker& get_instance() {
        static NusantaraBroker instance;
        return instance;
    }

    ~NusantaraBroker() {
        stop();
    }

    void start() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (running_.load()) return;
        running_.store(true);
        worker_ = std::thread([this]() { this->worker_loop(); });
        std::cout << "[GARUDA NUSANTARA] Async Worker Broker dimulai.\n";
    }

    void stop() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!running_.load()) return;
            running_.store(false);
        }
        cv_.notify_all();
        if (worker_.joinable()) {
            worker_.join();
        }
        std::cout << "[GARUDA NUSANTARA] Async Worker Broker dihentikan.\n";
    }

    // Mendaftarkan entitas (misal: Autopilot, Richat, GVM)
    void register_entity(EntityTarget target, EntityReceiver receiver) {
        std::lock_guard<std::mutex> lock(mutex_);
        receivers_[target] = receiver;
        std::cout << "[GARUDA NUSANTARA] Entitas terdaftar: " << (int)target << "\n";
    }

    // Mengirim pesan (Bisa dari mana saja)
    void send_message(const NusantaraMessage& msg) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            message_queue_.push(msg);
        }
        cv_.notify_one();
        std::cout << "[GARUDA NUSANTARA] Pesan masuk: Action=" << (int)msg.action
                  << ", Dari=" << (int)msg.sender
                  << ", Ke=" << (int)msg.receiver << "\n";
    }

    // Memproses antrean pesan secara manual (Sync / Deterministic)
    void process_queue() {
        while (true) {
            NusantaraMessage msg;
            EntityReceiver receiver = nullptr;
            bool has_msg = false;

            {
                std::lock_guard<std::mutex> lock(mutex_);
                if (message_queue_.empty()) break;
                msg = message_queue_.front();
                message_queue_.pop();
                has_msg = true;

                auto it = receivers_.find(msg.receiver);
                if (it != receivers_.end()) {
                    receiver = it->second;
                }
            }

            if (has_msg) {
                if (receiver) {
                    try {
                        receiver(msg);
                    } catch (const std::exception& e) {
                        std::cerr << "[GARUDA NUSANTARA ERROR] Gagal memproses pesan: " << e.what() << "\n";
                    }
                } else {
                    std::cerr << "[GARUDA NUSANTARA] Peringatan: Penerima tidak ditemukan!\n";
                }
            }
        }
    }
};

} // namespace garuda::nusantara
