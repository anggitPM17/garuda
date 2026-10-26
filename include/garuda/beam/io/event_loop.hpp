#pragma once
// Event loop: menyatukan scheduler proses, timer wheel, I/O readiness (poll/WSAPoll) dan
// injeksi thread-safe (post). Mode Real = jam dinding; Virtual = waktu loncat ke timer
// berikutnya (deterministik, cocok untuk tes).
#include "../core/runtime.hpp"

#include <atomic>
#include <chrono>
#include <mutex>
#include <vector>

#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <cerrno>
#endif

namespace garuda::beam {

enum class ClockMode { Real, Virtual };

class EventLoop {
public:
    explicit EventLoop(Runtime& rt, ClockMode mode = ClockMode::Real)
        : rt_(rt), mode_(mode), start_(std::chrono::steady_clock::now()) {
#if defined(_WIN32)
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
        SOCKET listener = ::socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        addr.sin_port = 0;
        ::bind(listener, (struct sockaddr*)&addr, sizeof(addr));
        ::listen(listener, 1);
        int len = sizeof(addr);
        ::getsockname(listener, (struct sockaddr*)&addr, &len);
        wake_[1] = static_cast<int>(::socket(AF_INET, SOCK_STREAM, 0));
        ::connect(static_cast<SOCKET>(wake_[1]), (struct sockaddr*)&addr, sizeof(addr));
        wake_[0] = static_cast<int>(::accept(listener, nullptr, nullptr));
        closesocket(listener);
        u_long nonblock = 1;
        ::ioctlsocket(static_cast<SOCKET>(wake_[0]), FIONBIO, &nonblock);
        ::ioctlsocket(static_cast<SOCKET>(wake_[1]), FIONBIO, &nonblock);
#else
        if (::pipe(wake_) == 0) {
            for (int fd : wake_) ::fcntl(fd, F_SETFL, ::fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
        } else { wake_[0] = wake_[1] = -1; }
#endif
    }
    ~EventLoop() {
#if defined(_WIN32)
        if (wake_[0] >= 0) ::closesocket(static_cast<SOCKET>(wake_[0]));
        if (wake_[1] >= 0) ::closesocket(static_cast<SOCKET>(wake_[1]));
#else
        if (wake_[0] >= 0) { ::close(wake_[0]); ::close(wake_[1]); }
#endif
    }
    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop&) = delete;

    Runtime& runtime() { return rt_; }
    void set_keep_alive(bool v) { keep_alive_ = v; }

    // Kirim {io_ready, Tag, Fd} ke `owner` setiap kali fd siap dibaca (level-triggered).
    void watch_read(int fd, Pid owner, Term tag) { watches_[fd] = Watch{owner, std::move(tag)}; }
    void unwatch(int fd) { watches_.erase(fd); }

    // Thread-safe: jalankan fn(Runtime&) di thread loop.
    void post(std::function<void(Runtime&)> fn) {
        { std::lock_guard<std::mutex> g(mu_); posts_.push_back(std::move(fn)); }
        wake();
    }
    void stop() { stopped_ = true; wake(); }

    // allow_jump=false: pada mode Virtual jangan loncatkan waktu ke timer berikutnya (dipakai run_until).
    bool run_once(bool allow_jump = true) {
        drain_posts();
        sync_clock();
        for (int i = 0; i < kSlicesPerTurn && rt_.run_slice(); ++i) {}
        sync_clock();
        bool runnable = rt_.has_runnable();
        auto nt = rt_.next_timer();
        int timeout;
        if (runnable) timeout = 0;
        else if (nt) {
            if (mode_ == ClockMode::Virtual) timeout = 0;
            else {
                std::uint64_t now = rt_.now_ms();
                timeout = *nt > now ? static_cast<int>(std::min<std::uint64_t>(*nt - now, 1000000)) : 0;
            }
        } else timeout = (watches_.empty() && !keep_alive_ && !posts_pending()) ? 0 : -1;
        bool woke = poll_io(timeout);
        if (!runnable && !woke && nt && mode_ == ClockMode::Virtual) {
            if (allow_jump) rt_.advance_time(*nt);
            return true;
        }
        if (mode_ == ClockMode::Real) sync_clock();
        return rt_.has_runnable() || rt_.next_timer() || !watches_.empty() || keep_alive_ || woke || posts_pending();
    }

    void run() { while (!stopped_ && run_once()) {} }

