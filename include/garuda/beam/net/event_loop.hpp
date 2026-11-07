#pragma once

#include <vector>
#include <map>
#include <functional>
#include <mutex>
#include <atomic>
#include <thread>
#include <chrono>
#include <memory>
#include <algorithm>
#include <cstring>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
typedef SOCKET socket_t;
#define INVALID_SOCKET_VAL INVALID_SOCKET
#define CLOSE_SOCKET(s) closesocket(s)
#else
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#ifdef __linux__
#include <sys/epoll.h>
#endif
#ifdef __APPLE__
#include <sys/event.h>
#include <sys/time.h>
#endif
typedef int socket_t;
#define INVALID_SOCKET_VAL -1
#define CLOSE_SOCKET(s) close(s)
#endif

namespace garuda::beam::net {

constexpr int EV_READ  = 1 << 0;
constexpr int EV_WRITE = 1 << 1;
constexpr int EV_ERROR = 1 << 2;

struct Event {
    socket_t fd;
    int events;
};

// ---------------------------------------------------------------------
// Backend interface
// ---------------------------------------------------------------------
class EventLoopBackend {
public:
    virtual ~EventLoopBackend() = default;
    virtual bool        init() = 0;
    virtual int         wait(Event* out, int max_events, int timeout_ms) = 0;
    virtual bool        add(int fd) = 0;
    virtual bool        remove(int fd) = 0;
    virtual const char* name() const = 0;
};

// ---------------------------------------------------------------------
// PollBackend — portable fallback (poll on POSIX, WSAPoll on Windows)
// ---------------------------------------------------------------------
class PollBackend : public EventLoopBackend {
public:
    bool init() override { return true; }

    bool add(int fd) override {
        for (auto& p : fds_) if ((int)p.fd == fd) return true;
#ifdef _WIN32
        WSAPOLLFD pfd{}; pfd.fd = (SOCKET)fd; pfd.events = POLLRDNORM;
#else
        struct pollfd pfd{}; pfd.fd = fd; pfd.events = POLLIN;
#endif
        fds_.push_back(pfd);
        return true;
    }

    bool remove(int fd) override {
        for (auto it = fds_.begin(); it != fds_.end(); ++it) {
            if ((int)it->fd == fd) { fds_.erase(it); return true; }
        }
        return false;
    }

    int wait(Event* out, int max_events, int timeout_ms) override {
        if (fds_.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(
                timeout_ms < 0 ? 50 : (std::min)(timeout_ms, 50)));
            return 0;
        }
#ifdef _WIN32
        int ret = WSAPoll(fds_.data(), (ULONG)fds_.size(), timeout_ms);
#else
        int ret = poll(fds_.data(), (nfds_t)fds_.size(), timeout_ms);
#endif
        if (ret <= 0) return ret;

        int n = 0;
        for (auto& p : fds_) {
            if (n >= max_events) break;
            int ev = 0;
#ifdef _WIN32
            if (p.revents & (POLLRDNORM | POLLIN)) ev |= EV_READ;
            if (p.revents & (POLLERR | POLLHUP | POLLNVAL)) ev |= EV_ERROR;
#else
            if (p.revents & (POLLIN | POLLPRI)) ev |= EV_READ;
            if (p.revents & (POLLERR | POLLHUP | POLLNVAL)) ev |= EV_ERROR;
#endif
            if (ev != 0) {
                out[n].fd = (socket_t)p.fd;
                out[n].events = ev;
                ++n;
                p.revents = 0;
            }
        }
        return n;
    }

    const char* name() const override { return "poll"; }

private:
#ifdef _WIN32
    std::vector<WSAPOLLFD> fds_;
#else
    std::vector<struct pollfd> fds_;
#endif
};

// ---------------------------------------------------------------------
// EpollBackend — Linux
// ---------------------------------------------------------------------
#ifdef __linux__
class EpollBackend : public EventLoopBackend {
public:
    bool init() override {
        epfd_ = epoll_create1(EPOLL_CLOEXEC);
        return epfd_ >= 0;
    }
    ~EpollBackend() override { if (epfd_ >= 0) ::close(epfd_); }

    bool add(int fd) override {
        epoll_event ev{};
        ev.events = EPOLLIN;
        ev.data.fd = fd;
        return epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &ev) == 0;
    }
    bool remove(int fd) override {
        return epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, nullptr) == 0;
    }
    int wait(Event* out, int max_events, int timeout_ms) override {
        epoll_event evs[512];
        int max = std::min(max_events, 512);
        int n = epoll_wait(epfd_, evs, max, timeout_ms);
        if (n <= 0) return n;
        for (int i = 0; i < n; ++i) {
            out[i].fd = evs[i].data.fd;
            out[i].events = 0;
            if (evs[i].events & EPOLLIN)                 out[i].events |= EV_READ;
            if (evs[i].events & EPOLLOUT)                out[i].events |= EV_WRITE;
            if (evs[i].events & (EPOLLERR | EPOLLHUP))   out[i].events |= EV_ERROR;
        }
        return n;
    }
    const char* name() const override { return "epoll"; }
