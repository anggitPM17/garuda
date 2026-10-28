#pragma once
// =====================================================================
// FASE G8.5 - RICHAT Cold Database (setara richat_cold_db.erl)
// =====================================================================
#include "../beam/otp/mnesia.hpp"
#include "../beam/otp/mnesia_tier.hpp"

#include <string>
#include <vector>
#include <mutex>
#include <optional>
#include <chrono>
#include <cstddef>

namespace garuda::richat {

struct ColdArchiveResult {
    std::string table;
    std::size_t archived_count = 0;
    std::size_t failed_count = 0;
    std::chrono::microseconds duration{0};
};

class RichatColdDb {
private:
    garuda::beam::Mnesia* mnesia_;
    garuda::beam::TieredStorage<std::string, std::string> tier_storage_;
    mutable std::mutex mu_;

public:
    explicit RichatColdDb(garuda::beam::Mnesia* mnesia)
        : mnesia_(mnesia) {}

    ColdArchiveResult archive(const std::string& table, std::chrono::seconds /*older_than*/) {
        auto t0 = std::chrono::steady_clock::now();
        ColdArchiveResult res;
        res.table = table;

        std::lock_guard<std::mutex> lock(mu_);
        if (mnesia_) {
            auto rows = mnesia_->select(table, [](const garuda::beam::Term&, const garuda::beam::Term&) {
                return true;
            });

            for (std::size_t i = 0; i < rows.size(); ++i) {
                std::string k = "arc_" + table + "_" + std::to_string(i);
                std::string v = garuda::beam::to_string(rows[i]);
                tier_storage_.write(k, v, garuda::beam::Tier::COLD);
                res.archived_count++;
            }
        }

        auto t1 = std::chrono::steady_clock::now();
        res.duration = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0);
        return res;
    }

    bool archive_record(const std::string& table, const std::string& key, const std::string& value) {
        std::lock_guard<std::mutex> lock(mu_);
        std::string full_key = table + ":" + key;
        tier_storage_.write(full_key, value, garuda::beam::Tier::COLD);
        return true;
    }

    bool restore(const std::string& table, const std::string& key) {
        std::lock_guard<std::mutex> lock(mu_);
        std::string full_key = table + ":" + key;
        auto val = tier_storage_.read(full_key);
        if (val.has_value()) {
            tier_storage_.promote_to_hot(full_key);
            if (mnesia_) {
                mnesia_->write(table, garuda::beam::Term(key), garuda::beam::Term(*val));
            }
            return true;
        }
        return false;
    }

    std::size_t cold_count() const {
        std::lock_guard<std::mutex> lock(mu_);
        return tier_storage_.cold_count();
    }
};

} // namespace garuda::richat