    bool run_until(const std::function<bool()>& done, std::uint64_t timeout_ms) {
        std::uint64_t deadline = rt_.now_ms() + timeout_ms;
        TimerId guard = rt_.after(timeout_ms, [] {});
        while (!done()) {
            if (rt_.now_ms() >= deadline) break;
            bool more = run_once(false);
            if (done()) break;
            if (mode_ == ClockMode::Virtual && !rt_.has_runnable()) {
                // idle: baru sekarang waktu virtual boleh loncat (kondisi `done` sudah dicek dulu)
                if (auto nt = rt_.next_timer()) rt_.advance_time(*nt);
                else if (!more) break;
            } else if (!more) break;
        }
        rt_.cancel_timer(guard);
        return done();
    }

    // gen_server call sinkron dari luar proses.
    std::optional<Term> call(Pid server, Term request, std::uint64_t timeout_ms = 5000) {
        Pid probe = rt_.spawn([](Process&) { return Status::Wait; });
        Ref ref = rt_.make_ref();
        rt_.send(probe, server, tuple({atom("$gen_call"), tuple({Term(probe), Term(ref)}), std::move(request)}));
        std::optional<Term> out;
        run_until([&] {
            if (out) return true;
            auto m = rt_.take(probe, [&](const Message& msg) {
                const Term* r = tuple_at(msg.body, 0);
                return tuple_size(msg.body) == 2 && r && r->get_if<Ref>() && *r->get_if<Ref>() == ref;
            });
            if (m) { out = *tuple_at(m->body, 1); return true; }
            return false;
        }, timeout_ms);
        rt_.exit_signal(Pid{}, probe, atom("kill"));
        return out;
    }

private:
    struct Watch { Pid owner; Term tag; };
    static constexpr int kSlicesPerTurn = 256;

    void sync_clock() {
        if (mode_ != ClockMode::Real) return;
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_).count();
        rt_.advance_time(static_cast<std::uint64_t>(ms));
    }

    void wake() {
        if (wake_[1] >= 0) {
            char c = 1;
#if defined(_WIN32)
            ::send(static_cast<SOCKET>(wake_[1]), &c, 1, 0);
#else
            (void)!::write(wake_[1], &c, 1);
#endif
        }
    }

    bool posts_pending() { std::lock_guard<std::mutex> g(mu_); return !posts_.empty(); }
    void drain_posts() {
        std::vector<std::function<void(Runtime&)>> local;
        { std::lock_guard<std::mutex> g(mu_); local.swap(posts_); }
        for (auto& fn : local) fn(rt_);
    }

    bool poll_io(int timeout) {
#if defined(_WIN32)
        std::vector<WSAPOLLFD> fds;
        fds.push_back({static_cast<SOCKET>(wake_[0]), POLLIN, 0});
        for (auto& [fd, w] : watches_) fds.push_back({static_cast<SOCKET>(fd), POLLIN, 0});
        int n = ::WSAPoll(fds.data(), static_cast<ULONG>(fds.size()), timeout);
        if (n <= 0) return false;
        bool any = false;
        if (fds[0].revents & POLLIN) {
            char buf[64];
            while (::recv(static_cast<SOCKET>(wake_[0]), buf, sizeof(buf), 0) > 0) {}
            any = true;
        }
        for (std::size_t i = 1; i < fds.size(); ++i) {
            if (!(fds[i].revents & (POLLIN | POLLHUP | POLLERR))) continue;
            auto it = watches_.find(static_cast<int>(fds[i].fd));
            if (it == watches_.end()) continue;
            rt_.send(Pid{}, it->second.owner, tuple({atom("io_ready"), it->second.tag, Term(static_cast<int>(fds[i].fd))}));
            any = true;
        }
        return any;
#else
        std::vector<pollfd> fds;
        fds.push_back({wake_[0], POLLIN, 0});
        for (auto& [fd, w] : watches_) fds.push_back({fd, POLLIN, 0});
        int n;
        do { n = ::poll(fds.data(), fds.size(), timeout); } while (n < 0 && errno == EINTR);
        if (n <= 0) return false;
        bool any = false;
        if (fds[0].revents & POLLIN) {
            char buf[64];
            while (::read(wake_[0], buf, sizeof buf) > 0) {}
            any = true;
        }
        for (std::size_t i = 1; i < fds.size(); ++i) {
            if (!(fds[i].revents & (POLLIN | POLLHUP | POLLERR))) continue;
            auto it = watches_.find(fds[i].fd);
            if (it == watches_.end()) continue;
            rt_.send(Pid{}, it->second.owner, tuple({atom("io_ready"), it->second.tag, Term(fds[i].fd)}));
            any = true;
        }
        return any;
#endif
    }

    Runtime& rt_;
    ClockMode mode_;
    std::chrono::steady_clock::time_point start_;
    std::map<int, Watch> watches_;
    std::vector<std::function<void(Runtime&)>> posts_;
    std::mutex mu_;
    int wake_[2] = {-1, -1};
    std::atomic<bool> keep_alive_{false};
    std::atomic<bool> stopped_{false};
};

}  // namespace garuda::beam
