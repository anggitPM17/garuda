#ifndef GARUDA_BEAM_SYS_SANDBOX_HPP
#define GARUDA_BEAM_SYS_SANDBOX_HPP

#include <string>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <cstdint>
#include <iostream>

namespace garuda {
namespace beam {
namespace sys {

class SecurityException : public std::runtime_error {
public:
    explicit SecurityException(const std::string& msg) : std::runtime_error(msg) {}
};

struct SandboxPolicy {
    bool allow_fs_read = true;
    bool allow_fs_write = false;
    bool allow_network = false;
    bool allow_exec = false;
    size_t max_memory_bytes = 64 * 1024 * 1024; // 64 MB
    uint64_t max_execution_time_ms = 5000;      // 5000 ms
    std::vector<std::string> allowed_paths;

    static SandboxPolicy Permissive() {
        SandboxPolicy p;
        p.allow_fs_read = true;
        p.allow_fs_write = true;
        p.allow_network = true;
        p.allow_exec = true;
        p.max_memory_bytes = SIZE_MAX;
        p.max_execution_time_ms = UINT64_MAX;
        return p;
    }

    static SandboxPolicy StrictlyIsolated() {
        SandboxPolicy p;
        p.allow_fs_read = false;
        p.allow_fs_write = false;
        p.allow_network = false;
        p.allow_exec = false;
        p.max_memory_bytes = 16 * 1024 * 1024; // 16 MB
        p.max_execution_time_ms = 1000;
        return p;
    }
};

class GvmSandbox {
private:
    static inline thread_local SandboxPolicy current_policy = SandboxPolicy::Permissive();

public:
    static void set_thread_policy(const SandboxPolicy& policy) {
        current_policy = policy;
    }

    static const SandboxPolicy& get_thread_policy() {
        return current_policy;
    }

    static void reset_policy() {
        current_policy = SandboxPolicy::Permissive();
    }

    static bool check_fs_read(const std::string& path) {
        if (!current_policy.allow_fs_read) {
            bool path_allowed = false;
            for (const auto& allowed : current_policy.allowed_paths) {
                if (path.rfind(allowed, 0) == 0) { // starts_with
                    path_allowed = true;
                    break;
                }
            }
            if (!path_allowed) {
                throw SecurityException("[GvmSandbox] Access Denied: Filesystem read blocked for path: " + path);
            }
        }
        return true;
    }

    static bool check_fs_write(const std::string& path) {
        if (!current_policy.allow_fs_write) {
            bool path_allowed = false;
            for (const auto& allowed : current_policy.allowed_paths) {
                if (path.rfind(allowed, 0) == 0) {
                    path_allowed = true;
                    break;
                }
            }
            if (!path_allowed) {
                throw SecurityException("[GvmSandbox] Access Denied: Filesystem write blocked for path: " + path);
            }
        }
        return true;
    }

    static bool check_network(const std::string& host, int port) {
        if (!current_policy.allow_network) {
            throw SecurityException("[GvmSandbox] Access Denied: Network socket connection blocked for " + host + ":" + std::to_string(port));
        }
        return true;
    }

    static bool check_exec(const std::string& command) {
        if (!current_policy.allow_exec) {
            throw SecurityException("[GvmSandbox] Access Denied: Subprocess command execution blocked for: " + command);
        }
        return true;
    }
};

} // namespace sys
} // namespace beam
} // namespace garuda

#endif // GARUDA_BEAM_SYS_SANDBOX_HPP
