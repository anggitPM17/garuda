#pragma once
// Garuda Production Infrastructure: Async-Signal-Safe Signal Handler (`signal_handler.hpp`)
// 100% Async-Signal-Safe (Self-Pipe Trick & Atomic sig_atomic_t) tanpa deadlock / undefined behavior.
#include <atomic>
#include <csignal>
#include <functional>
#include <iostream>

#if !defined(_WIN32)
#include <fcntl.h>
#include <unistd.h>
#endif

namespace garuda::beam {

static volatile std::sig_atomic_t g_last_signal = 0;
static volatile std::sig_atomic_t g_shutdown_requested = 0;

class SignalHandler {
public:
    static void setup_signals(std::function<void(int)> on_shutdown) {
        shutdown_callback() = std::move(on_shutdown);

#if !defined(_WIN32)
        if (::pipe(self_pipe()) == 0) {
            ::fcntl(self_pipe()[0], F_SETFL, ::fcntl(self_pipe()[0], F_GETFL, 0) | O_NONBLOCK);
            ::fcntl(self_pipe()[1], F_SETFL, ::fcntl(self_pipe()[1], F_GETFL, 0) | O_NONBLOCK);
        }

        struct sigaction sa{};
        sa.sa_handler = SignalHandler::async_signal_handler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;

        ::sigaction(SIGINT, &sa, nullptr);
        ::sigaction(SIGTERM, &sa, nullptr);
        ::signal(SIGHUP, SIG_IGN);
        ::signal(SIGPIPE, SIG_IGN);
#else
        std::signal(SIGINT, SignalHandler::async_signal_handler);
        std::signal(SIGTERM, SignalHandler::async_signal_handler);
#endif
        std::cout << "🛡️ [GARUDA SIGNALS] Async-Signal-Safe handlers registered for SIGINT & SIGTERM." << std::endl;
    }

    static void check_and_dispatch() {
        if (g_shutdown_requested) {
            int sig = g_last_signal;
            g_shutdown_requested = 0;
            std::cout << "\n🛑 [GARUDA SHUTDOWN] Signal " << sig << " received safely! Executing graceful shutdown..." << std::endl;
            if (shutdown_callback()) {
                shutdown_callback()(sig);
            }
        }
    }

    static bool is_shutdown_requested() {
        return g_shutdown_requested != 0;
    }

private:
    static int* self_pipe() {
        static int p[2] = {-1, -1};
        return p;
    }

    static std::function<void(int)>& shutdown_callback() {
        static std::function<void(int)> cb;
        return cb;
    }

    static void async_signal_handler(int sig) {
        g_last_signal = sig;
        g_shutdown_requested = 1;
#if !defined(_WIN32)
        if (self_pipe()[1] >= 0) {
            char c = static_cast<char>(sig);
            (void)!::write(self_pipe()[1], &c, 1);
        }
#endif
    }
};

} // namespace garuda::beam
