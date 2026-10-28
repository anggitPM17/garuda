#pragma once
#include <string>

namespace garuda::autopilot {

struct LLMRequest {
    std::string error_log;
    std::string file_context;
    std::string language = "kotlin";
};

struct LLMResponse {
    bool success = false;
    std::string patch;
    std::string error;
    std::string model;
};

class LLMClient {
public:
    virtual ~LLMClient() = default;
    virtual LLMResponse generate_patch(const LLMRequest& request) = 0;
    virtual std::string name() const = 0;
};

} // namespace garuda::autopilot
