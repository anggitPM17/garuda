#pragma once
// =====================================================================
// FASE G5.4 - Read Receipt: Delivery & read receipt tracking
// =====================================================================
#include <string>
#include <map>
#include <vector>
#include <mutex>
#include <optional>
#include <chrono>
#include <cstddef>

namespace garuda::beam {

enum class ReceiptStatus { SENT, DELIVERED, READ };

struct ReceiptRecord {
    std::string message_id;
    std::string recipient_id;
    ReceiptStatus status = ReceiptStatus::SENT;
    std::chrono::steady_clock::time_point updated_at;
};

class ReadReceipt {
private:
    std::map<std::string, ReceiptRecord> receipts_;  // key: message_id:recipient_id
    mutable std::mutex mu_;

    static std::string make_key(const std::string& msg_id, const std::string& user_id) {
        return msg_id + ":" + user_id;
    }

public:
    void mark_delivered(const std::string& message_id, const std::string& recipient_id) {
        std::lock_guard<std::mutex> lock(mu_);
        std::string key = make_key(message_id, recipient_id);
        ReceiptRecord rec;
        rec.message_id = message_id;
        rec.recipient_id = recipient_id;
        rec.status = ReceiptStatus::DELIVERED;
        rec.updated_at = std::chrono::steady_clock::now();
        receipts_[key] = rec;
    }

    void mark_read(const std::string& message_id, const std::string& recipient_id) {
        std::lock_guard<std::mutex> lock(mu_);
        std::string key = make_key(message_id, recipient_id);
        ReceiptRecord rec;
        rec.message_id = message_id;
        rec.recipient_id = recipient_id;
        rec.status = ReceiptStatus::READ;
        rec.updated_at = std::chrono::steady_clock::now();
        receipts_[key] = rec;
    }

    std::optional<ReceiptRecord> get_receipt(const std::string& message_id, const std::string& recipient_id) const {
        std::lock_guard<std::mutex> lock(mu_);
        std::string key = make_key(message_id, recipient_id);
        auto it = receipts_.find(key);
        if (it != receipts_.end()) return it->second;
        return std::nullopt;
    }

    std::size_t count_by_status(ReceiptStatus status) const {
        std::lock_guard<std::mutex> lock(mu_);
        std::size_t count = 0;
        for (const auto& [k, rec] : receipts_) {
            if (rec.status == status) count++;
        }
        return count;
    }
};

} // namespace garuda::beam
