#include "placement_policy.h"

#include "gtest/gtest.h"

TEST(PolicyFirstFit, Basic) {
    auto head = std::make_unique<Allocator::detail::Block>();
    head->size_ = 10;

    auto block = std::make_unique<Allocator::detail::Block>();
    block->size_ = 50;
    block->prev = head.get();

    head->next = block.get();

    auto *raw_head = head.get();
    auto *available_block =
        Allocator::PlacementPolicy::FirstFit::get_available_block<int>(raw_head,
                                                                       30);
    EXPECT_EQ(available_block, reinterpret_cast<int *>(block.get() + 1));
}

TEST(PolicyFirstFit, First) {
    auto head = std::make_unique<Allocator::detail::Block>();
    head->size_ = 10;

    auto block = std::make_unique<Allocator::detail::Block>();
    block->size_ = 50;
    block->prev = head.get();

    head->next = block.get();

    auto *raw_head = head.get();
    auto *available_block =
        Allocator::PlacementPolicy::FirstFit::get_available_block<int>(raw_head,
                                                                       5);
    EXPECT_EQ(available_block, reinterpret_cast<int *>(head.get() + 1));
}

TEST(PolicyFirstFit, NoFit) {
    auto head = std::make_unique<Allocator::detail::Block>();
    head->size_ = 10;

    auto block = std::make_unique<Allocator::detail::Block>();
    block->size_ = 50;
    block->prev = head.get();

    head->next = block.get();

    auto *raw_head = head.get();
    auto *available_block =
        Allocator::PlacementPolicy::FirstFit::get_available_block<int>(raw_head,
                                                                       60);
    EXPECT_FALSE(available_block);
}

TEST(PolicyFirstFit, NoFree) {
    auto head = std::make_unique<Allocator::detail::Block>();
    head->size_ = 10;
    head->is_free_ = false;

    auto block = std::make_unique<Allocator::detail::Block>();
    block->size_ = 50;
    block->prev = head.get();
    block->is_free_ = false;

    head->next = block.get();

    auto *raw_head = head.get();
    auto *available_block =
        Allocator::PlacementPolicy::FirstFit::get_available_block<int>(raw_head,
                                                                       5);
    EXPECT_FALSE(available_block);
}

TEST(PolicyFirstFit, FirstOccupiedSecondFree) {
    auto head = std::make_unique<Allocator::detail::Block>();
    head->size_ = 10;
    head->is_free_ = false;

    auto block = std::make_unique<Allocator::detail::Block>();
    block->size_ = 50;
    block->prev = head.get();
    block->is_free_ = true;

    head->next = block.get();

    auto *raw_head = head.get();
    auto *available_block =
        Allocator::PlacementPolicy::FirstFit::get_available_block<int>(raw_head,
                                                                       5);
    EXPECT_EQ(available_block, reinterpret_cast<int *>(block.get() + 1));
}