#pragma once

#include <cassert>
#include <cstddef>
#include <exception>
#include <iostream>
#include <memory>
#include <utility>

template <typename T> class BoundaryTagAllocator {
private:
  struct Block {
    std::size_t size_{};
    bool is_free_{true};
    Block *next = nullptr;
    Block *prev = nullptr;
  };

public:
  constexpr BoundaryTagAllocator() = default;
  constexpr explicit BoundaryTagAllocator(std::size_t size)
      : total_size_(size) {
    ptr_ = std::make_unique<std::byte[]>(size);
    available_memory = reinterpret_cast<Block *>(ptr_.get());
    available_memory->size_ = total_size_;
    available_memory->is_free_ = true;
  }

  constexpr std::size_t max_size() const { return total_size_; }
  constexpr std::size_t count_occupied_memory() const {
    std::size_t free_memory = 0;
    Block *current = available_memory;
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

    std::size_t aligned_size = align_size<T, Block>(n + sizeof(Block));

    Block *current = available_memory;
    while (current) {
      if (current->is_free_ && current->size_ > aligned_size) {
        // Check if chunk is large enough to split
        if (current->size_ > aligned_size + sizeof(Block)) {
          Block *new_block =
              reinterpret_cast<Block *>(reinterpret_cast<Block *>(
                  reinterpret_cast<std::byte *>(available_memory) +
                  aligned_size));
          new_block->next = current->next;
          new_block->is_free_ = true;
          new_block->size_ = current->size_ - aligned_size;
          available_memory = new_block;
          current->size_ = aligned_size;
        }
        current->is_free_ = false;
        return reinterpret_cast<T *>(current + 1);
      }
      current = current->next;
    }
    return nullptr;
  }

  template <typename... ArgsT> constexpr void construct(T *p, ArgsT &&...args) {
    std::construct_at(p, std::forward<ArgsT>(args)...);
  }

  constexpr void deallocate(T *ptr) {
    if (!ptr) {
      return;
    }
    Block *block = reinterpret_cast<Block *>(ptr) - 1;
    if (!block) {
      return;
    }
    block->is_free_ = true;
    Block *current = available_memory;
    block->next = current;
    current->prev = block;

    if (block->next) {
      block->size_ += block->next->size_;
      block->next = block->next->next;
      if (block->next) {
        block->next->prev = block;
      }
    }

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
  Block *available_memory = nullptr;
};