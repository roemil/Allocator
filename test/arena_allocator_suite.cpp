#include "arena_allocator.h"

#include "gtest/gtest.h"
#include <cstddef>

TEST(ArenaAllocator, Basic) {
    Allocator::ArenaAllocator<int> alloc{1024};
    auto *ptr = alloc.allocate(sizeof(int));
    EXPECT_TRUE(ptr);
}

TEST(ArenaAllocator, Fill) {
    Allocator::ArenaAllocator<int> alloc{1024};

    for (std::size_t i = 0; i < alloc.max_size() / sizeof(int); ++i) {
        EXPECT_TRUE(alloc.allocate(sizeof(int)));
    }
}

TEST(ArenaAllocator, FillFreeFill) {
    Allocator::ArenaAllocator<int> alloc{1024};

    for (std::size_t i = 0; i < alloc.max_size() / sizeof(int); ++i) {
        EXPECT_TRUE(alloc.allocate(sizeof(int)));
    }
    alloc.deallocate();

    for (std::size_t i = 0; i < alloc.max_size() / sizeof(int); ++i) {
        EXPECT_TRUE(alloc.allocate(sizeof(int)));
    }
}

TEST(ArenaAllocator, OverFill) {
    Allocator::ArenaAllocator<int> alloc{sizeof(int) * 2};

    // To the limit
    for (std::size_t i = 0; i < alloc.max_size() / sizeof(int); ++i) {
        EXPECT_TRUE(alloc.allocate(sizeof(int)));
    }
    EXPECT_FALSE(alloc.allocate(sizeof(int)));
}