#include "block_allocator.h"

#include <gtest/gtest.h>

TEST(BlockAllocator, Constructor) {
    constexpr int size = sizeof(int) * 10;
    Allocator::BlockAllocator<int> alloc{size};
    EXPECT_EQ(alloc.get_max_storage(), size * sizeof(int));
}

TEST(BlockAllocator, Alloc) {
    constexpr int size = sizeof(int) * 10;
    Allocator::BlockAllocator<int> alloc{size};
    const auto my_int = alloc.allocate(sizeof(int));
    EXPECT_TRUE(my_int);
    EXPECT_EQ(alloc.count_occupied_blocks(), 1);
}

TEST(BlockAllocator, Free) {
    constexpr int size = sizeof(int) * 10;
    Allocator::BlockAllocator<int> alloc{size};
    const auto my_int = alloc.allocate(sizeof(int));
    EXPECT_TRUE(my_int);
    EXPECT_EQ(alloc.count_occupied_blocks(), 1);

    alloc.deallocate(my_int);
    EXPECT_EQ(alloc.count_occupied_blocks(), 0);
}

TEST(BlockAllocator, FillAlloc) {
    constexpr int size = sizeof(int) * 10;
    Allocator::BlockAllocator<int> alloc{size};
    for (int i = 0; i < size; ++i) {
        const auto my_int = alloc.allocate(sizeof(int));
        EXPECT_TRUE(my_int);
    }
    EXPECT_EQ(alloc.count_occupied_blocks(), size);
}
