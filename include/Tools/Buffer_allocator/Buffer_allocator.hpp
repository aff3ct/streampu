/*!
 * \file
 * \brief Class buffer::Buffer_allocator.
 */
#ifndef BUFFER_ALLOCATOR_HPP__
#define BUFFER_ALLOCATOR_HPP__

#include <utility>

#include "Tools/Interface/Interface_reset.hpp"

namespace spu
{
namespace runtime
{
class Sequence;
};
namespace tools
{
class Buffer_allocator
{
  public:
    static void allocate_sequence_memory(runtime::Sequence* sequence);
    static void deallocate_sequence_memory(runtime::Sequence* sequence);

  private:
    static void allocate_one_buffer_per_outsocket(runtime::Sequence* sequence);
    static void deallocate_one_buffer_per_outsocket(runtime::Sequence* sequence);
};
} // namespace tools
} // namespace spu

#endif // BUFFER_ALLOCATOR_HPP__