private:
    int epfd_ = -1;
};
#endif

// ---------------------------------------------------------------------
// KqueueBackend — macOS / BSD
// ---------------------------------------------------------------------
#ifdef __APPLE__
class KqueueBackend : public EventLoopBackend {
public:
    bool init() override { kq_ = kqueue(); return kq_ >= 0; }
    ~KqueueBackend() override { if (kq_ >= 0) ::close(kq_); }

    bool add(int fd) override {
        struct kevent ke{};
        EV_SET(&ke, fd, EVFILT_READ, EV_ADD, 0, 0, nullptr);
        return kevent(kq_, &ke, 1, nullptr, 0, nullptr) == 0;
    }
    bool remove(int fd) override {
        struct kevent ke{};
        EV_SET(&ke, fd, EVFILT_READ, EV_DELETE, 0, 0, nullptr);
        kevent(kq_, &ke, 1, nullptr, 0, nullptr);
        return true;  // idempotent: EBADF not fatal
    }
    int wait(Event* out, int max_events, int timeout_ms) override {
        struct timespec ts{ timeout_ms / 1000, (timeout_ms % 1000) * 1000000L };
        std::vector<struct kevent> evs((size_t)max_events);
        int n = kevent(kq_, nullptr, 0, evs.data(), max_events, &ts);
        if (n <= 0) return n;
        for (int i = 0; i < n; ++i) {
            out[i].fd = (socket_t)evs[i].ident;
            out[i].events = 0;
            if (evs[i].filter == EVFILT_READ)  out[i].events |= EV_READ;
            if (evs[i].flags & EV_ERROR)       out[i].events |= EV_ERROR;
        }
        return n;
    }
    const char* name() const override { return "kqueue"; }
private:
    int kq_ = -1;
};
#endif

// ---------------------------------------------------------------------
// Factory
// ---------------------------------------------------------------------
inline std::unique_ptr<EventLoopBackend> make_default_backend() {
#ifdef __linux__
    {
        auto b = std::make_unique<EpollBackend>();
        if (b->init()) return b;
    }
#endif
#ifdef __APPLE__
    {
        auto b = std::make_unique<KqueueBackend>();
        if (b->init()) return b;
    }
#endif
    auto b = std::make_unique<PollBackend>();
    b->init();
    return b;
}

// ---------------------------------------------------------------------
// EventLoop facade — API publik Fase 11 dipertahankan 100%
// ---------------------------------------------------------------------
class EventLoop {
public:
    using ReadCallback = std::function<void(socket_t)>;

    EventLoop() : backend_(make_default_backend()), running_(false) {}
    ~EventLoop() { stop(); }

    const char* backend_name() const { return backend_ ? backend_->name() : "none"; }

    void add_socket(socket_t fd, ReadCallback cb) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (callbacks_.count(fd) == 0) backend_->add((int)fd);
        callbacks_[fd] = std::move(cb);
    }

    void remove_socket(socket_t fd) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = callbacks_.find(fd);
        if (it != callbacks_.end()) {
            backend_->remove((int)fd);
            callbacks_.erase(it);
        }
    }

    void start_in_thread() {
        if (running_.exchange(true)) return;
        loop_thread_ = std::thread([this]() { run(); });
    }

    void run() {
        while (running_) {
            Event evs[256];
            int n = backend_->wait(evs, 256, 50);
            if (n <= 0) continue;
            for (int i = 0; i < n; ++i) {
                if (!(evs[i].events & EV_READ)) continue;
                ReadCallback cb;
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    auto it = callbacks_.find(evs[i].fd);
                    if (it != callbacks_.end()) cb = it->second;
                }
                if (cb) cb(evs[i].fd);
            }
        }
    }

    void stop() {
        running_ = false;
        if (loop_thread_.joinable()) loop_thread_.join();
    }

    bool is_running() const { return running_; }

private:
    std::unique_ptr<EventLoopBackend> backend_;
    std::atomic<bool>                 running_;
    std::map<socket_t, ReadCallback>  callbacks_;
    std::mutex                        mutex_;
    std::thread                       loop_thread_;
};

} // namespace garuda::beam::net
