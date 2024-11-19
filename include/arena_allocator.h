#include <cstddef>
#include <cstdint>
#include <memory>

#pragma
namespace Allocator {
template <typename T> class ArenaAllocator {
  public:
    constexpr ArenaAllocator() = default;
    constexpr ArenaAllocator(std::size_t size) : size_(size) {
        ptr_ = std::make_unique<std::byte[]>(size_);
    }

    constexpr std::size_t max_size() const { return size_; }

    constexpr T *allocate(std::size_t n) {
        if (n == 0) {
            return nullptr;
        }
        if (n + offset_ > size_) {
            return nullptr;
        }
        if (n < sizeof(T)) {
            return nullptr;
        }
        auto space = size_ - offset_;
        std::byte *beg = ptr_.get() + offset_;
        T *ptr = (T *)std::align(alignof(T), n, reinterpret_cast<void *&>(beg),
                                 space);
        auto alignment = size_ - offset_ - space;
        if (!ptr) {
            return nullptr;
        }
        offset_ += sizeof(T) + alignment;

        return ptr;
    }

    constexpr void deallocate() {
        ptr_.reset();
        ptr_ = std::make_unique<std::byte[]>(size_);
        offset_ = 0;
    }

  private:
    std::unique_ptr<std::byte[]> ptr_ = nullptr;
    std::ptrdiff_t offset_{};
    std::size_t size_{};
};
} // namespace Allocator