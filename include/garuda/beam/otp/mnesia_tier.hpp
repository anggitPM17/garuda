#pragma once
// =====================================================================
// FASE G3.2 - GARUDA MNESIA TIER (mnesia_tier.hpp)
// Hot (RAM) / Warm (Disc) / Cold (Archive) Tiered Storage Engine
// =====================================================================
#include <string>
#include <map>
#include <vector>
#include <mutex>
#include <memory>
#include <optional>

namespace garuda::beam {

enum class Tier { HOT, WARM, COLD };

template<typename Key, typename Value>
class TieredStorage {
private:
    std::map<Key, Value> hot_;   // RAM
    std::map<Key, Value> warm_;  // Disc
    std::map<Key, Value> cold_;  // Archive
    mutable std::mutex mu_;

public:
    void write(const Key& k, const Value& v, Tier tier = Tier::HOT) {
        std::lock_guard<std::mutex> lock(mu_);
        if (tier == Tier::HOT) {
            hot_[k] = v;
        } else if (tier == Tier::WARM) {
            warm_[k] = v;
        } else {
            cold_[k] = v;
        }
    }

    std::optional<Value> read(const Key& k) {
        std::lock_guard<std::mutex> lock(mu_);
        auto it_h = hot_.find(k);
        if (it_h != hot_.end()) return it_h->second;

        auto it_w = warm_.find(k);
        if (it_w != warm_.end()) return it_w->second;

        auto it_c = cold_.find(k);
        if (it_c != cold_.end()) return it_c->second;

        return std::nullopt;
    }

    bool demote_to_warm(const Key& k) {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = hot_.find(k);
        if (it == hot_.end()) return false;
        warm_[k] = it->second;
        hot_.erase(it);
        return true;
    }

    bool demote_to_cold(const Key& k) {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = warm_.find(k);
        if (it == warm_.end()) return false;
        cold_[k] = it->second;
        warm_.erase(it);
        return true;
    }

    bool promote_to_hot(const Key& k) {
        std::lock_guard<std::mutex> lock(mu_);
        auto it_w = warm_.find(k);
        if (it_w != warm_.end()) {
            hot_[k] = it_w->second;
            warm_.erase(it_w);
            return true;
        }
        auto it_c = cold_.find(k);
        if (it_c != cold_.end()) {
            hot_[k] = it_c->second;
            cold_.erase(it_c);
            return true;
        }
        return false;
    }

    std::size_t hot_count() const {
        std::lock_guard<std::mutex> lock(mu_);
        return hot_.size();
    }

    std::size_t warm_count() const {
        std::lock_guard<std::mutex> lock(mu_);
        return warm_.size();
    }

    std::size_t cold_count() const {
        std::lock_guard<std::mutex> lock(mu_);
        return cold_.size();
    }

    std::optional<Tier> get_tier(const Key& k) const {
        std::lock_guard<std::mutex> lock(mu_);
        if (hot_.find(k) != hot_.end()) return Tier::HOT;
        if (warm_.find(k) != warm_.end()) return Tier::WARM;
        if (cold_.find(k) != cold_.end()) return Tier::COLD;
        return std::nullopt;
    }
};

} // namespace garuda::beam
