#pragma once
// Garuda BEAM-core: Advanced Concurrency Primitives (`concurrency_primitives.hpp`)
// Mengimplementasikan Mutex, Semaphore, CSP Channel (dengan Select/Pilih), Future/Promise, & Task.

#include "term.hpp"
#include <chrono>
#include <condition_variable>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <string>
#include <thread>
#include <vector>

namespace garuda::beam {

// === 1. Mutex Primitive ===
class Mutex {
public:
    Mutex() = default;

    void kunci() { mu_.lock(); }
    void buka() { mu_.unlock(); }
    bool coba_kunci() { return mu_.try_lock(); }

    void lock() { kunci(); }
    void unlock() { buka(); }
    bool try_lock() { return coba_kunci(); }

private:
    std::mutex mu_;
};

// === 2. Semaphore Primitive ===
class Semaphore {
public:
    explicit Semaphore(int initial_permits = 1) : count_(initial_permits) {}

    void tunggu() {
        std::unique_lock<std::mutex> lock(mu_);
        cv_.wait(lock, [this]() { return count_ > 0; });
        --count_;
    }

    void lepas() {
        std::unique_lock<std::mutex> lock(mu_);
        ++count_;
        cv_.notify_one();
    }

    void wait() { tunggu(); }
    void signal() { lepas(); }
    void release() { lepas(); }

private:
    std::mutex mu_;
    std::condition_variable cv_;
    int count_;
};

// === 3. CSP Channel Primitive ===
template <typename T = Term>
class Channel {
public:
    explicit Channel(size_t capacity = 100) : capacity_(capacity), closed_(false) {}

    bool kirim(const T& item) {
        std::unique_lock<std::mutex> lock(mu_);
        cv_send_.wait(lock, [this]() { return queue_.size() < capacity_ || closed_; });
        if (closed_) return false;
        queue_.push(item);
        cv_recv_.notify_one();
        return true;
    }

    std::optional<T> terima() {
        std::unique_lock<std::mutex> lock(mu_);
        cv_recv_.wait(lock, [this]() { return !queue_.empty() || closed_; });
        if (queue_.empty() && closed_) return std::nullopt;
        T val = queue_.front();
        queue_.pop();
        cv_send_.notify_one();
        return val;
    }

    void tutup() {
        std::unique_lock<std::mutex> lock(mu_);
        closed_ = true;
        cv_recv_.notify_all();
        cv_send_.notify_all();
    }

    bool apakah_tutup() const {
        std::lock_guard<std::mutex> lock(mu_);
        return closed_;
    }

    size_t ukuran() const {
        std::lock_guard<std::mutex> lock(mu_);
        return queue_.size();
    }

    bool send(const T& item) { return kirim(item); }
    std::optional<T> receive() { return terima(); }
    void close() { tutup(); }

private:
    mutable std::mutex mu_;
    std::condition_variable cv_send_;
    std::condition_variable cv_recv_;
    std::queue<T> queue_;
    size_t capacity_;
    bool closed_;
};

// === 4. Future / Promise Primitive ===
template <typename T = Term>
class Promise {
public:
    Promise() : future_(promise_.get_future()) {}

    void selesai(const T& val) {
        promise_.set_value(val);
    }

    void resolve(const T& val) { selesai(val); }

    T tunggu() {
        return future_.get();
    }

    T await_value() { return tunggu(); }

    bool siap() const {
        return future_.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
    }

private:
    std::promise<T> promise_;
    std::future<T> future_;
};

// === 5. Asynchronous Task Primitive ===
class Task {
public:
    Task() = default;

    template <typename Func>
    static Task spawn(Func&& func) {
        Task t;
        t.thread_ = std::thread([func = std::forward<Func>(func), promise = t.promise_]() mutable {
            try {
                Term res = func();
                promise->selesai(res);
            } catch (const std::exception& e) {
                promise->selesai(Term(Atom{"gagal"}));
            }
        });
        return t;
    }

    Term join() {
        if (thread_.joinable()) {
            thread_.join();
        }
        return promise_->tunggu();
    }

    void cancel() {
        if (thread_.joinable()) {
            thread_.detach();
        }
    }

private:
    std::thread thread_;
    std::shared_ptr<Promise<Term>> promise_ = std::make_shared<Promise<Term>>();
};

} // namespace garuda::beam
