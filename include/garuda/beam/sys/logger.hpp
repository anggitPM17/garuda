#pragma once
// Garuda Production Infrastructure: Thread-Safe Production Logger (`logger.hpp`)
// Logger produksi terstruktur untuk VPS Linux (/var/log/garuda/garuda.log)
// Menggunakan localtime_r/localtime_s thread-safe, log level filtering, dan format JSON.
#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>

namespace garuda::beam {

enum class LogLevel { Debug = 0, Info = 1, Warning = 2, Error = 3, Critical = 4 };

class StructuredLogger {
public:
    static std::string format_json_log(const std::string& level, const std::string& msg, const std::string& module = "garuda") {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);

        struct tm tm_buf{};
#if defined(_WIN32)
        gmtime_s(&tm_buf, &in_time_t);
#else
        gmtime_r(&in_time_t, &tm_buf);
#endif

        std::ostringstream ss;
        ss << "{\"timestamp\":\"" << std::put_time(&tm_buf, "%Y-%m-%dT%H:%M:%SZ") << "\","
           << "\"level\":\"" << level << "\","
           << "\"module\":\"" << module << "\","
           << "\"msg\":\"" << escape_json_str(msg) << "\"}";
        return ss.str();
    }

    static void info(const std::string& msg, const std::string& module = "garuda", std::ostream& os = std::cout) {
        os << format_json_log("INFO", msg, module) << "\n";
    }

    static void warn(const std::string& msg, const std::string& module = "garuda", std::ostream& os = std::cout) {
        os << format_json_log("WARN", msg, module) << "\n";
    }

    static void error(const std::string& msg, const std::string& module = "garuda", std::ostream& os = std::cout) {
        os << format_json_log("ERROR", msg, module) << "\n";
    }

    static void debug(const std::string& msg, const std::string& module = "garuda", std::ostream& os = std::cout) {
        os << format_json_log("DEBUG", msg, module) << "\n";
    }

private:
    static std::string escape_json_str(const std::string& s) {
        std::string res;
        res.reserve(s.length() + 8);
        for (char c : s) {
            if (c == '"') res += "\\\"";
            else if (c == '\\') res += "\\\\";
            else if (c == '\n') res += "\\n";
            else if (c == '\r') res += "\\r";
            else if (c == '\t') res += "\\t";
            else res += c;
        }
        return res;
    }
};

class Logger {
public:
    static Logger& instance() {
        static Logger inst;
        return inst;
    }

    void set_log_file(const std::string& filepath) {
        std::lock_guard<std::mutex> lock(mu_);
        auto parent = std::filesystem::path(filepath).parent_path();
        if (!parent.empty()) {
            std::filesystem::create_directories(parent);
        }
        if (log_stream_.is_open()) log_stream_.close();
        log_stream_.open(filepath, std::ios::app);
    }

    void set_min_level(LogLevel min_lvl) {
        std::lock_guard<std::mutex> lock(mu_);
        min_level_ = min_lvl;
    }

    void log(LogLevel level, const std::string& module_name, const std::string& msg, bool is_json = false) {
        if (level < min_level_) return;

        std::lock_guard<std::mutex> lock(mu_);
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);

        struct tm tm_buf{};
#if defined(_WIN32)
        localtime_s(&tm_buf, &in_time_t);
#else
        localtime_r(&in_time_t, &tm_buf);
#endif

        std::ostringstream ss;
        if (is_json) {
            ss << "{\"timestamp\":\"" << std::put_time(&tm_buf, "%Y-%m-%dT%H:%M:%SZ") << "\","
               << "\"level\":\"" << level_to_string(level) << "\","
               << "\"module\":\"" << module_name << "\","
               << "\"message\":\"" << msg << "\"}\n";
        } else {
            ss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S") << " ["
               << level_to_string(level) << "] [" << module_name << "] " << msg << "\n";
        }

        std::string formatted = ss.str();
        std::cout << formatted;
        if (log_stream_.is_open()) {
            log_stream_ << formatted;
            log_stream_.flush();
        }
    }

private:
    Logger() = default;
    ~Logger() {
        if (log_stream_.is_open()) log_stream_.close();
    }

    std::string level_to_string(LogLevel lvl) {
        switch (lvl) {
            case LogLevel::Debug: return "DEBUG";
            case LogLevel::Info: return "INFO";
            case LogLevel::Warning: return "WARN";
            case LogLevel::Error: return "ERROR";
            case LogLevel::Critical: return "CRIT";
        }
        return "INFO";
    }

    std::mutex mu_;
    std::ofstream log_stream_;
    LogLevel min_level_ = LogLevel::Info;
};

#define GARUDA_LOG_INFO(mod, msg) ::garuda::beam::Logger::instance().log(::garuda::beam::LogLevel::Info, mod, msg)
#define GARUDA_LOG_WARN(mod, msg) ::garuda::beam::Logger::instance().log(::garuda::beam::LogLevel::Warning, mod, msg)
#define GARUDA_LOG_ERROR(mod, msg) ::garuda::beam::Logger::instance().log(::garuda::beam::LogLevel::Error, mod, msg)

} // namespace garuda::beam
