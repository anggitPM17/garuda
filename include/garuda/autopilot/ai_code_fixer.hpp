#pragma once
#include "llm_client.hpp"
#include <string>
#include <mutex>
#include <cstddef>

namespace garuda::autopilot {

struct FixResult {
    bool success = false;
    std::string file;
    std::string patch;
    std::string error;
    std::string llm_model;
};

class AICodeFixer {
public:
    static constexpr std::size_t MAX_PATCH_SIZE = 64 * 1024;
    static constexpr std::size_t MAX_ERROR_LOG_SIZE = 16 * 1024;
private:
    LLMClient& llm_;
    std::mutex mutex_;
public:
    explicit AICodeFixer(LLMClient& llm) : llm_(llm) {}

    FixResult fix(const std::string& error_log, const std::string& file_context, const std::string& target_file = "") {
        std::lock_guard<std::mutex> lock(mutex_);
        FixResult result;
        result.file = target_file;

        if (error_log.empty()) {
            result.error = "empty error_log";
            return result;
        }
        if (error_log.size() > MAX_ERROR_LOG_SIZE) {
            result.error = "error_log too large";
            return result;
        }

        LLMRequest req;
        req.error_log = error_log;
        req.file_context = file_context;
        req.language = "kotlin";

        LLMResponse resp;
        try {
            resp = llm_.generate_patch(req);
        } catch (const std::exception& e) {
            result.error = std::string("LLM threw exception: ") + e.what();
            return result;
        } catch (...) {
            result.error = "LLM threw unknown exception";
            return result;
        }

        result.llm_model = resp.model;
        if (!resp.success) {
            result.error = resp.error.empty() ? "LLM returned failure" : resp.error;
            return result;
        }
        if (resp.patch.empty()) {
            result.error = "LLM returned empty patch";
            return result;
        }
        if (resp.patch.size() > MAX_PATCH_SIZE) {
            result.error = "patch too large (>64KB)";
            return result;
        }

        result.success = true;
        result.patch = resp.patch;
        return result;
    }
};

} // namespace garuda::autopilot
