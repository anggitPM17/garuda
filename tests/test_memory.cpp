#include "../include/garuda/beam/core/memory.hpp"
#include <chrono>
#include <future>
#include <iostream>

using namespace garuda::beam::memory;

#define GARUDA_CHECK(cond, msg) \
    if (!(cond)) { \
        std::cerr << "  ❌ ASSERTION FAILED: " << msg << " (" << #cond << ")\n"; \
        std::exit(1); \
    }

void run_test_with_timeout(const std::string& test_name, std::function<void()> test_fn, int timeout_seconds = 5) {
    auto future = std::async(std::launch::async, test_fn);
    if (future.wait_for(std::chrono::seconds(timeout_seconds)) == std::future_status::timeout) {
        std::cerr << "  ❌ " << test_name << " FAILED (TIMEOUT > " << timeout_seconds << "s)\n";
        std::exit(1);
    }
}

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA BEAM CORE — ARENA MEMORY ALLOCATOR SUITE\n";
    std::cout << "=======================================================\n\n";

    // === Test 1: Arena Creation, 8-Byte Alignment & Read/Write ===
    run_test_with_timeout("Test 1: Arena Creation & 8-Byte Alignment", []() {
        Arena* arena = arena_create(1024);
        GARUDA_CHECK(arena != nullptr, "arena_create must not return nullptr");
        GARUDA_CHECK(arena->capacity == 1024, "arena capacity must be 1024");
        GARUDA_CHECK(arena->offset == 0, "initial offset must be 0");

        void* p1 = arena_alloc(arena, 13);
        GARUDA_CHECK(p1 != nullptr, "p1 allocation must succeed");
        GARUDA_CHECK(reinterpret_cast<std::uintptr_t>(p1) % 8 == 0, "p1 pointer must be 8-byte aligned");

        void* p2 = arena_alloc(arena, 21);
        GARUDA_CHECK(p2 != nullptr, "p2 allocation must succeed");
        GARUDA_CHECK(reinterpret_cast<std::uintptr_t>(p2) % 8 == 0, "p2 pointer must be 8-byte aligned");

        // Read/Write verification
        std::uint64_t* num_ptr = static_cast<std::uint64_t*>(p1);
        *num_ptr = 0xDEADBEEF12345678ULL;
        GARUDA_CHECK(*num_ptr == 0xDEADBEEF12345678ULL, "Written memory value must match");

        arena_destroy(arena);
        std::cout << "  ✓ Test 1: Arena Creation & 8-Byte Alignment PASS\n";
    }, 5);

    // === Test 2: Arena Reset (Offset Reset & Memory Reuse) ===
    run_test_with_timeout("Test 2: Arena Reset", []() {
        Arena* arena = arena_create(512);
        void* p1 = arena_alloc(arena, 100);
        GARUDA_CHECK(p1 != nullptr, "p1 must not be nullptr");
        GARUDA_CHECK(arena->offset >= 100, "offset must advance");

        arena_reset(arena);
        GARUDA_CHECK(arena->offset == 0, "arena_reset must reset offset to 0");

        void* p2 = arena_alloc(arena, 100);
        GARUDA_CHECK(p2 != nullptr, "p2 allocation after reset must succeed");
        GARUDA_CHECK(p1 == p2, "p2 must reuse the same base address after reset");

        arena_destroy(arena);
        std::cout << "  ✓ Test 2: Arena Reset PASS\n";
    }, 5);

    // === Test 3: Sequential Allocation until Full (Returns nullptr when exceeded) ===
    run_test_with_timeout("Test 3: Capacity Overflow Handling", []() {
        std::size_t cap = 256;
        Arena* arena = arena_create(cap);
        GARUDA_CHECK(arena != nullptr, "arena creation must succeed");

        // Fill arena using 64-byte chunks
        void* p1 = arena_alloc(arena, 64);
        void* p2 = arena_alloc(arena, 64);
        void* p3 = arena_alloc(arena, 64);
        GARUDA_CHECK(p1 != nullptr && p2 != nullptr && p3 != nullptr, "First 3 allocations must succeed");

        // Attempt allocation that exceeds remaining capacity
        void* overflow_ptr = arena_alloc(arena, 200);
        GARUDA_CHECK(overflow_ptr == nullptr, "Exceeding allocation must return nullptr without crashing");

        arena_destroy(arena);
        std::cout << "  ✓ Test 3: Capacity Overflow Handling PASS\n";
    }, 5);

    // === Test 4: Stub API Validation ===
    run_test_with_timeout("Test 4: Stub API Validation", []() {
        void* a_ptr = actor_alloc(nullptr, 64);
        GARUDA_CHECK(a_ptr == nullptr, "actor_alloc stub must return nullptr");

        actor_gc_collect(nullptr); // Must not crash

        void* b_ptr = binary_alloc(128);
        GARUDA_CHECK(b_ptr == nullptr, "binary_alloc stub must return nullptr");

        binary_retain(nullptr); // Must not crash
        binary_release(nullptr); // Must not crash

        std::cout << "  ✓ Test 4: Stub API Validation PASS\n";
    }, 5);

    std::cout << "\n🎉 ALL ARENA MEMORY ALLOCATOR TESTS PASSED PERFECTLY!\n";
    return 0;
}
