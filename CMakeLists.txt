cmake_minimum_required(VERSION 3.14)
project(allocator)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.15.2
)
FetchContent_MakeAvailable(googletest)
include_directories(include)

enable_testing()

add_executable(
    block_allocator_suite
    test/block_allocator_suite.cpp
)
target_link_libraries(
    block_allocator_suite gtest_main
)

target_compile_options(block_allocator_suite PRIVATE -fsanitize=address,undefined)
target_link_options(block_allocator_suite PRIVATE -fsanitize=address,undefined)

add_executable(
    boundary_tag_allocator_suite
    test/boundary_tag_allocator_suite.cpp
)

target_link_libraries(
  boundary_tag_allocator_suite gtest_main
)

target_compile_options(boundary_tag_allocator_suite PRIVATE -fsanitize=address,undefined)
target_link_options(boundary_tag_allocator_suite PRIVATE -fsanitize=address,undefined)

add_executable(
    placement_policy_suite
    test/placement_policy_suite.cpp
)

target_link_libraries(
  placement_policy_suite gtest_main
)

target_compile_options(placement_policy_suite PRIVATE -fsanitize=address,undefined)
target_link_options(placement_policy_suite PRIVATE -fsanitize=address,undefined)

add_executable(
    arena_allocator_suite
    test/arena_allocator_suite.cpp
)

target_link_libraries(
  arena_allocator_suite gtest_main
)

target_compile_options(arena_allocator_suite PRIVATE -fsanitize=address,undefined)
target_link_options(arena_allocator_suite PRIVATE -fsanitize=address,undefined)

include(GoogleTest)
gtest_discover_tests(block_allocator_suite)
gtest_discover_tests(boundary_tag_allocator_suite)
gtest_discover_tests(placement_policy_suite)
gtest_discover_tests(arena_allocator_suite)