#include "boundary_tag_allocator.h"

#include <gtest/gtest.h>
#include <vector>

TEST(BlockAllocator, Constructor) {
  constexpr std::size_t size = 1024;
  BoundaryTagAllocator<int> alloc{size};
  EXPECT_EQ(alloc.max_size(), size);
}

template <typename AllocT, typename T>
constexpr T *allocate_helper(AllocT &alloc, std::size_t n) {
  const auto p = alloc.allocate(n);
  EXPECT_TRUE(p);
  return p;
}
TEST(BlockAllocator, Alloc) {
  constexpr std::size_t size = 1024;
  BoundaryTagAllocator<int> alloc{size};
  auto my_int =
      allocate_helper<BoundaryTagAllocator<int>, int>(alloc, sizeof(int));
  *my_int = 5;
  EXPECT_EQ(alloc.count_occupied_memory(), 48); // Is this correct?
  EXPECT_EQ(*my_int, 5);
}

TEST(BlockAllocator, Free) {
  BoundaryTagAllocator<int> alloc{1024};
  auto my_int =
      allocate_helper<BoundaryTagAllocator<int>, int>(alloc, sizeof(int));
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

struct S {
  bool is_initialized = false;
  constexpr S() : is_initialized(true) {}
  constexpr ~S() { is_initialized = false; }
};

TEST(BlockAllocator, Construct) {
  constexpr std::size_t size = 1024;
  BoundaryTagAllocator<int> alloc{size};
  auto my_int =
      allocate_helper<BoundaryTagAllocator<int>, int>(alloc, sizeof(int));

  constexpr int expected_value = 5;
  alloc.construct(my_int, expected_value);

  EXPECT_EQ(*my_int, expected_value);
}

TEST(BlockAllocator, Destroy) {
  constexpr std::size_t size = 1024;
  BoundaryTagAllocator<S> alloc{size};
  auto p = allocate_helper<BoundaryTagAllocator<S>, S>(alloc, sizeof(S));
  EXPECT_TRUE(p);
  alloc.construct(p);
  EXPECT_TRUE(p->is_initialized);

  alloc.destroy(p);
  EXPECT_FALSE(p->is_initialized);
}