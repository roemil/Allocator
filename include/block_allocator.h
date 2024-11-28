#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <list>
#include <memory>

namespace Allocator {

template <typename T> class BlockAllocator {
  private:
    struct Block {
        constexpr Block() = default;
        alignas(T) std::array<std::byte, sizeof(T)> data_{};
        bool is_free_{true};
    };

  public:
    constexpr explicit BlockAllocator(std::size_t num_blocks)
        : num_blocks_(num_blocks) {
        for (int i = 0; i < num_blocks_; ++i) {
            list_.emplace_back(std::make_unique_for_overwrite<Block>());
        }
    }

    constexpr std::size_t get_max_storage() const {
        return num_blocks_ * sizeof(T);
    }

    constexpr T *allocate(std::size_t n) {
        if (n != sizeof(T)) {
            return nullptr;
        }
        auto *block = find_free_block();
        if (!block) {
            std::cout << "Failed to find free block\n";
            return nullptr;
        }
        block->is_free_ = false;
        return reinterpret_cast<T *>(block->data_.data());
    }

    constexpr void deallocate(T *ptr) {
        if (!ptr) {
            return;
        }
        auto block_it =
            std::find_if(list_.begin(), list_.end(), [ptr](auto &block) {
                if (block) {
                    auto *block_ptr =
                        reinterpret_cast<T *>(block->data_.data());
                    if (!block_ptr) {
                        return false;
                    }
                    return block_ptr == ptr;
                }
                return false;
            });
        if (block_it != list_.end()) {
            (*block_it)->is_free_ = true;
        }
    }

    constexpr std::size_t count_occupied_blocks() const {
        std::size_t count{};
        for (const auto &block : list_) {
            if (block && !block->is_free_) {
                ++count;
            }
        }
        return count;
    }

  private:
    Block *find_free_block() const {
        auto it = std::find_if(list_.cbegin(), list_.cend(), [](auto &block) {
            return block && block->is_free_;
        });
        if (it == list_.end()) {
            return nullptr;
        }
        return (*it).get();
    }

    std::size_t num_blocks_{};
    std::list<std::unique_ptr<Block>> list_{};
};
} // namespace Allocator