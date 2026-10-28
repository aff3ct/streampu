#include <functional>
#include <iostream>
#include <sstream>

#include "Runtime/Sequence/Sequence.hpp"
#include "Tools/Buffer_allocator/Buffer_allocator.hpp"

using namespace spu;
using namespace spu::tools;

bool g_task_autoalloc = false;

void
Buffer_allocator::set_task_autoalloc(bool autoalloc)
{
    g_task_autoalloc = autoalloc;
}

bool
Buffer_allocator::get_task_autoalloc()
{
    return g_task_autoalloc;
}

void
Buffer_allocator::allocate_one_buffer_per_outsocket(runtime::Sequence* sequence)
{
    for (auto thread_tasks : sequence->get_tasks_per_threads())
    {
        for (auto task : thread_tasks)
        {
            task->allocate_outbuffers();
        }
    }
}
void
Buffer_allocator::deallocate_one_buffer_per_outsocket(runtime::Sequence* sequence)
{
    for (auto thread_tasks : sequence->get_tasks_per_threads())
    {
        for (auto task : thread_tasks)
        {
            task->deallocate_outbuffers();
        }
    }
}

void
Buffer_allocator::allocate_sequence_memory(runtime::Sequence* sequence)
{
    if (sequence == nullptr)
    {
        std::stringstream message;
        message << "The sequence pointer is null"
                << ", call set_sequence before allocating memory";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }
    else
    {
        Buffer_allocator::allocate_one_buffer_per_outsocket(sequence);
    }
}

void
Buffer_allocator::deallocate_sequence_memory(runtime::Sequence* sequence)
{
    if (sequence == nullptr)
    {
        std::stringstream message;
        message << "The sequence pointer is null";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }
    else
    {
        Buffer_allocator::deallocate_one_buffer_per_outsocket(sequence);
    }
}