#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <functional>
#include <mutex>
#include <atomic>
#include <thread>
#include <chrono>
#include <memory>
#include <algorithm>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
typedef SOCKET socket_t;
#define INVALID_SOCKET_VAL INVALID_SOCKET
#define CLOSE_SOCKET(s) closesocket(s)
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
typedef int socket_t;
#define INVALID_SOCKET_VAL -1
#define CLOSE_SOCKET(s) close(s)
#endif

namespace garuda::beam::net {

constexpr int EV_READ  = 1 << 0;
constexpr int EV_WRITE = 1 << 1;
constexpr int EV_ERROR = 1 << 2;

struct Event { socket_t fd; int events; };

class EventLoopBackend {
public:
    virtual ~EventLoopBackend() = default;
    virtual bool        init() { return false; }
    virtual int         wait(Event*, int, int) { return -1; }
    virtual bool        add(int) { return false; }
    virtual bool        remove(int) { return false; }
    virtual const char* name() const { return "stub"; }
};

class PollBackend : public EventLoopBackend {};

inline std::unique_ptr<EventLoopBackend> make_default_backend() {
    return nullptr; // STUB RED: return null to make Test 1 fail
}

class EventLoop {
public:
    using ReadCallback = std::function<void(socket_t)>;

    EventLoop() : running_(false) {}
    ~EventLoop() { stop(); }

    const char* backend_name() const { return "stub"; }

    void add_socket(socket_t, ReadCallback) {}
    void remove_socket(socket_t) {}
    void start_in_thread() {}
    void run() {}
    void stop() {}
    bool is_running() const { return false; }

private:
    std::atomic<bool> running_;
};

} // namespace garuda::beam::net
