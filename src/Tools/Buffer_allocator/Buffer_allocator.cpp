#include "Tools/Buffer_allocator/Buffer_allocator.hpp"
#include "Runtime/Sequence/Sequence.hpp"
#include "Runtime/Socket/Socket.hpp"
#include <functional>
#include <iostream>
#include <sstream>

using namespace spu;
using namespace spu::tools;

void
Buffer_allocator::one_buffer_per_outsocket(runtime::Sequence* sequence)
{
    for (auto thread_tasks : sequence->get_tasks_per_threads())
    {
        for (auto task : thread_tasks)
        {
            // Try to remove condition
            if (!task->is_outbuffers_allocated()) task->allocate_outbuffers();
        }
    }
}

void
Buffer_allocator::allocate_sequence_memory(runtime::Sequence* sequence)
{
    if (sequence == nullptr)
    {
        std::stringstream message;
        message << "The sequence pointer is null ," << "Call set_sequence before allocating memory";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }
    else
    {
        one_buffer_per_outsocket(sequence);
    }
}