#pragma once

#include "jalur_langit_ultimate.hpp"
#include <vector>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <string>
#include <cstddef>

namespace garuda::jalur_langit {

class ConnectionPool {
private:
    std::vector<std::shared_ptr<JalurLangitUltimate>> pool_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    size_t max_connections_;

public:
    ConnectionPool(size_t max_conn, const std::string& data_dir = "./jalur_langit_data")
        : max_connections_(max_conn) {
        for (size_t i = 0; i < max_conn; ++i) {
            pool_.push_back(std::make_shared<JalurLangitUltimate>(data_dir));
        }
    }

    ~ConnectionPool() = default;

    // Pinjam koneksi (blocking jika pool kosong)
    std::shared_ptr<JalurLangitUltimate> acquire() {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this]() { return !pool_.empty(); });
        auto conn = pool_.back();
        pool_.pop_back();
        return conn;
    }

    // Kembalikan koneksi ke pool
    void release(std::shared_ptr<JalurLangitUltimate> conn) {
        if (!conn) return;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            pool_.push_back(conn);
        }
        cv_.notify_one();
    }

    // Query jumlah koneksi yang tersedia (untuk monitoring)
    size_t available_connections() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return pool_.size();
    }
};

} // namespace garuda::jalur_langit
