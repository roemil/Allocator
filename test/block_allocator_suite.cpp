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
    EXPECT_EQ(*my_int, 5);
}

TEST(BlockAllocator, Leak){
    int* ptr = new int(5);
}