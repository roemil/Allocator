#include <cstddef>
#include <memory>

#pragma
namespace Allocator {
template <typename T> class ArenaAllocator {
  public:
    constexpr ArenaAllocator() = default;
    constexpr ArenaAllocator(std::size_t size) : size_(size), space_(size_) {
        ptr_ = std::make_unique_for_overwrite<RawData[]>(size_);
    }

    constexpr std::size_t max_size() const { return size_; }
    constexpr T *allocate(std::size_t n) {
        if (n == 0) {
            return nullptr;
        }
        if (space_ < n) {
            return nullptr;
        }
        if (n + offset_ > size_) {
            return nullptr;
        }
        if (n < sizeof(T)) {
            return nullptr;
        }

        auto *beg = ptr_.get() + offset_;
        auto old_space = space_;
        T *ptr = (T *)std::align(alignof(T), n, reinterpret_cast<void *&>(beg),
                                 space_);
        if (!ptr) {
            return nullptr;
        }
        space_ -= sizeof(T);
        offset_ += old_space - space_;

        return ptr;
    }

    constexpr void deallocate() {
        ptr_.reset();
        ptr_ = std::make_unique_for_overwrite<RawData[]>(size_);
        offset_ = 0;
        space_ = size_;
    }

  private:
    using RawData = std::aligned_storage_t<sizeof(T), alignof(T)>;
    std::unique_ptr<RawData[]> ptr_ = nullptr;
    std::ptrdiff_t offset_{};
    std::size_t size_{};
    std::size_t space_{};
};
} // namespace Allocator