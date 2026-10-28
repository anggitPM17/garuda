#pragma once
#include "llm_client.hpp"
#include <atomic>
#include <stdexcept>
#include <utility>

namespace garuda::autopilot {

class MockLLMClient : public LLMClient {
public:
    enum class Behavior { SUCCESS, EMPTY_PATCH, FAILURE, THROW };
private:
    Behavior behavior_;
    std::string canned_patch_;
    std::string canned_error_;
    std::string name_;
    std::atomic<int> call_count_{0};

public:
    explicit MockLLMClient(Behavior behavior = Behavior::SUCCESS,
                           std::string canned_patch = "fun fixed() { return 42 }",
                           std::string canned_error = "mock failure",
                           std::string client_name = "MockLLM")
        : behavior_(behavior), canned_patch_(std::move(canned_patch)),
          canned_error_(std::move(canned_error)), name_(std::move(client_name)) {}

    LLMResponse generate_patch(const LLMRequest& request) override {
        call_count_.fetch_add(1);
        LLMResponse resp;
        resp.model = name_;
        switch (behavior_) {
            case Behavior::SUCCESS: resp.success = true; resp.patch = canned_patch_; break;
            case Behavior::EMPTY_PATCH: resp.success = true; resp.patch = ""; break;
            case Behavior::FAILURE: resp.success = false; resp.error = canned_error_; break;
            case Behavior::THROW: throw std::runtime_error("mock LLM threw");
        }
        (void)request;
        return resp;
    }
    std::string name() const override { return name_; }
    int call_count() const { return call_count_.load(); }
};

} // namespace garuda::autopilot
