#pragma once

#include "boundary_tag_allocator.h"

namespace Allocator {

namespace PlacementPolicy {

struct FirstFit {
    template <typename T>
    static T *get_available_block(detail::Block *&head, std::size_t size);
};
} // namespace PlacementPolicy

template <typename T>
T *PlacementPolicy::FirstFit::get_available_block(detail::Block *&head,
                                                  std::size_t size) {
    auto *current = head;
    while (current) {
        if (current->is_free_ && current->size_ > size) {
            // Check if chunk is large enough to split
            if (current->size_ > size + sizeof(detail::Block)) {
                detail::Block *new_block = reinterpret_cast<detail::Block *>(
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
} // namespace Allocator