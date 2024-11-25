#pragma once

#include "boundary_tag_allocator.h"
#include <cstddef>

namespace Allocator::PlacementPolicy {

struct FirstFit {
    static detail::Block *get_available_block(detail::Block *&head,
                                              std::size_t size);
};

inline detail::Block *
PlacementPolicy::FirstFit::get_available_block(detail::Block *&head,
                                               std::size_t size) {
    auto *current = head;
    while (current) {
        if (current->is_free_ && current->size_ >= size) {
            current->is_free_ = false;
            return current;
        }
        current = current->next;
    }
    return nullptr;
}

struct BestFit {
    static detail::Block *get_available_block(detail::Block *&head,
                                              std::size_t size);
};

inline detail::Block *
PlacementPolicy::BestFit::get_available_block(detail::Block *&head,
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

    return current_best_fit;
}

} // namespace Allocator::PlacementPolicy
