#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <functional>
#include <mutex>
#include <atomic>
#include <thread>
#include <chrono>
#include <algorithm>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
typedef SOCKET socket_t;
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
typedef int socket_t;
#define INVALID_SOCKET_VAL -1
#define CLOSE_SOCKET(s) close(s)
#endif

namespace garuda::beam::net {

class EventLoop {
public:
    using ReadCallback = std::function<void(socket_t)>;

    EventLoop() : running_(false) {}

    ~EventLoop() {
        stop();
    }

    void add_socket(socket_t fd, ReadCallback cb) {
        std::lock_guard<std::mutex> lock(mutex_);
        callbacks_[fd] = cb;
    }

    void remove_socket(socket_t fd) {
        std::lock_guard<std::mutex> lock(mutex_);
        callbacks_.erase(fd);
    }

    void start_in_thread() {
        running_ = true;
        loop_thread_ = std::thread([this]() { run(); });
    }

    void run() {
        running_ = true;
        while (running_) {
            std::map<socket_t, ReadCallback> current_cbs;
            {
                std::lock_guard<std::mutex> lock(mutex_);
                current_cbs = callbacks_;
            }

            if (current_cbs.empty()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
                continue;
            }

            #ifdef _WIN32
            std::vector<WSAPOLLFD> poll_fds;
            poll_fds.reserve(current_cbs.size());
            std::vector<socket_t> fds_list;
            fds_list.reserve(current_cbs.size());

            for (const auto& [fd, cb] : current_cbs) {
                WSAPOLLFD pfd;
                pfd.fd = fd;
                pfd.events = POLLIN;
                pfd.revents = 0;
                poll_fds.push_back(pfd);
                fds_list.push_back(fd);
            }

            int ret = WSAPoll(poll_fds.data(), static_cast<ULONG>(poll_fds.size()), 50);

            if (ret > 0) {
                for (size_t i = 0; i < poll_fds.size(); ++i) {
                    if (poll_fds[i].revents & POLLIN) {
                        socket_t fd = fds_list[i];
                        ReadCallback cb = nullptr;
                        {
                            std::lock_guard<std::mutex> lock(mutex_);
                            auto it = callbacks_.find(fd);
                            if (it != callbacks_.end()) cb = it->second;
                        }
                        if (cb) cb(fd);
                    }
                }
            }
            #else
            std::vector<struct pollfd> poll_fds;
            poll_fds.reserve(current_cbs.size());
            std::vector<socket_t> fds_list;
            fds_list.reserve(current_cbs.size());

            for (const auto& [fd, cb] : current_cbs) {
                struct pollfd pfd;
                pfd.fd = fd;
                pfd.events = POLLIN;
                pfd.revents = 0;
                poll_fds.push_back(pfd);
                fds_list.push_back(fd);
            }

            int ret = poll(poll_fds.data(), static_cast<nfds_t>(poll_fds.size()), 50);

            if (ret > 0) {
                for (size_t i = 0; i < poll_fds.size(); ++i) {
                    if (poll_fds[i].revents & POLLIN) {
                        socket_t fd = fds_list[i];
                        ReadCallback cb = nullptr;
                        {
                            std::lock_guard<std::mutex> lock(mutex_);
                            auto it = callbacks_.find(fd);
                            if (it != callbacks_.end()) cb = it->second;
                        }
                        if (cb) cb(fd);
                    }
                }
            }
            #endif
        }
    }

    void stop() {
        running_ = false;
        if (loop_thread_.joinable()) {
            loop_thread_.join();
        }
    }

    bool is_running() const {
        return running_;
    }

private:
    std::atomic<bool> running_;
    std::map<socket_t, ReadCallback> callbacks_;
    std::mutex mutex_;
    std::thread loop_thread_;
};

} // namespace garuda::beam::net
