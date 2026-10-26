#pragma once
// Garuda Million-Scale Engine: Lock-Free Multi-Producer Multi-Consumer (MPMC) Queue
// Menggunakan std::atomic memory order (acquire/release) untuk throughput jutaan pesan/detik tanpa lock contention.
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

namespace garuda::beam {

template <typename T, std::size_t Capacity = 65536>
class LockFreeMpmcQueue {
public:
    LockFreeMpmcQueue() {
        for (std::size_t i = 0; i < Capacity; ++i) {
            buffer_[i].sequence.store(i, std::memory_order_relaxed);
        }
        enqueue_pos_.store(0, std::memory_order_relaxed);
        dequeue_pos_.store(0, std::memory_order_relaxed);
    }

    bool push(T data) {
        Cell* cell;
        std::size_t pos = enqueue_pos_.load(std::memory_order_relaxed);
        while (true) {
            cell = &buffer_[pos & (Capacity - 1)];
            std::size_t seq = cell->sequence.load(std::memory_order_acquire);
            intptr_t diff = static_cast<intptr_t>(seq) - static_cast<intptr_t>(pos);
            if (diff == 0) {
                if (enqueue_pos_.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) {
                    break;
                }
            } else if (diff < 0) {
                return false; // Queue full
            } else {
                pos = enqueue_pos_.load(std::memory_order_relaxed);
            }
        }
        cell->data = std::move(data);
        cell->sequence.store(pos + 1, std::memory_order_release);
        return true;
    }

    std::optional<T> pop() {
        Cell* cell;
        std::size_t pos = dequeue_pos_.load(std::memory_order_relaxed);
        while (true) {
            cell = &buffer_[pos & (Capacity - 1)];
            std::size_t seq = cell->sequence.load(std::memory_order_acquire);
            intptr_t diff = static_cast<intptr_t>(seq) - static_cast<intptr_t>(pos + 1);
            if (diff == 0) {
                if (dequeue_pos_.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) {
                    break;
                }
            } else if (diff < 0) {
                return std::nullopt; // Queue empty
            } else {
                pos = dequeue_pos_.load(std::memory_order_relaxed);
            }
        }
        T data = std::move(cell->data);
        cell->sequence.store(pos + (Capacity - 1) + 1, std::memory_order_release);
        return data;
    }

    std::size_t capacity() const { return Capacity; }

private:
    struct Cell {
        std::atomic<std::size_t> sequence;
        T data;
    };

    static constexpr std::size_t kCacheLineSize = 64;
    alignas(kCacheLineSize) Cell buffer_[Capacity];
    alignas(kCacheLineSize) std::atomic<std::size_t> enqueue_pos_;
    alignas(kCacheLineSize) std::atomic<std::size_t> dequeue_pos_;
};

} // namespace garuda::beam
