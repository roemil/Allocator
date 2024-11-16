#pragma once

#include <cassert>
#include <cstddef>
#include <new>
template<typename std::size_t BlockSize>
class BlockAllocator{
    private:
        template<typename T>
        struct Block{
            constexpr Block(T&& val) : val_(std::move(val)) {}
            T val_;
            bool is_free_ {false};
        };
    public:
    constexpr explicit BlockAllocator(std::size_t num_blocks) : num_blocks_(num_blocks), offset_(0) {
        start_ = new std::byte[num_blocks_ * BlockSize];
        assert(start_);
    }
    ~BlockAllocator() {
        delete[] start_;
    }

    constexpr std::size_t get_max_storage() const {
        return num_blocks_*BlockSize;
    }

    template<typename T>
    constexpr T* allocate(T&& val) {
        auto* block =  new(start_+offset_) Block<T>{std::forward<T>(val)};
        if(!block) {
            return nullptr;
        }
        offset_ += sizeof(Block<T>);
        return &block->val_;
    }

    template<typename T>
    constexpr void deallocate(T* ptr){
        Block<T>* block = start_+ptr;
        block->is_free_ = true;
    }


    private:
        std::size_t num_blocks_{};
        std::byte* start_;
        std::size_t offset_{};
};