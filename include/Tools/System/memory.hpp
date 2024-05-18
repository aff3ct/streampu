/*!
 * \file
 * \brief Memory allocation utilities.
 */
#ifndef SYSTEM_MEMORY_HPP__
#define SYSTEM_MEMORY_HPP__

#include <new>

namespace spu
{
namespace tools
{
void*
mem_alloc(std::size_t size);
void
mem_free(void* ptr);

template<typename T>
T*
buffer_alloc(std::size_t nData)
{
    T* const ptr = reinterpret_cast<T*>(mem_alloc(nData * sizeof(T)));
    if (!ptr) throw std::bad_alloc();
    return ptr;
}

template<typename T>
void
buffer_free(T* ptr)
{
    mem_free(ptr);
}

template<class T>
struct aligned_allocator
{
    typedef T value_type;
    aligned_allocator() {}
    template<class C>
    aligned_allocator(const aligned_allocator<C>& other)
    {
    }
    T* allocate(std::size_t n) { return buffer_alloc<T>(n); }
    void deallocate(T* p, std::size_t n) { buffer_free<T>(p); }
};

// Returns true if and only if storage allocated from ma1 can be
// deallocated from ma2, and vice versa. Always returns true for
// stateless allocators.
template<class C1, class C2>
bool
operator==(const aligned_allocator<C1>& ma1, const aligned_allocator<C2>& ma2)
{
    return true;
}

template<class C1, class C2>
bool
operator!=(const aligned_allocator<C1>& ma1, const aligned_allocator<C2>& ma2)
{
    return !(ma1 == ma2);
}

}
}

#endif // SYSTEM_MEMORY_HPP__
