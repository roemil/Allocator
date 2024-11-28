#pragma once

#include <cassert>
#include <cstddef>
#include <memory>
#include <utility>

namespace Allocator {
namespace detail {
struct Block {
    std::size_t size_{};
    bool is_free_{true};
    Block *next = nullptr;
    Block *prev = nullptr;
};

} // namespace detail

inline void coalesce_once(detail::Block *p) {
    if (!p) {
        return;
    }
    if (p->next && p->next->is_free_) {
        p->size_ += p->next->size_;
        p->next = p->next->next;
        if (p->next) {
            p->next->prev = p;
        }
    }
    if (p->prev && p->prev->is_free_) {
        p->prev->next = p->next;
        p->prev->size_ += p->size_;
        if (p->next) {
            p->next->prev = p->prev;
        }
    }
}

inline std::pair<detail::Block *, detail::Block *>
split_block_if_possible(detail::Block *candidate, std::size_t size) {
    // Check if chunk is large enough to split
    if (candidate->size_ <= size + sizeof(detail::Block)) {
        return {candidate, nullptr};
    }

    auto *new_memory_region = reinterpret_cast<detail::Block *>(
        reinterpret_cast<std::uintptr_t>(candidate) + size);

    auto *new_block = new (new_memory_region) detail::Block{};

    new_block->next = candidate->next;
    new_block->is_free_ = true;
    new_block->size_ = candidate->size_ - size;

    if (new_block->next) {
        new_block->next->prev = new_block;
    }

    candidate->size_ = size;

    return {candidate, new_block};
}

template <typename U, typename V> static size_t align_size(size_t size) {
    constexpr size_t alignment = std::max(alignof(U), alignof(V));
    return (size + alignment - 1) & ~(alignment - 1);
}

template <typename T, typename PlacementPolicyT> class BoundaryTagAllocator {

  public:
    constexpr BoundaryTagAllocator() = default;
    // TODO: Ctor for memory on stack
    constexpr explicit BoundaryTagAllocator(std::size_t size)
        : total_size_(size),
          ptr_(std::make_unique_for_overwrite<RawData[]>(size)) {

        auto *new_memory_region = reinterpret_cast<detail::Block *>(ptr_.get());
        available_memory = new (new_memory_region) detail::Block{};

        available_memory->size_ = total_size_;
        available_memory->is_free_ = true;
    }

    constexpr std::size_t max_size() const { return total_size_; }
    constexpr std::size_t count_occupied_memory() const {
        std::size_t free_memory = 0;
        detail::Block *current = available_memory;
        while (current) {
            free_memory += current->size_;
            current = current->next;
        }
        return total_size_ - free_memory;
    }

    constexpr T *allocate(std::size_t n) {
        assert(n >= sizeof(T));
        if (!available_memory) {
            return nullptr;
        }
        if (available_memory->size_ < n) {
            return nullptr;
        }

        std::size_t aligned_size =
            align_size<T, detail::Block>(n + sizeof(detail::Block));

        auto *block = PlacementPolicyT::get_available_block(available_memory,
                                                            aligned_size);
        auto [new_block, new_pool] =
            split_block_if_possible(block, aligned_size);
        if (new_pool) {
            available_memory = new_pool;
        }
        return reinterpret_cast<T *>(
            reinterpret_cast<std::uintptr_t>(new_block) +
            sizeof(detail::Block));
    }

    template <typename... ArgsT>
    constexpr void construct(T *p, ArgsT &&...args) {
        std::construct_at(p, std::forward<ArgsT>(args)...);
    }

    constexpr void deallocate(T *ptr) {
        if (!ptr) {
            return;
        }
        detail::Block *block = reinterpret_cast<detail::Block *>(ptr) - 1;
        if (!block) {
            return;
        }
        block->is_free_ = true;
        detail::Block *current = available_memory;
        block->next = current;
        current->prev = block;

        coalesce_once(block);

        available_memory = block;
    }

    constexpr void destroy(T *p) {
        if (!p) {
            return;
        }
        p->~T();
    }

  private:
    std::size_t total_size_{};
    detail::Block *available_memory = nullptr;
    using RawData = std::byte;
    std::unique_ptr<RawData[]> ptr_ = nullptr;
};
} // namespace Allocator