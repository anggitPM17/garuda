#pragma once
// Garuda Million-Scale Engine: Work-Stealing Multi-Core Scheduler
// Algoritma Work-Stealing antar thread/shard untuk memastikan 100% pemanfaatan CPU saat jutaan proses berjalan.
#include "lockfree_queue.hpp"
#include "runtime.hpp"

#include <atomic>
#include <chrono>
#include <iostream>
#include <memory>
#include <random>
#include <thread>
#include <vector>

namespace garuda::beam {

class WorkStealingPool {
public:
    explicit WorkStealingPool(std::size_t num_threads = std::thread::hardware_concurrency())
        : num_threads_(num_threads > 0 ? num_threads : 2), running_(false) {
        queues_.resize(num_threads_);
        for (std::size_t i = 0; i < num_threads_; ++i) {
            queues_[i] = std::make_unique<LockFreeMpmcQueue<std::function<void()>, 16384>>();
        }
    }

    ~WorkStealingPool() {
        stop();
    }

    void start() {
        if (running_) return;
        running_ = true;
        for (std::size_t i = 0; i < num_threads_; ++i) {
            workers_.emplace_back([this, i]() {
                this->worker_loop(i);
            });
        }
        std::cout << "⚡ [GARUDA WORK-STEALING] Started " << num_threads_ << " multi-core worker schedulers." << std::endl;
    }

    void stop() {
        if (!running_) return;
        running_ = false;
        for (auto& w : workers_) {
            if (w.joinable()) w.join();
        }
        workers_.clear();
    }

    bool submit(std::size_t thread_hint, std::function<void()> task) {
        std::size_t idx = thread_hint % num_threads_;
        return queues_[idx]->push(std::move(task));
    }

    std::size_t num_threads() const { return num_threads_; }

private:
    void worker_loop(std::size_t thread_id) {
        std::random_device rd;
        std::mt19937 gen(rd() + static_cast<unsigned>(thread_id));
        std::uniform_int_distribution<std::size_t> dis(0, num_threads_ - 1);

        while (running_) {
            // 1. Pop dari queue sendiri
            auto task = queues_[thread_id]->pop();
            if (task) {
                (*task)();
                steals_success_++;
                continue;
            }

            // 2. Work-Stealing: curi dari queue thread lain jika queue sendiri kosong
            bool stole = false;
            for (std::size_t attempts = 0; attempts < num_threads_; ++attempts) {
                std::size_t victim_id = dis(gen);
                if (victim_id == thread_id) continue;

                auto stolen_task = queues_[victim_id]->pop();
                if (stolen_task) {
                    (*stolen_task)();
                    steals_stolen_++;
                    stole = true;
                    break;
                }
            }

            if (!stole) {
                std::this_thread::yield();
            }
        }
    }

    std::size_t num_threads_;
    std::atomic<bool> running_;
    std::vector<std::thread> workers_;
    std::vector<std::unique_ptr<LockFreeMpmcQueue<std::function<void()>, 16384>>> queues_;
    std::atomic<std::uint64_t> steals_success_{0};
    std::atomic<std::uint64_t> steals_stolen_{0};
};

} // namespace garuda::beam
