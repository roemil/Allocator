#pragma once

#include <cassert>
#include <cstddef>
#include <iostream>
#include <memory>

template <typename T> class BoundaryTagAllocator {
private:
  struct Block {
    std::size_t size_{};
    bool is_free_{true};
    Block *next = nullptr;
    Block *prev = nullptr;
  };

public:
  BoundaryTagAllocator() = default;
  constexpr explicit BoundaryTagAllocator(std::size_t size)
      : total_size_(size) {
    ptr_ = std::make_unique<std::byte[]>(size);
    available_memory = reinterpret_cast<Block *>(ptr_.get());
    available_memory->size_ = total_size_;
    available_memory->is_free_ = true;
  }

  constexpr std::size_t max_size() const { return total_size_; }
  constexpr std::size_t count_occupied_memory() const {
    return total_size_ - available_memory->size_;
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
    std::cout << "aligned size " << aligned_size << "\n";
    std::cout << "Align of T " << alignof(T) << "\n";
    std::cout << "Align of Block " << alignof(Block) << "\n";

    Block *current = available_memory;
    while (current) {
      if (current->is_free_ && current->size_ > aligned_size) {
        // Check if chunk is large enough to split
        if (current->size_ > aligned_size + sizeof(Block)) {
          Block *new_block =
              reinterpret_cast<Block *>(reinterpret_cast<Block *>(
                  reinterpret_cast<std::byte *>(available_memory) +
                  aligned_size));
          current->next = new_block;
          new_block->prev = current;
          new_block->is_free_ = true;
          new_block->size_ = current->size_ - aligned_size;
          available_memory = new_block;
        }
        current->is_free_ = false;
        return reinterpret_cast<T *>(current);
      }
      current = current->next;
    }
    return nullptr;
  }

private:
  template <typename U, typename V> static size_t align_size(size_t size) {
    return (sizeof(U) + sizeof(V) - 1) & ~(sizeof(V) - 1);
  }
  std::size_t total_size_{};
  std::unique_ptr<std::byte[]> ptr_ = nullptr;
  Block *available_memory = nullptr;
};