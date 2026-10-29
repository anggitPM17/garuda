#pragma once
// =====================================================================
// FASE G8.3 - RICHAT Group Fanout (setara richat_group_fanout.erl)
// =====================================================================
#include "richat_group_member.hpp"
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <optional>
#include <chrono>
#include <functional>
#include <cstdint>

namespace garuda::richat {

struct FanoutResult {
    std::string message_id;
    std::string group_id;
    std::size_t target_count = 0;
    std::size_t delivered_count = 0;
    std::size_t failed_count = 0;
    std::chrono::microseconds duration{0};
};

using DeliveryCallback = std::function<bool(const std::string& user_id, const std::string& message_id)>;

class RichatGroupFanout {
private:
    RichatGroupMember* member_store_;
    DeliveryCallback delivery_fn_;
    mutable std::mutex mu_;

public:
    RichatGroupFanout(RichatGroupMember* member_store, DeliveryCallback delivery_fn)
        : member_store_(member_store), delivery_fn_(std::move(delivery_fn)) {}

    FanoutResult fanout(const std::string& group_id,
                        const std::string& sender_id,
                        const std::string& message_id) {
        auto t0 = std::chrono::steady_clock::now();
        FanoutResult res;
        res.group_id = group_id;
        res.message_id = message_id;

        if (!member_store_ || !delivery_fn_) {
            auto t1 = std::chrono::steady_clock::now();
            res.duration = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0);
            return res;
        }

        auto members = member_store_->list_members(group_id);
        std::lock_guard<std::mutex> lock(mu_);

        for (const auto& member : members) {
            if (member.user_id == sender_id) continue; // Exclude sender
            res.target_count++;

            bool ok = delivery_fn_(member.user_id, message_id);
            if (ok) {
                res.delivered_count++;
            } else {
                res.failed_count++;
            }
        }

        auto t1 = std::chrono::steady_clock::now();
        res.duration = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0);
        return res;
    }

    std::vector<FanoutResult> batch_fanout(const std::string& group_id,
                                           const std::string& sender_id,
                                           const std::vector<std::string>& message_ids) {
        std::vector<FanoutResult> results;
        for (const auto& msg_id : message_ids) {
            results.push_back(fanout(group_id, sender_id, msg_id));
        }
        return results;
    }
};

} // namespace garuda::richat
