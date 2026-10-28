#pragma once
#include "nusantara_protocol.hpp"
#include "nusantara_transport.hpp"
#include "nusantara_store.hpp"
#include <atomic>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_map>

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

    // Plaintext TCP Listener
    std::thread listener_thread_;
    std::atomic<bool> listener_running_{false};
    transport::Listener listener_;

    // TLS TCP Listener
    std::thread tls_listener_thread_;
    std::atomic<bool> tls_listener_running_{false};
    transport::TlsListener tls_listener_;

    // Plaintext WebSocket Listener
    std::thread ws_listener_thread_;
    std::atomic<bool> ws_listener_running_{false};
    transport::WsListener ws_listener_;

    // WSS Listener
    std::thread wss_listener_thread_;
    std::atomic<bool> wss_listener_running_{false};
    transport::WssListener wss_listener_;

    // Persistensi SQLite WAL
    std::unique_ptr<NusantaraMessageStore> store_;
    mutable std::mutex store_mutex_;
    std::atomic<bool> persistence_on_{false};
    std::unordered_map<std::string, std::int64_t> pending_row_ids_;

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

                if (persistence_on_.load()) {
                    std::lock_guard<std::mutex> store_lock(store_mutex_);
                    auto it = pending_row_ids_.find(msg.message_id);
                    if (it != pending_row_ids_.end()) {
                        if (store_ && store_->is_open()) {
                            store_->mark_completed(it->second);
                        }
                        pending_row_ids_.erase(it);
                    }
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

    void tls_listener_loop() {
        while (tls_listener_running_.load()) {
            auto client = tls_listener_.accept();
            if (!client.valid()) {
                if (!tls_listener_running_.load()) break;
                continue;
            }

            std::thread([this, client_sock = std::move(client)]() mutable {
                this->handle_tls_client(std::move(client_sock));
            }).detach();
        }
    }

    void handle_tls_client(transport::TlsSocket client) {
        while (true) {
            auto line = client.recv_until('\n');
            if (!line.has_value()) break;
            if (line->empty()) continue;

            auto msg = NusantaraMessage::from_json(*line);
            if (msg.has_value()) {
                send_message(*msg);
            } else {
                std::cerr << "[GARUDA NUSANTARA WARNING] Formulasi JSON TLS tidak valid: " << *line << "\n";
            }
        }
    }

    void ws_listener_loop() {
        while (ws_listener_running_.load()) {
            auto client = ws_listener_.accept();
            if (!client.valid()) {
                if (!ws_listener_running_.load()) break;
                continue;
            }

            std::thread([this, client_sock = std::move(client)]() mutable {
                this->handle_ws_client(std::move(client_sock));
            }).detach();
        }
    }

    void handle_ws_client(transport::WsSocket client) {
        while (true) {
            auto text = client.recv_text();
            if (!text.has_value()) break;
            if (text->empty()) continue;

            auto msg = NusantaraMessage::from_json(*text);
            if (msg.has_value()) {
                send_message(*msg);
            } else {
                std::cerr << "[GARUDA NUSANTARA WARNING] Formulasi JSON WS tidak valid: " << *text << "\n";
            }
        }
    }

    void wss_listener_loop() {
        while (wss_listener_running_.load()) {
            auto client = wss_listener_.accept();
            if (!client.valid()) {
                if (!wss_listener_running_.load()) break;
                continue;
            }

            std::thread([this, client_sock = std::move(client)]() mutable {
                this->handle_wss_client(std::move(client_sock));
            }).detach();
        }
    }

    void handle_wss_client(transport::WssSocket client) {
        while (true) {
            auto text = client.recv_text();
            if (!text.has_value()) break;
            if (text->empty()) continue;

            auto msg = NusantaraMessage::from_json(*text);
            if (msg.has_value()) {
                send_message(*msg);
            } else {
                std::cerr << "[GARUDA NUSANTARA WARNING] Formulasi JSON WSS tidak valid: " << *text << "\n";
            }
        }
    }

public:
    static NusantaraBroker& get_instance() {
        static NusantaraBroker instance;
        return instance;
    }

    ~NusantaraBroker() {
        stop_wss_listener();
        stop_ws_listener();
        stop_listener_tls();
        stop_listener();
        stop();
        disable_persistence();
    }

    // === PERSISTENSI SQLITE WAL API ===
    bool enable_persistence(const std::string& db_path) {
        std::lock_guard<std::mutex> lock(store_mutex_);
        if (persistence_on_.load()) return false;
        auto new_store = std::make_unique<NusantaraMessageStore>();
        if (!new_store->open(db_path)) return false;
        store_ = std::move(new_store);
        persistence_on_.store(true);
        std::cout << "[GARUDA NUSANTARA] Persistensi aktif: " << db_path << "\n";
        return true;
    }

    void disable_persistence() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            std::queue<NusantaraMessage> empty;
            std::swap(message_queue_, empty);
        }
        std::lock_guard<std::mutex> lock(store_mutex_);
        if (!persistence_on_.load()) return;
        persistence_on_.store(false);
        if (store_) {
            store_->close();
            store_.reset();
        }
        pending_row_ids_.clear();
        std::cout << "[GARUDA NUSANTARA] Persistensi nonaktif.\n";
    }

    bool persistence_enabled() const {
        return persistence_on_.load();
    }

    std::size_t persistent_pending_count() const {
        std::lock_guard<std::mutex> lock(store_mutex_);
        if (persistence_on_.load() && store_) {
            return store_->pending_count();
        }
        return 0;
    }

    void start() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (running_.load()) return;
            running_.store(true);
        }

        if (persistence_on_.load()) {
            std::lock_guard<std::mutex> store_lock(store_mutex_);
            if (store_ && store_->is_open()) {
                auto pending = store_->load_pending();
                if (!pending.empty()) {
                    std::lock_guard<std::mutex> lock(mutex_);
                    for (const auto& [row_id, msg] : pending) {
                        message_queue_.push(msg);
                        pending_row_ids_[msg.message_id] = row_id;
                    }
                    std::cout << "[GARUDA NUSANTARA] Recovery: " << pending.size() << " pesan pending dari DB.\n";
                }
            }
        }

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

        if (persistence_on_.load()) {
            std::lock_guard<std::mutex> store_lock(store_mutex_);
            if (store_ && store_->is_open()) {
                std::size_t pruned = store_->prune_completed(3600);
                if (pruned > 0) {
                    std::cout << "[GARUDA NUSANTARA] Prune DB: " << pruned << " pesan 'done' dibersihkan.\n";
                }
            }
        }

        std::cout << "[GARUDA NUSANTARA] Async Worker Broker dihentikan.\n";
    }

    // === PLAINTEXT TCP LISTENER API ===
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

    // === TLS TCP LISTENER API ===
    bool start_listener_tls(std::uint16_t port, const std::string& cert_pem, const std::string& key_pem) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (tls_listener_running_.load()) return false;
        if (!tls_listener_.listen("0.0.0.0", port, cert_pem, key_pem)) return false;

        tls_listener_running_.store(true);
        tls_listener_thread_ = std::thread([this]() { this->tls_listener_loop(); });
        std::cout << "[GARUDA NUSANTARA] Listener TLS terpasang pada port: " << tls_listener_.port() << "\n";
        return true;
    }

    std::uint16_t tls_listener_port() const {
        return tls_listener_.port();
    }

    bool tls_listener_active() const {
        return tls_listener_running_.load();
    }

    void stop_listener_tls() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!tls_listener_running_.load()) return;
            tls_listener_running_.store(false);
            tls_listener_.close();
        }
        if (tls_listener_thread_.joinable()) {
            tls_listener_thread_.join();
        }
        std::cout << "[GARUDA NUSANTARA] Listener TLS dihentikan.\n";
    }

    bool send_remote_tls(const std::string& host, std::uint16_t port, const NusantaraMessage& msg, const std::string& ca_cert_path = "") {
        auto sock = transport::connect_tls_to(host, port, ca_cert_path);
        if (!sock.has_value()) return false;
        std::string payload = msg.to_json() + "\n";
        return sock->send_all(payload);
    }

    // === PLAINTEXT WEBSOCKET API ===
    bool start_ws_listener(std::uint16_t port) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (ws_listener_running_.load()) return false;
        if (!ws_listener_.listen("0.0.0.0", port)) return false;

        ws_listener_running_.store(true);
        ws_listener_thread_ = std::thread([this]() { this->ws_listener_loop(); });
        std::cout << "[GARUDA NUSANTARA] Listener WS terpasang pada port: " << ws_listener_.port() << "\n";
        return true;
    }

    std::uint16_t ws_listener_port() const {
        return ws_listener_.port();
    }

    bool ws_listener_active() const {
        return ws_listener_running_.load();
    }

    void stop_ws_listener() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!ws_listener_running_.load()) return;
            ws_listener_running_.store(false);
            ws_listener_.close();
        }
        if (ws_listener_thread_.joinable()) {
            ws_listener_thread_.join();
        }
        std::cout << "[GARUDA NUSANTARA] Listener WS dihentikan.\n";
    }

    bool send_remote_ws(const std::string& host, std::uint16_t port, const NusantaraMessage& msg) {
        auto sock = transport::connect_ws_to(host, port);
        if (!sock.has_value()) return false;
        return sock->send_text(msg.to_json());
    }

    // === WSS (WEBSOCKET OVER TLS) API ===
    bool start_wss_listener(std::uint16_t port, const std::string& cert_pem, const std::string& key_pem) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (wss_listener_running_.load()) return false;
        if (!wss_listener_.listen("0.0.0.0", port, cert_pem, key_pem)) return false;

        wss_listener_running_.store(true);
        wss_listener_thread_ = std::thread([this]() { this->wss_listener_loop(); });
        std::cout << "[GARUDA NUSANTARA] Listener WSS terpasang pada port: " << wss_listener_.port() << "\n";
        return true;
    }

    std::uint16_t wss_listener_port() const {
        return wss_listener_.port();
    }

    bool wss_listener_active() const {
        return wss_listener_running_.load();
    }

    void stop_wss_listener() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!wss_listener_running_.load()) return;
            wss_listener_running_.store(false);
            wss_listener_.close();
        }
        if (wss_listener_thread_.joinable()) {
            wss_listener_thread_.join();
        }
        std::cout << "[GARUDA NUSANTARA] Listener WSS dihentikan.\n";
    }

    bool send_remote_wss(const std::string& host, std::uint16_t port, const NusantaraMessage& msg, const std::string& ca_cert_path = "") {
        auto sock = transport::connect_wss_to(host, port, ca_cert_path);
        if (!sock.has_value()) return false;
        return sock->send_text(msg.to_json());
    }

    // Mendaftarkan entitas (misal: Autopilot, Richat, GVM)
    void register_entity(EntityTarget target, EntityReceiver receiver) {
        std::lock_guard<std::mutex> lock(mutex_);
        receivers_[target] = receiver;
        std::cout << "[GARUDA NUSANTARA] Entitas terdaftar: " << (int)target << "\n";
    }

    // Mengirim pesan (Bisa dari mana saja)
    void send_message(const NusantaraMessage& msg) {
        if (persistence_on_.load()) {
            std::lock_guard<std::mutex> store_lock(store_mutex_);
            if (store_ && store_->is_open()) {
                std::int64_t row_id = store_->store_message(msg);
                if (row_id >= 0) {
                    pending_row_ids_[msg.message_id] = row_id;
                }
            }
        }

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

                if (persistence_on_.load()) {
                    std::lock_guard<std::mutex> store_lock(store_mutex_);
                    auto it = pending_row_ids_.find(msg.message_id);
                    if (it != pending_row_ids_.end()) {
                        if (store_ && store_->is_open()) {
                            store_->mark_completed(it->second);
                        }
                        pending_row_ids_.erase(it);
                    }
                }
            }
        }
    }
};

} // namespace garuda::nusantara
