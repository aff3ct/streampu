#include "Tools/System/memory.hpp"
#include <cstdlib>

namespace aff3ct
{
namespace tools
{

static void*
mem_alloc_aligned(std::size_t alignment, std::size_t size)
{
    void* ptr = nullptr;
#if __cplusplus >= 201703L
    ptr = std::aligned_alloc(alignment, size);
#elif defined _POSIX_C_SOURCE && (_POSIX_C_SOURCE >= 200112L)
    if (posix_memalign(&ptr, alignment, size) != 0) ptr = nullptr;
#else
    {
        if (alignment < sizeof(void*))
        {
            alignment = sizeof(void*);
        }
        void* raw_ptr = malloc(size + alignment);
        uintptr_t addr = (uintptr_t)raw_ptr;
        addr = (addr & ~(alignment - 1)) + alignment;
        *(void**)(addr - sizeof(void*)) = raw_ptr;
        ptr = (void*)addr;
    }
#endif
    return ptr;
}

void*
mem_alloc(std::size_t size)
{
    std::size_t alignment = 64; // default alignment for up to 512-bit vectors
    // XXX: for large enough buffers (heuristics TBD), we could
    // make the alignment a page size to allow for CPU/GPU Shared
    // Virtual Memory (SVM) configurations
    return mem_alloc_aligned(alignment, size);
}

void
mem_free(void* ptr)
{
#if (__cplusplus >= 201703L) || (defined _POSIX_C_SOURCE && (_POSIX_C_SOURCE >= 200112L))
    free(ptr);
#else
    {
        uintptr_t addr = (uintptr_t)ptr;
        void* raw_ptr = *(void**)(addr - sizeof(void*));
        free(raw_ptr);
    }
#endif
}

}
}
