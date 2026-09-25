#pragma once

#include <memory_resource>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <new>
#include <concepts>
#include <algorithm>

namespace execore {

/**
 * @brief High-performance Polymorphic Memory Resource (PMR) bump/arena allocator.
 *
 * Implements std::pmr::memory_resource for zero-heap allocation across compiler passes
 * and runtime execution loops. Complies with C++2026 Hardware Sympathy guidelines.
 */
class MonotonicArenaResource : public std::pmr::memory_resource {
public:
    explicit MonotonicArenaResource(size_t chunk_size = 64 * 1024,
                                   std::pmr::memory_resource* upstream = std::pmr::get_default_resource()) noexcept
        : chunk_size_(chunk_size), upstream_(upstream) {}

    ~MonotonicArenaResource() override {
        release();
    }

    MonotonicArenaResource(const MonotonicArenaResource&) = delete;
    MonotonicArenaResource& operator=(const MonotonicArenaResource&) = delete;
    MonotonicArenaResource(MonotonicArenaResource&&) noexcept = delete;
    MonotonicArenaResource& operator=(MonotonicArenaResource&&) noexcept = delete;

    void release() noexcept {
        for (const auto& chunk : chunks_) {
            upstream_->deallocate(chunk.ptr, chunk.size, chunk.alignment);
        }
        chunks_.clear();
        current_ptr_ = nullptr;
        chunk_end_ = nullptr;
        total_allocated_ = 0;
    }

    [[nodiscard]] size_t total_allocated() const noexcept { return total_allocated_; }
    [[nodiscard]] size_t chunk_count() const noexcept { return chunks_.size(); }

protected:
    void* do_allocate(size_t bytes, size_t alignment) override {
        if (bytes == 0) return nullptr;

        size_t current_addr = reinterpret_cast<size_t>(current_ptr_);
        size_t aligned_addr = (current_addr + alignment - 1) & ~(alignment - 1);
        size_t padding = aligned_addr - current_addr;

        if (current_ptr_ && current_ptr_ + padding + bytes <= chunk_end_) {
            current_ptr_ += padding + bytes;
            total_allocated_ += padding + bytes;
            return reinterpret_cast<void*>(aligned_addr);
        }

        size_t new_chunk_size = std::max(chunk_size_, bytes + alignment);
        void* new_chunk = upstream_->allocate(new_chunk_size, alignment);
        chunks_.push_back(Chunk{new_chunk, new_chunk_size, alignment});

        current_ptr_ = static_cast<std::byte*>(new_chunk);
        chunk_end_ = current_ptr_ + new_chunk_size;

        current_addr = reinterpret_cast<size_t>(current_ptr_);
        aligned_addr = (current_addr + alignment - 1) & ~(alignment - 1);
        padding = aligned_addr - current_addr;

        current_ptr_ += padding + bytes;
        total_allocated_ += padding + bytes;
        return reinterpret_cast<void*>(aligned_addr);
    }

    void do_deallocate(void* /*p*/, size_t /*bytes*/, size_t /*alignment*/) noexcept override {
        // Individual deallocations are no-ops because monotonic memory is reclaimed in bulk during release()
    }

    bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override {
        return this == &other;
    }

private:
    struct Chunk {
        void* ptr;
        size_t size;
        size_t alignment;
    };

    size_t chunk_size_;
    std::pmr::memory_resource* upstream_;
    std::byte* current_ptr_{nullptr};
    std::byte* chunk_end_{nullptr};
    size_t total_allocated_{0};
    std::vector<Chunk> chunks_;
};

} // namespace execore
