#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <memory>
#include <new>
#include <list>
#include <iostream>

template<typename std::size_t BlockSize>
class BlockAllocator{
    private:
        struct Block{
            constexpr Block() = default;
            std::byte data_[BlockSize]{};
            bool is_free_ {true};
        };
    public:
    constexpr explicit BlockAllocator(std::size_t num_blocks) : num_blocks_(num_blocks), offset_(0) {
        for(int i = 0; i < num_blocks_; ++i){
            list_.emplace_back(new Block{});
        }
    }

    constexpr std::size_t get_max_storage() const {
        return num_blocks_*BlockSize;
    }

    template<typename T>
    constexpr T* allocate(T& val) {
        return allocate(std::forward<T>(val));
    }
    template<typename T>
    constexpr T* allocate(T&& val) {
        assert(sizeof(T) <= BlockSize);
        auto* block = find_free_block();
        if(!block) {
            std::cout << "Failed to find free block\n";
            return nullptr;
        }
        block->is_free_ = false;
        auto* ptr = new(block->data_) T{val};
        return reinterpret_cast<T*>(ptr);
    }

    template<typename T>
    constexpr void deallocate(T* ptr){
        if(!ptr){
            return;
        }
        auto block = std::find_if(list_.begin(), list_.end(), [ptr](auto block){
            if(block){
                auto* block_ptr = reinterpret_cast<T*>(block->data_);
                if(!block_ptr){
                    return false;
                }
                return block_ptr == ptr;
            }
            return false;
        });
        if(block != list_.end()) {
            (*block)->is_free_ = true;
            auto* ptr = reinterpret_cast<T*>((*block)->data_);
            if(ptr){
                ptr->~T();
            }
        }
    }

    constexpr std::size_t count_occupied_blocks() const {
        std::size_t count{};
        for(const auto block : list_){
            if(block && !block->is_free_){
                ++count;
            }
        }
        return count;

    }


    private:
        Block* find_free_block() const {
            auto it = std::find_if(list_.cbegin(), list_.cend(), [](auto* block){
                return block && block->is_free_;
            });
            if(it == list_.end()){
                return nullptr;
            }
            return *it;
        }

        std::size_t num_blocks_{};
        std::list<Block*> list_{};
        std::size_t offset_{};
};