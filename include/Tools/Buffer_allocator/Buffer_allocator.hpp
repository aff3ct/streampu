/*!
 * \file
 * \brief Class buffer::Buffer_allocation.
 */
#ifndef BUFFER_ALLOCATOR_HPP__
#define BUFFER_ALLOCATOR_HPP__

#include <utility>

#include "Tools/Interface/Interface_reset.hpp"

namespace spu
{
namespace runtime
{
class Socket;
class Sequence;
};
namespace tools
{
class Buffer_allocator
{
  public:
    void allocate_sequence_memory(runtime::Sequence* sequence);
    void deallocate_sequence_memory(runtime::Sequence* sequence);

  private:
    void allocate_one_buffer_per_outsocket(runtime::Sequence* sequence);
    void deallocate_one_buffer_per_outsocket(runtime::Sequence* sequence);
};
} // namespace tools
} // namespace spu

#endif // BUFFER_ALLOCATOR_HPP__