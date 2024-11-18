#include "boundary_tag_allocator.h"

#include <gtest/gtest.h>
#include <vector>

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
  *my_int = 5;
  EXPECT_EQ(alloc.count_occupied_memory(), 48); // Is this correct?
  EXPECT_EQ(*my_int, 5);
}

TEST(BlockAllocator, Free) {
  BoundaryTagAllocator<int> alloc{1024};
  const auto my_int = alloc.allocate(sizeof(int));
  EXPECT_TRUE(my_int);
  EXPECT_EQ(alloc.count_occupied_memory(), 48);

  alloc.deallocate(my_int);
  EXPECT_EQ(alloc.count_occupied_memory(), 0);
}

TEST(BlockAllocator, AllocDeallocMany) {
  constexpr std::size_t size = 1024;
  BoundaryTagAllocator<int> alloc{size};
  std::vector<int *> ptr_vec{};
  for (int i = 0; i < 10; ++i) {
    const auto my_int = alloc.allocate(sizeof(int));
    EXPECT_TRUE(my_int);
    ptr_vec.push_back(my_int);
  }
  EXPECT_EQ(alloc.count_occupied_memory(), 48 * 10);

  for (const auto ptr : ptr_vec) {
    alloc.deallocate(ptr);
  }
  EXPECT_EQ(alloc.count_occupied_memory(), 0);
}
