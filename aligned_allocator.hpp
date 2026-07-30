#pragma once

#include <cstddef>
#include <cstdint>
#include <new>

namespace i
{
template <std::size_t A, class T> constexpr bool is_aligned(const T* p) noexcept
{
    return reinterpret_cast<std::uintptr_t>(p) % A == 0;
}

template <class T, std::size_t A> struct aligned_allocator
{
    using value_type = T;

    template <class U> struct rebind
    {
        using other = aligned_allocator<U, A>;
    };

    aligned_allocator() = default;

    template <class U> aligned_allocator(const aligned_allocator<U, A>&) noexcept {}

    T* allocate(std::size_t n)
    {
        return static_cast<T*>(::operator new(n * sizeof(T), std::align_val_t{A}));
    }

    void deallocate(T* ptr, std::size_t n) noexcept
    {
        ::operator delete(ptr, n * sizeof(T), std::align_val_t{A});
    }

    template <class U> bool operator==(const aligned_allocator<U, A>&) const noexcept
    {
        return true;
    }
};
} // namespace i
