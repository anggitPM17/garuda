#pragma once
// Garuda Production Infrastructure: Production Linux Daemon & Real Systemd Notifier (`daemon.hpp`)
// Mengirim sinyal asli ke Systemd socket ($NOTIFY_SOCKET: READY=1, WATCHDOG=1, STATUS=...)
// dan mengelola daemonization dengan PID file & fd cleanup.
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

#if !defined(_WIN32)
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#endif

namespace garuda::beam {

class Daemon {
public:
    // Kirim sinyal asli ke Systemd via UNIX domain socket ($NOTIFY_SOCKET)
    static bool sd_notify(const std::string& state) {
#if !defined(_WIN32)
        const char* socket_path = std::getenv("NOTIFY_SOCKET");
        if (!socket_path || std::strlen(socket_path) == 0) {
            std::cout << "⚙️ [GARUDA SYSTEMD] NOTIFY_SOCKET not set (running standalone or without Systemd Type=notify)." << std::endl;
            return false;
        }

        int fd = ::socket(AF_UNIX, SOCK_DGRAM | SOCK_CLOEXEC, 0);
        if (fd < 0) return false;

        struct sockaddr_un sockaddr{};
        sockaddr.sun_family = AF_UNIX;

        std::size_t path_len = std::strlen(socket_path);
        if (socket_path[0] == '@') {
            sockaddr.sun_path[0] = '\0';
            std::memcpy(sockaddr.sun_path + 1, socket_path + 1, std::min(path_len - 1, sizeof(sockaddr.sun_path) - 2));
        } else {
            std::strncpy(sockaddr.sun_path, socket_path, sizeof(sockaddr.sun_path) - 1);
        }

        socklen_t addr_len = static_cast<socklen_t>(offsetof(struct sockaddr_un, sun_path) + path_len);
        ssize_t sent = ::sendto(fd, state.c_str(), state.length(), 0, (struct sockaddr*)&sockaddr, addr_len);
        ::close(fd);

        if (sent > 0) {
            std::cout << "⚙️ [GARUDA SYSTEMD] Successfully sent signal to Systemd: '" << state << "'" << std::endl;
            return true;
        }
        return false;
#else
        std::cout << "⚙️ [GARUDA SYSTEMD] Systemd notifier skipped on Windows (state: " << state << ")" << std::endl;
        return true;
#endif
    }

    static bool notify_ready() {
        return sd_notify("READY=1\nSTATUS=Garuda GVM v67.0.0-production online and ready");
    }

    static bool notify_watchdog() {
        return sd_notify("WATCHDOG=1");
    }

    static bool daemonize(const std::string& pid_filepath = "/var/run/garuda.pid") {
#if !defined(_WIN32)
        pid_t pid = ::fork();
        if (pid < 0) return false;
        if (pid > 0) ::_exit(0); // Parent exits

        if (::setsid() < 0) return false;

        ::signal(SIGCHLD, SIG_IGN);

        pid = ::fork();
        if (pid < 0) return false;
        if (pid > 0) ::_exit(0);

        ::umask(0022);
        (void)!::chdir("/");

        // Write PID file
        std::ofstream pid_file(pid_filepath);
        if (pid_file.is_open()) {
            pid_file << ::getpid() << "\n";
            pid_file.close();
        }

        // Close standard descriptors
        int null_fd = ::open("/dev/null", O_RDWR);
        if (null_fd >= 0) {
            ::dup2(null_fd, STDIN_FILENO);
            ::dup2(null_fd, STDOUT_FILENO);
            ::dup2(null_fd, STDERR_FILENO);
            if (null_fd > STDERR_FILENO) ::close(null_fd);
        }
#else
        std::ofstream pid_file("garuda.pid");
        if (pid_file.is_open()) {
            pid_file << GetCurrentProcessId() << "\n";
            pid_file.close();
        }
#endif
        return true;
    }
};

} // namespace garuda::beam
