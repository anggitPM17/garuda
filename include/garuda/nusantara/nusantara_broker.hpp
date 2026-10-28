#pragma once
#include "nusantara_protocol.hpp"
#include "nusantara_transport.hpp"
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

    std::thread listener_thread_;
    std::atomic<bool> listener_running_{false};
    transport::Listener listener_;

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

    void listener_loop() {
        while (listener_running_.load()) {
            auto client = listener_.accept();
            if (!client.valid()) {
                if (!listener_running_.load()) break;
                continue;
            }

            std::thread([this, client_sock = std::move(client)]() mutable {
                this->handle_client(std::move(client_sock));
            }).detach();
        }
    }

    void handle_client(transport::Socket client) {
        while (true) {
            auto line = client.recv_until('\n');
            if (!line.has_value()) break;
            if (line->empty()) continue;

            auto msg = NusantaraMessage::from_json(*line);
            if (msg.has_value()) {
                send_message(*msg);
            } else {
                std::cerr << "[GARUDA NUSANTARA WARNING] Formulasi JSON TCP tidak valid: " << *line << "\n";
            }
        }
    }

public:
    static NusantaraBroker& get_instance() {
        static NusantaraBroker instance;
        return instance;
    }

    ~NusantaraBroker() {
        stop_listener();
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

    bool start_listener(std::uint16_t port) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (listener_running_.load()) return false;
        if (!listener_.listen("0.0.0.0", port)) return false;

        listener_running_.store(true);
        listener_thread_ = std::thread([this]() { this->listener_loop(); });
        std::cout << "[GARUDA NUSANTARA] Listener TCP terpasang pada port: " << listener_.port() << "\n";
        return true;
    }

    std::uint16_t listener_port() const {
        return listener_.port();
    }

    bool listener_active() const {
        return listener_running_.load();
    }

    void stop_listener() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!listener_running_.load()) return;
            listener_running_.store(false);
            listener_.close();
        }
        if (listener_thread_.joinable()) {
            listener_thread_.join();
        }
        std::cout << "[GARUDA NUSANTARA] Listener TCP dihentikan.\n";
    }

    bool send_remote(const std::string& host, std::uint16_t port, const NusantaraMessage& msg) {
        auto sock = transport::connect_to(host, port);
        if (!sock.has_value()) return false;
        std::string payload = msg.to_json() + "\n";
        return sock->send_all(payload);
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
