#pragma once
// Garuda Production Infrastructure: Resource Limits & Linux VPS Optimizer (`resource_limits.hpp`)
// Meningkatkan soft & hard limit file descriptor (RLIMIT_NOFILE ke 1.048.576) untuk menangani jutaan koneksi bersamaan di VPS.
#include <iostream>

#if !defined(_WIN32)
#include <sys/resource.h>
#endif

namespace garuda::beam {

class ResourceLimits {
public:
    static bool optimize_for_vps(std::size_t max_nofile = 1048576) {
#if !defined(_WIN32)
        struct rlimit rl;
        if (getrlimit(RLIMIT_NOFILE, &rl) == 0) {
            rl.rlim_max = std::max<rlim_t>(rl.rlim_max, max_nofile);
            rl.rlim_cur = rl.rlim_max;
            if (setrlimit(RLIMIT_NOFILE, &rl) == 0) {
                std::cout << "📈 [GARUDA VPS OPTIMIZER] File Descriptors (RLIMIT_NOFILE) raised to: " << rl.rlim_cur << std::endl;
                return true;
            }
        }
        std::cout << "⚠️ [GARUDA VPS OPTIMIZER] Could not set rlimit NOFILE (run as root or increase ulimit)" << std::endl;
        return false;
#else
        std::cout << "📈 [GARUDA VPS OPTIMIZER] Running on Windows environment (NOFILE limit managed by OS)" << std::endl;
        return true;
#endif
    }
};

} // namespace garuda::beam
