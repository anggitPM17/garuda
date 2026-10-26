#pragma once
// Hierarchical timing wheel (5 level x 64 slot, resolusi 1 ms, jangkauan ~12 hari + overflow).
// Schedule/cancel O(1) amortized; next_expiry O(log n).
#include <algorithm>
#include <cstdint>
#include <functional>
#include <list>
#include <map>
#include <optional>
#include <unordered_map>
#include <array>

namespace garuda::beam {

using TimerId = std::uint64_t;

class TimerWheel {
public:
    static constexpr int kBits = 6;
    static constexpr int kSlots = 1 << kBits;
    static constexpr int kLevels = 5;

    TimerId schedule(std::uint64_t delay_ms, std::function<void()> fn) {
        TimerId id = next_id_++;
        Entry e{id, now_ + std::max<std::uint64_t>(delay_ms, 1), std::move(fn)};
        live_[id] = e.expire;
        ++expiries_[e.expire];
        place(std::move(e));
        return id;
    }
    TimerId after(std::uint64_t delay_ms, std::function<void()> fn) {
        return schedule(delay_ms, std::move(fn));
    }
    bool cancel(TimerId id) { return erase_live(id); }

    std::uint64_t now() const { return now_; }
    std::size_t pending() const { return live_.size(); }
    std::optional<std::uint64_t> next_expiry() const {
        if (expiries_.empty()) return std::nullopt;
        return expiries_.begin()->first;
    }

    void advance(std::uint64_t to) {
        while (now_ < to) {
            if (live_.empty()) { now_ = to; return; }
            tick();
        }
    }
    std::size_t advance_to(std::uint64_t to) {
        std::size_t count = 0;
        while (now_ < to) {
            if (live_.empty()) { now_ = to; break; }
            tick();
            count++;
        }
        return count;
    }

private:
    struct Entry { TimerId id; std::uint64_t expire; std::function<void()> fn; };

    bool erase_live(TimerId id) {
        auto it = live_.find(id);
        if (it == live_.end()) return false;
        auto ex = expiries_.find(it->second);
        if (ex != expiries_.end() && --ex->second == 0) expiries_.erase(ex);
        live_.erase(it);
        return true;
    }

    void place(Entry e) {
        std::uint64_t delta = e.expire > now_ ? e.expire - now_ : 0;
        for (int L = 0; L < kLevels; ++L) {
            if (delta < (std::uint64_t{1} << (kBits * (L + 1)))) {
                w_[L][(e.expire >> (kBits * L)) & (kSlots - 1)].push_back(std::move(e));
                return;
            }
        }
        overflow_.push_back(std::move(e));
    }

    void tick() {
        ++now_;
        for (int L = 1; L < kLevels; ++L) {
            if ((now_ & ((std::uint64_t{1} << (kBits * L)) - 1)) != 0) break;
            auto& slot = w_[L][(now_ >> (kBits * L)) & (kSlots - 1)];
            std::list<Entry> moved(std::move(slot));
            slot.clear();
            for (auto& e : moved) if (live_.count(e.id)) place(std::move(e));
        }
        if ((now_ & ((std::uint64_t{1} << (kBits * kLevels)) - 1)) == 0 && !overflow_.empty()) {
            std::list<Entry> moved(std::move(overflow_));
            overflow_.clear();
            for (auto& e : moved) if (live_.count(e.id)) place(std::move(e));
        }
        auto& slot = w_[0][now_ & (kSlots - 1)];
        std::list<Entry> due(std::move(slot));
        slot.clear();
        for (auto& e : due) {
            if (erase_live(e.id)) e.fn();  // fn boleh menjadwalkan timer baru
        }
    }

    std::array<std::array<std::list<Entry>, kSlots>, kLevels> w_;
    std::list<Entry> overflow_;
    std::unordered_map<TimerId, std::uint64_t> live_;
    std::map<std::uint64_t, std::size_t> expiries_;
    std::uint64_t now_ = 0;
    TimerId next_id_ = 1;
};

}  // namespace garuda::beam
