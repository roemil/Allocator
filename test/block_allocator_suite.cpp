#include "block_allocator.h"

#include <gtest/gtest.h>

TEST(BlockAllocator, Constructor){
    BlockAllocator<10> alloc{10};
    EXPECT_EQ(alloc.get_max_storage(), 100);
}

TEST(BlockAllocator, Alloc){
    BlockAllocator<10> alloc{10};
    const auto my_int = alloc.allocate(5);
    EXPECT_TRUE(my_int);
    EXPECT_EQ(alloc.count_occupied_blocks(), 1);
}

TEST(BlockAllocator, Free){
    BlockAllocator<10> alloc{10};
    const auto my_int = alloc.allocate(5);
    EXPECT_TRUE(my_int);
    EXPECT_EQ(alloc.count_occupied_blocks(), 1);

    alloc.deallocate(my_int);
    EXPECT_EQ(alloc.count_occupied_blocks(), 0);
}



TEST(BlockAllocator, FillAlloc){
    constexpr std::size_t size = 10;
    BlockAllocator<10> alloc{size};
    for(int i = 0; i < size; ++i){
        const auto my_int = alloc.allocate(i);
        EXPECT_TRUE(my_int);
    }
    EXPECT_EQ(alloc.count_occupied_blocks(), size);
}
