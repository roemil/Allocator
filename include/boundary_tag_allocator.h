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

void coalesce_once(detail::Block *p);

template <typename T, typename PlacementPolicyT> class BoundaryTagAllocator {

  public:
    constexpr BoundaryTagAllocator() = default;
    constexpr explicit BoundaryTagAllocator(std::size_t size)
        : total_size_(size) {
        ptr_ = std::make_unique<std::byte[]>(size);
        available_memory = reinterpret_cast<detail::Block *>(ptr_.get());
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

        return PlacementPolicyT::template get_available_block<T>(
            available_memory, aligned_size);
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
    template <typename U, typename V> static size_t align_size(size_t size) {
        constexpr size_t alignment = alignof(std::max_align_t);
        return (size + alignment - 1) & ~(alignment - 1);
    }
    std::size_t total_size_{};
    std::unique_ptr<std::byte[]> ptr_ = nullptr;
    detail::Block *available_memory = nullptr;
};
} // namespace Allocator