#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>
#include <utility>
#include <concepts>
#include <new>
#include <algorithm>

#include "execore/common/concepts.hpp"

namespace execore {

/**
 * @brief High-performance region/arena bump allocator for AST nodes and compiler passes.
 *
 * Provides cache-localized, contiguous allocations with amortized O(1) allocation time
 * and O(1) bulk deallocation per compilation unit.
 */
class ArenaAllocator {
public:
    explicit ArenaAllocator(size_t chunk_size = 64 * 1024) noexcept
        : chunk_size_(chunk_size) {}

    ~ArenaAllocator() {
        reset();
    }

    ArenaAllocator(const ArenaAllocator&) = delete;
    ArenaAllocator& operator=(const ArenaAllocator&) = delete;
    ArenaAllocator(ArenaAllocator&& other) noexcept = default;
    ArenaAllocator& operator=(ArenaAllocator&& other) noexcept = default;

    [[nodiscard]] void* allocate(size_t bytes, size_t alignment = alignof(std::max_align_t)) {
        if (bytes == 0) return nullptr;

        size_t current_addr = reinterpret_cast<size_t>(current_ptr_);
        size_t aligned_addr = (current_addr + alignment - 1) & ~(alignment - 1);
        size_t padding = aligned_addr - current_addr;

        if (current_ptr_ && current_ptr_ + padding + bytes <= chunk_end_) {
            current_ptr_ += padding + bytes;
            allocated_bytes_ += padding + bytes;
            return reinterpret_cast<void*>(aligned_addr);
        }

        size_t new_chunk_size = std::max(chunk_size_, bytes + alignment);
        auto chunk = std::make_unique<std::byte[]>(new_chunk_size);
        current_ptr_ = chunk.get();
        chunk_end_ = current_ptr_ + new_chunk_size;
        chunks_.push_back(std::move(chunk));

        current_addr = reinterpret_cast<size_t>(current_ptr_);
        aligned_addr = (current_addr + alignment - 1) & ~(alignment - 1);
        padding = aligned_addr - current_addr;

        current_ptr_ += padding + bytes;
        allocated_bytes_ += padding + bytes;
        return reinterpret_cast<void*>(aligned_addr);
    }

    template <typename T, typename... Args>
    requires Allocatable<T> && std::constructible_from<T, Args...>
    [[nodiscard]] T* create(Args&&... args) {
        void* mem = allocate(sizeof(T), alignof(T));
        T* obj = ::new (mem) T(std::forward<Args>(args)...);
        if constexpr (!std::is_trivially_destructible_v<T>) {
            destructors_.push_back([](void* ptr) {
                static_cast<T*>(ptr)->~T();
            });
            destructor_ptrs_.push_back(mem);
        }
        return obj;
    }

    void reset() noexcept {
        // Run destructors in reverse allocation order so dependent child nodes destruct before parents
        for (size_t i = destructors_.size(); i > 0; --i) {
            destructors_[i - 1](destructor_ptrs_[i - 1]);
        }
        destructors_.clear();
        destructor_ptrs_.clear();
        chunks_.clear();
        current_ptr_ = nullptr;
        chunk_end_ = nullptr;
        allocated_bytes_ = 0;
    }

    [[nodiscard]] size_t total_allocated() const noexcept { return allocated_bytes_; }
    [[nodiscard]] size_t chunk_count() const noexcept { return chunks_.size(); }

private:
    size_t chunk_size_;
    std::byte* current_ptr_{nullptr};
    std::byte* chunk_end_{nullptr};
    size_t allocated_bytes_{0};

    std::vector<std::unique_ptr<std::byte[]>> chunks_;
    std::vector<void (*)(void*)> destructors_;
    std::vector<void*> destructor_ptrs_;
};

} // namespace execore
