#pragma once
// Garuda Desktop Toolchain: System Process Execution Engine (`process.hpp`)
#include <cstdio>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <utility>

namespace garuda::beam {

class SystemProcess {
public:
    static std::pair<int, std::string> run(const std::string& command) {
        std::string full_cmd;
#if defined(_WIN32)
        full_cmd = "cmd /c \"" + command + " 2>&1\"";
        FILE* pipe = _popen(full_cmd.c_str(), "r");
#else
        full_cmd = command + " 2>&1";
        FILE* pipe = popen(full_cmd.c_str(), "r");
#endif

        if (!pipe) {
            return {-1, "Failed to open process pipe"};
        }

        char buffer[256];
        std::ostringstream oss;
        while (std::fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            oss << buffer;
        }

#if defined(_WIN32)
        int exit_code = _pclose(pipe);
#else
        int exit_code = pclose(pipe);
#endif

        return {exit_code, oss.str()};
    }
};

} // namespace garuda::beam
