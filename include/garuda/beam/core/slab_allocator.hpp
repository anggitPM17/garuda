#pragma once
// Garuda Million-Scale Engine: Slab & Arena Memory Allocator
// Menghilangkan overhead malloc/new heap fragmentation saat ribuan/jutaan proses dan pesan di-spawn/dihapus.
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <mutex>
#include <vector>

namespace garuda::beam {

template <typename T, std::size_t BlockSize = 4096>
class SlabAllocator {
public:
    SlabAllocator() = default;
    ~SlabAllocator() {
        for (void* ptr : chunks_) {
            ::operator delete(ptr);
        }
    }

    T* allocate() {
        std::lock_guard<std::mutex> lock(mu_);
        if (free_list_ == nullptr) {
            allocate_new_chunk();
        }
        Node* head = free_list_;
        free_list_ = free_list_->next;
        allocated_count_++;
        return reinterpret_cast<T*>(head);
    }

    void deallocate(T* ptr) {
        if (!ptr) return;
        std::lock_guard<std::mutex> lock(mu_);
        Node* node = reinterpret_cast<Node*>(ptr);
        node->next = free_list_;
        free_list_ = node;
        allocated_count_--;
    }

    std::size_t active_allocations() const { return allocated_count_; }

private:
    union Node {
        Node* next;
        typename std::aligned_storage<sizeof(T), alignof(T)>::type storage;
    };

    void allocate_new_chunk() {
        std::size_t num_nodes = BlockSize;
        Node* chunk = static_cast<Node*>(::operator new(sizeof(Node) * num_nodes));
        chunks_.push_back(chunk);
        for (std::size_t i = 0; i < num_nodes - 1; ++i) {
            chunk[i].next = &chunk[i + 1];
        }
        chunk[num_nodes - 1].next = nullptr;
        free_list_ = chunk;
    }

    Node* free_list_ = nullptr;
    std::vector<void*> chunks_;
    std::size_t allocated_count_ = 0;
    std::mutex mu_;
};

} // namespace garuda::beam
