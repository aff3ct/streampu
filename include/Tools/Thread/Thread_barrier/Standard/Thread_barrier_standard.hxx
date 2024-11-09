#include <iostream>
#include <sstream>

#include "Tools/Exception/exception.hpp"
#include "Tools/Thread/Thread_barrier/Standard/Thread_barrier_standard.hpp"

namespace spu
{
namespace tools
{

Thread_barrier_standard::Thread_barrier_standard(const uint32_t n_threads)
  : n_threads(n_threads)
  , count(0)
{
}

Thread_barrier_standard::Thread_barrier_standard(const Thread_barrier_standard& other)
  : n_threads(other.n_threads)
  , count(0)
{
}

void
Thread_barrier_standard::arrive()
{
    this->count++;
    if (this->count > this->n_threads)
    {
        std::stringstream message;
        message << "Something went wrong, 'count' cannot be higher than 'n_threads' ('count' = " << this->count
                << " , 'n_threads' = " << n_threads << ").";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
}

void
Thread_barrier_standard::reset()
{
    this->count = 0;
}

void
Thread_barrier_standard::wait()
{
    while (this->count != this->n_threads)
        std::this_thread::sleep_for(std::chrono::microseconds(1));
    this->reset();
}

}
}
