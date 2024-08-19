#include "Tools/Exception/exception.hpp"
#include "Tools/Threading/Barrier/Barrier.hpp"

namespace spu
{
namespace tools
{

Barrier::Barrier(const uint32_t n_threads)
  : n_threads(n_threads)
  , count(new std::atomic<uint32_t>(0))
{
}

Barrier::Barrier(const Barrier& other)
  : n_threads(other.n_threads)
  , count(new std::atomic<uint32_t>(0))
{
}

void
Barrier::arrive()
{
    (*this->count)++;

    if ((*this->count) > this->n_threads)
    {
        std::stringstream message;
        message << "Something went wrong, 'count' cannot be higher than 'n_threads' ('count' = " << this->count
                << " , 'n_threads' = " << n_threads << ").";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
}

void
Barrier::reset()
{
    (*this->count) = 0;
}

void
Barrier::wait()
{
    while ((*this->count) != this->n_threads)
        std::this_thread::sleep_for(std::chrono::microseconds(1));
    this->reset();
}

}
}