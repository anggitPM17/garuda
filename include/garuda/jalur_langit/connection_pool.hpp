#pragma once
#include <string>
#include <vector>
#include <mutex>

namespace garuda::jalur_langit {
    class ConnectionPool {
    public:
        ConnectionPool(size_t capacity, const std::string& path)
            : capacity_(capacity), path_(path), available_(capacity) {}

        void* acquire() {
            std::lock_guard<std::mutex> lock(mutex_);
            if (available_ == 0) return nullptr;
            available_--;
            return (void*)0x12345;
        }

        void release(void* conn) {
            std::lock_guard<std::mutex> lock(mutex_);
            if (available_ < capacity_) available_++;
        }

        size_t available_connections() const {
            return available_;
        }

    private:
        size_t capacity_;
        std::string path_;
        size_t available_;
        mutable std::mutex mutex_;
    };
}
