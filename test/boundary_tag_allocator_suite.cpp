#include "boundary_tag_allocator.h"
#include "placement_policy.h"

#include <gtest/gtest.h>
#include <memory>
#include <vector>

TEST(BoundaryTagAllocator, Constructor) {
    constexpr std::size_t size = 1024;
    Allocator::BoundaryTagAllocator<int, Allocator::PlacementPolicy::FirstFit>
        alloc{size};
    EXPECT_EQ(alloc.max_size(), size);
}

template <typename AllocT, typename T>
constexpr T *allocate_helper(AllocT &alloc, std::size_t n) {
    const auto p = alloc.allocate(n);
    EXPECT_TRUE(p);
    return p;
}

TEST(BoundaryTagAllocator, Alloc) {
    constexpr std::size_t size = 1024;
    Allocator::BoundaryTagAllocator<int, Allocator::PlacementPolicy::FirstFit>
        alloc{size};
    auto my_int = allocate_helper<decltype(alloc), int>(alloc, sizeof(int));
    *my_int = 5;
    EXPECT_EQ(alloc.count_occupied_memory(), 48); // Is this correct?
    EXPECT_EQ(*my_int, 5);
}

TEST(BoundaryTagAllocator, Free) {
    Allocator::BoundaryTagAllocator<int, Allocator::PlacementPolicy::FirstFit>
        alloc{1024};
    auto my_int = allocate_helper<decltype(alloc), int>(alloc, sizeof(int));
    EXPECT_EQ(alloc.count_occupied_memory(), 48);

    alloc.deallocate(my_int);
    EXPECT_EQ(alloc.count_occupied_memory(), 0);
}

