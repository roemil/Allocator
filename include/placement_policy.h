#pragma once

#include "boundary_tag_allocator.h"
#include <cstddef>

namespace Allocator::PlacementPolicy {

struct FirstFit {
    template <typename T>
    static T *get_available_block(detail::Block *&head, std::size_t size);
};

template <typename T>
T *PlacementPolicy::FirstFit::get_available_block(detail::Block *&head,
                                                  std::size_t size) {
    auto *current = head;
    while (current) {
        if (current->is_free_ && current->size_ > size) {
            // Check if chunk is large enough to split
            if (current->size_ > size + sizeof(detail::Block)) {
                auto *new_block = reinterpret_cast<detail::Block *>(
                    reinterpret_cast<detail::Block *>(
                        reinterpret_cast<std::byte *>(head) + size));
                new_block->next = current->next;
                new_block->is_free_ = true;
                new_block->size_ = current->size_ - size;
                head = new_block;
                current->size_ = size;
            }
            current->is_free_ = false;
            return reinterpret_cast<T *>(current + 1);
        }
        current = current->next;
    }
    return nullptr;
}

struct BestFit {
    template <typename T>
    static T *get_available_block(detail::Block *&head, std::size_t size);
};

template <typename T>
T *PlacementPolicy::BestFit::get_available_block(detail::Block *&head,
                                                 std::size_t size) {
    if (!head) {
        return nullptr;
    }
    auto *current = head;
    detail::Block *current_best_fit = head;
    std::size_t best_diff = size - current_best_fit->size_;

    while (current) {
        if (current->is_free_ && current->size_ >= size &&
            current->size_ < current_best_fit->size_) {
            current_best_fit = current;
        }
        current = current->next;
    }

    if (!current_best_fit) {
        return nullptr;
    }

    // Check if chunk is large enough to split
    if (current_best_fit->size_ > size + sizeof(detail::Block)) {
        auto *new_block =
            reinterpret_cast<detail::Block *>(reinterpret_cast<detail::Block *>(
                reinterpret_cast<std::byte *>(head) + size));
        new_block->next = current_best_fit->next;
        new_block->is_free_ = true;
        new_block->size_ = current_best_fit->size_ - size;
        head = new_block;
        current_best_fit->size_ = size;
    }
    return reinterpret_cast<T *>(current_best_fit + 1);
}

} // namespace Allocator::PlacementPolicy
