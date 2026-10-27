#pragma once
// Garuda BEAM Core: Memory Model & Arena Allocator (`memory.hpp`)
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>

namespace garuda::beam::memory {

struct Arena {
    std::uint8_t* buffer = nullptr;
    std::size_t capacity = 0;
    std::size_t offset = 0;
};

inline Arena* arena_create(std::size_t capacity) {
    if (capacity == 0) return nullptr;
    Arena* arena = static_cast<Arena*>(std::malloc(sizeof(Arena)));
    if (!arena) return nullptr;
    arena->buffer = static_cast<std::uint8_t*>(std::malloc(capacity));
    if (!arena->buffer) {
        std::free(arena);
        return nullptr;
    }
    arena->capacity = capacity;
    arena->offset = 0;
    return arena;
}

inline void* arena_alloc(Arena* arena, std::size_t size, std::size_t alignment = 8) {
    if (!arena || !arena->buffer || size == 0) return nullptr;

    std::uintptr_t current_addr = reinterpret_cast<std::uintptr_t>(arena->buffer + arena->offset);
    std::size_t align_mask = alignment - 1;
    std::uintptr_t aligned_addr = (current_addr + align_mask) & ~align_mask;
    std::size_t padding = aligned_addr - current_addr;

    if (arena->offset + padding + size > arena->capacity) {
        return nullptr; // Capacity exceeded
    }

    arena->offset += padding + size;
    return reinterpret_cast<void*>(aligned_addr);
}

inline void arena_reset(Arena* arena) {
    if (arena) {
        arena->offset = 0;
    }
}

inline void arena_destroy(Arena* arena) {
    if (arena) {
        if (arena->buffer) {
            std::free(arena->buffer);
            arena->buffer = nullptr;
        }
        std::free(arena);
    }
}

// --- Stub API Level Desain (Tahap 3 - Actor GC & Shared Binary Pool) ---
inline void* actor_alloc(void* /*process_ptr*/, std::size_t /*size*/) {
    return nullptr; // Stub: To be implemented in Phase 3 Actor GC Integration
}

inline void actor_gc_collect(void* /*process_ptr*/) {
    // Stub: To be implemented in Phase 3 Actor GC Integration
}

inline void* binary_alloc(std::size_t /*size*/) {
    return nullptr; // Stub: To be implemented in Phase 3 Shared Binary Pool
}

inline void binary_retain(void* /*binary_ptr*/) {
    // Stub
}

inline void binary_release(void* /*binary_ptr*/) {
    // Stub
}

} // namespace garuda::beam::memory
