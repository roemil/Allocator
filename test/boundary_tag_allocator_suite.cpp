#include "boundary_tag_allocator.h"

#include <gtest/gtest.h>

TEST(BlockAllocator, Constructor) {
  constexpr std::size_t size = 1024;
  BoundaryTagAllocator<int> alloc{size};
  EXPECT_EQ(alloc.max_size(), size);
}

TEST(BlockAllocator, Alloc) {
  constexpr std::size_t size = 1024;
  BoundaryTagAllocator<int> alloc{size};
  const auto my_int = alloc.allocate(sizeof(int));
  EXPECT_TRUE(my_int);
  EXPECT_EQ(alloc.count_occupied_memory(), 32); // Is 32 correct?
}

// TEST(BlockAllocator, Free){
//     BlockAllocator<int> alloc{10};
//     const auto my_int = alloc.allocate(5);
//     EXPECT_TRUE(my_int);
//     EXPECT_EQ(alloc.count_occupied_blocks(), 1);

//     alloc.deallocate(my_int);
//     EXPECT_EQ(alloc.count_occupied_blocks(), 0);
// }

// TEST(BlockAllocator, FillAlloc){
//     constexpr std::size_t size = 10;
//     BlockAllocator<int> alloc{size};
//     for(int i = 0; i < size; ++i){
//         const auto my_int = alloc.allocate(i);
//         EXPECT_TRUE(my_int);
//     }
//     EXPECT_EQ(alloc.count_occupied_blocks(), size);
// }
