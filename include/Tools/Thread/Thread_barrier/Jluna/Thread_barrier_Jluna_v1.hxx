#include <iostream>
#include <sstream>

#include "Tools/Exception/exception.hpp"
#include "Tools/Thread/Thread_barrier/Jluna/Thread_barrier_Jluna_v1.hpp"

namespace spu
{
namespace tools
{

Thread_barrier_Jluna_v1::Thread_barrier_Jluna_v1(const uint32_t n_threads)
  : n_threads(n_threads)
  , count(0)
  , jl_yield(jluna::Base.safe_eval("return Base.Threads.yield"))
  , jl_sleep(jluna::Base.safe_eval("return Base.sleep"))
{
}

Thread_barrier_Jluna_v1::Thread_barrier_Jluna_v1(const Thread_barrier_Jluna_v1& other)
  : n_threads(other.n_threads)
  , count(0)
  , jl_yield(jluna::Base.safe_eval("return Base.Threads.yield"))
  , jl_sleep(jluna::Base.safe_eval("return Base.sleep"))
{
}

void
Thread_barrier_Jluna_v1::arrive()
{
    return this->arrive(0);
}

void
Thread_barrier_Jluna_v1::reset()
{
    return this->reset(0);
}

void
Thread_barrier_Jluna_v1::wait()
{
    return this->wait(0);
}

void
Thread_barrier_Jluna_v1::arrive(const size_t tid)
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
Thread_barrier_Jluna_v1::reset(const size_t tid)
{
    this->count = 0;
}

void
Thread_barrier_Jluna_v1::wait(const size_t tid)
{
    while (this->count != this->n_threads)
    {
        this->jl_yield();
        if (this->count != this->n_threads) this->jl_sleep(0.0);
    }
    this->reset();
}

}
}