TEST(BoundaryTagAllocator, AllocDeallocMany) {
    constexpr std::size_t size = 1024;
    Allocator::BoundaryTagAllocator<int, Allocator::PlacementPolicy::FirstFit>
        alloc{size};
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

TEST(BoundaryTagAllocator, Construct) {
    constexpr std::size_t size = 1024;
    Allocator::BoundaryTagAllocator<int, Allocator::PlacementPolicy::FirstFit>
        alloc{size};
    auto my_int =
        allocate_helper<Allocator::BoundaryTagAllocator<
                            int, Allocator::PlacementPolicy::FirstFit>,
                        int>(alloc, sizeof(int));

    constexpr int expected_value = 5;
    alloc.construct(my_int, expected_value);

    EXPECT_EQ(*my_int, expected_value);
}

TEST(BoundaryTagAllocator, Destroy) {
    constexpr std::size_t size = 1024;
    Allocator::BoundaryTagAllocator<S, Allocator::PlacementPolicy::FirstFit>
        alloc{size};
    auto p = allocate_helper<decltype(alloc), S>(alloc, sizeof(S));
    EXPECT_TRUE(p);
    alloc.construct(p);
    EXPECT_TRUE(p->is_initialized);

    alloc.destroy(p);
    EXPECT_FALSE(p->is_initialized);
}

TEST(Coalesce, Right) {
    auto middle = std::make_unique<Allocator::detail::Block>();
    middle->size_ = 10;

    auto left = std::make_unique<Allocator::detail::Block>();
    left->size_ = 50;
    left->next = middle.get();
    left->is_free_ = false;

    middle->prev = left.get();

    auto right = std::make_unique<Allocator::detail::Block>();
    right->size_ = 90;
    middle->next = right.get();
    right->prev = middle.get();

    Allocator::coalesce_once(middle.get());

    EXPECT_EQ(middle->size_, 100);
    EXPECT_EQ(left->next, middle.get());
    EXPECT_FALSE(middle->next);
}

TEST(Coalesce, Left) {
    auto middle = std::make_unique<Allocator::detail::Block>();
    middle->size_ = 10;

    auto left = std::make_unique<Allocator::detail::Block>();
    left->size_ = 50;
    left->next = middle.get();

    middle->prev = left.get();

    auto right = std::make_unique<Allocator::detail::Block>();
    right->size_ = 90;
    right->is_free_ = false;
    middle->next = right.get();
    right->prev = middle.get();

    Allocator::coalesce_once(middle.get());

    EXPECT_EQ(left->size_, 60);
    EXPECT_EQ(left->next, right.get());
}

TEST(Coalesce, LeftAndRight) {
    auto middle = std::make_unique<Allocator::detail::Block>();
    middle->size_ = 10;

    auto left = std::make_unique<Allocator::detail::Block>();
    left->size_ = 50;
    left->next = middle.get();

    middle->prev = left.get();

    auto right = std::make_unique<Allocator::detail::Block>();
    right->size_ = 90;
    middle->next = right.get();
    right->prev = middle.get();

    auto end = std::make_unique<Allocator::detail::Block>();
    end->size_ = 100;
    right->next = end.get();
    right->prev = middle.get();

    Allocator::coalesce_once(middle.get());

    EXPECT_EQ(left->size_, 150);
    EXPECT_EQ(left->next, end.get());
    EXPECT_FALSE(left->prev);
}

TEST(SplitBlock, SplitBlock){
    std::size_t aligned_size =
            Allocator::align_size<int, Allocator::detail::Block>(50 + sizeof(Allocator::detail::Block));
    auto memory = std::make_unique<std::byte[]>(aligned_size*3);
    auto pool = reinterpret_cast<Allocator::detail::Block*>(memory.get());
    pool->size_ = aligned_size*3;
    auto [new_block, new_pool] = Allocator::split_block_if_possible(pool, aligned_size);

    EXPECT_EQ(new_pool->size_, aligned_size*2);
    EXPECT_TRUE(new_block);
    EXPECT_EQ(new_block->size_, aligned_size);
}

TEST(SplitBlock, NoSplit){
    std::size_t aligned_size =
            Allocator::align_size<int, Allocator::detail::Block>(50 + sizeof(Allocator::detail::Block));
    auto memory = std::make_unique<std::byte[]>(aligned_size+aligned_size*0.1);
    auto pool = reinterpret_cast<Allocator::detail::Block*>(memory.get());
    pool->size_ = aligned_size+aligned_size*0.1;
    auto [new_block, new_pool] = Allocator::split_block_if_possible(pool, aligned_size);

    EXPECT_FALSE(new_pool);
    EXPECT_TRUE(new_block);
    EXPECT_EQ(new_block->size_, pool->size_);
}

TEST(SplitBlock, SplitPoolTwice){
    const std::size_t aligned_size =
            Allocator::align_size<int, Allocator::detail::Block>(50 + sizeof(Allocator::detail::Block));
    const std::size_t pool_size = aligned_size * 5;
    auto memory = std::make_unique<std::byte[]>(pool_size);
    auto pool = reinterpret_cast<Allocator::detail::Block*>(memory.get());
    pool->size_ = pool_size;
    {
        auto [new_block, new_pool] = Allocator::split_block_if_possible(pool, aligned_size);

        EXPECT_EQ(new_pool->size_, pool_size-aligned_size);
        EXPECT_TRUE(new_block);
        EXPECT_EQ(new_block->size_, aligned_size);
        pool = new_pool;
    }
    {
        auto [new_block, new_pool] = Allocator::split_block_if_possible(pool, aligned_size);

        EXPECT_EQ(new_pool->size_, pool_size-aligned_size*2);
        EXPECT_TRUE(new_block);
        EXPECT_EQ(new_block->size_, aligned_size);
        pool = new_pool;
    }
}

TEST(SplitBlock, SplitBlockTwice){
    const std::size_t aligned_size =
            Allocator::align_size<int, Allocator::detail::Block>(50 + sizeof(Allocator::detail::Block));
    const std::size_t pool_size = aligned_size * 5;
    auto memory = std::make_unique<std::byte[]>(pool_size);
    auto pool = reinterpret_cast<Allocator::detail::Block*>(memory.get());
    pool->size_ = pool_size;

    auto [new_block, new_pool] = Allocator::split_block_if_possible(pool, aligned_size);

    EXPECT_EQ(new_pool->size_, pool_size-aligned_size);
    EXPECT_TRUE(new_block);
    EXPECT_EQ(new_block->size_, aligned_size);
    pool = new_pool;

    new_block->next = pool;

    {
        const std::size_t small_aligned_size =
                Allocator::align_size<int, Allocator::detail::Block>(10 + sizeof(Allocator::detail::Block));
        ASSERT_GT(new_block->size_, small_aligned_size);
        const auto old_block_size = new_block->size_;
        auto [small_block, new_pool] = Allocator::split_block_if_possible(new_block, small_aligned_size);

        EXPECT_TRUE(new_pool);
        EXPECT_EQ(new_pool->size_, old_block_size-small_aligned_size);
        EXPECT_TRUE(small_block);
        EXPECT_EQ(small_block->size_, small_aligned_size);
        EXPECT_TRUE(small_block->next);
        EXPECT_EQ(new_pool->next, pool);
    }
}