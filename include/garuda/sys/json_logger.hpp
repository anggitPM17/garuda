#pragma once
// =====================================================================
// GARUDA SYSTEM — STRUCTURED JSON LOGGER
// Single-line JSON logging for journalctl, jq, Filebeat, & Loki
// =====================================================================

#include <iostream>
#include <chrono>
#include <ctime>
#include <string>
#include <sstream>
#include <mutex>

namespace garuda::sys {

class JsonLogger {
private:
    static std::string escape_json(const std::string& input) {
        std::ostringstream ss;
        for (char c : input) {
            if (c == '"') ss << "\\\"";
            else if (c == '\\') ss << "\\\\";
            else if (c == '\n') ss << "\\n";
            else if (c == '\r') ss << "\\r";
            else if (c == '\t') ss << "\\t";
            else ss << c;
        }
        return ss.str();
    }

    static std::string current_time_iso() {
        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        char buf[32];
#ifdef _WIN32
        struct tm tm_buf;
        gmtime_s(&tm_buf, &t);
        std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &tm_buf);
#else
        struct tm tm_buf;
        gmtime_r(&t, &tm_buf);
        std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &tm_buf);
#endif
        return std::string(buf);
    }

    static void log(const std::string& level, const std::string& msg) {
        static std::mutex log_mutex;
        std::lock_guard<std::mutex> lock(log_mutex);
        std::cout << "{\"time\":\"" << current_time_iso()
                  << "\",\"level\":\"" << level
                  << "\",\"msg\":\"" << escape_json(msg) << "\"}"
                  << std::endl;
    }

public:
    static void info(const std::string& msg) { log("info", msg); }
    static void warn(const std::string& msg) { log("warn", msg); }
    static void error(const std::string& msg) { log("error", msg); }
};

} // namespace garuda::sys
