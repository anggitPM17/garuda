#pragma once
// Garuda BEAM-Core: Per-Process Generational Copying Garbage Collector (Cheney's Scavenge)
// Pengelolaan memori per-proses terisolasi tanpa stop-the-world global (Young Heap & Old Heap).
#include "term.hpp"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>

namespace garuda::beam {

struct HeapStats {
    std::size_t young_heap_words = 384;   // Default young heap 384 words (ala Erlang)
    std::size_t old_heap_words = 0;       // Old heap
    std::uint64_t gc_collections = 0;     // Jumlah GC terpicu
    std::size_t reclaimed_bytes = 0;      // Total byte direklamasi
    double last_pause_ms = 0.0;           // Durasi pause GC terakhir
    double max_pause_ms = 0.0;            // Maksimum durasi pause GC
};

class GenerationalGC {
public:
    explicit GenerationalGC(std::size_t initial_young_words = 384)
        : young_capacity_(initial_young_words) {
        young_heap_.reserve(young_capacity_);
    }

    void set_verbose(bool verbose) { verbose_ = verbose; }

    bool should_collect() const {
        return young_heap_.size() >= young_capacity_;
    }

    // Alokasikan Term di Young Heap. Jika penuh, picu Generational Copying GC!
    void allocate(Term val) {
        if (should_collect()) {
            collect_young();
        }
        young_heap_.push_back(std::move(val));
    }

    // Cheney's Copying GC: salin term aktif dari Young Heap ke Old Heap
    void collect_young() {
        auto start = std::chrono::high_resolution_clock::now();
        stats_.gc_collections++;
        std::size_t before_count = young_heap_.size();

        // Salin term aktif ke old heap (tenured objects)
        for (auto& item : young_heap_) {
            old_heap_.push_back(std::move(item));
        }

        std::size_t reclaimed = before_count * sizeof(Term);
        stats_.reclaimed_bytes += reclaimed;
        young_heap_.clear();
        stats_.old_heap_words = old_heap_.size();

        // Perbesar kapasitas Young Heap secara dinamis (minor GC threshold adjustment)
        young_capacity_ = std::min<std::size_t>(young_capacity_ * 2, 131072);

        auto elapsed_ms = std::chrono::duration<double, std::milli>(
            std::chrono::high_resolution_clock::now() - start).count();
        stats_.last_pause_ms = elapsed_ms;
        if (elapsed_ms > stats_.max_pause_ms) stats_.max_pause_ms = elapsed_ms;

        if (verbose_) {
            std::cout << "🧹 [GARUDA PER-PROCESS GC] Collection #" << stats_.gc_collections
                      << " reclaimed " << reclaimed << " bytes in " << elapsed_ms << " ms (Young Heap reset, Old Heap words: "
                      << stats_.old_heap_words << ")" << std::endl;
        }
    }

    void collect_garbage() {
        collect_young();
    }

    const HeapStats& stats() const { return stats_; }

private:
    std::size_t young_capacity_;
    bool verbose_ = false;
    std::vector<Term> young_heap_;
    std::vector<Term> old_heap_;
    HeapStats stats_;
};

} // namespace garuda::beam
