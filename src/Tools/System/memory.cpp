#include <cstdlib>
#include "Tools/System/memory.hpp"

namespace aff3ct
{
namespace tools
{

static void* mem_alloc_aligned(std::size_t alignment, std::size_t size)
{
	void* ptr = nullptr;
#if __cplusplus >= 201703L
	ptr = std::aligned_alloc(alignment, size);
#elif defined _POSIX_C_SOURCE && (_POSIX_C_SOURCE >= 200112L)
	if (posix_memalign(&ptr, alignment, size) != 0)
		ptr = nullptr;
#else
#error "XXX: unimplemented mem_alloc_aligned()"
#endif
	return ptr;
}

void* mem_alloc(std::size_t size)
{
	std::size_t alignment = 64; // default alignment for up to 512-bit vectors
	// XXX: for large enough buffers (heuristics TBD), we could
	// make the alignment a page size to allow for CPU/GPU Shared
	// Virtual Memory (SVM) configurations
	return mem_alloc_aligned(alignment, size);
}

void mem_free(void* ptr)
{
	free(ptr);
}

}
}
