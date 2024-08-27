#include <iostream>
#include <sstream>

#include "Tools/Exception/exception.hpp"
#include "Tools/Thread/Thread_barrier/Jluna/Thread_barrier_Jluna_v2.hpp"

namespace spu
{
namespace tools
{

Thread_barrier_Jluna_v2::Thread_barrier_Jluna_v2(const uint32_t n_threads)
  : n_threads(n_threads)
  , count(0)
  , jl_unsafe_yield(jluna::unsafe::get_function(jluna::Base, jluna::Symbol("yield")))
  , jl_mtx(jluna::Base.safe_eval("return Base.Threads.ReentrantLock()"))
  , jl_unsafe_mtx(static_cast<jluna::unsafe::Value*>(this->jl_mtx))
  , jl_unsafe_lock(jluna::unsafe::get_function(jluna::Base, jluna::Symbol("lock")))
  , jl_unsafe_unlock(jluna::unsafe::get_function(jluna::Base, jluna::Symbol("unlock")))
  , jl_unsafe_sleep(jluna::unsafe::get_function(jluna::Base, jluna::Symbol("sleep")))
{
}

Thread_barrier_Jluna_v2::Thread_barrier_Jluna_v2(const Thread_barrier_Jluna_v2& other)
  : n_threads(other.n_threads)
  , count(0)
  , jl_unsafe_yield(jluna::unsafe::get_function(jluna::Base, jluna::Symbol("yield")))
  , jl_mtx(jluna::Base.safe_eval("return Base.Threads.ReentrantLock()"))
  , jl_unsafe_mtx(static_cast<jluna::unsafe::Value*>(this->jl_mtx))
  , jl_unsafe_lock(jluna::unsafe::get_function(jluna::Base, jluna::Symbol("lock")))
  , jl_unsafe_unlock(jluna::unsafe::get_function(jluna::Base, jluna::Symbol("unlock")))
  , jl_unsafe_sleep(jluna::unsafe::get_function(jluna::Base, jluna::Symbol("sleep")))
{
}

void
Thread_barrier_Jluna_v2::arrive()
{
    return this->arrive(0);
}

void
Thread_barrier_Jluna_v2::reset()
{
    return this->reset(0);
}

void
Thread_barrier_Jluna_v2::wait()
{
    return this->wait(0);
}

void
Thread_barrier_Jluna_v2::arrive(const size_t tid)
{
    jluna::unsafe::call(this->jl_unsafe_lock, this->jl_unsafe_mtx);
    this->count++;
    jluna::unsafe::call(this->jl_unsafe_unlock, this->jl_unsafe_mtx);
    if (this->count > this->n_threads)
    {
        std::stringstream message;
        message << "Something went wrong, 'count' cannot be higher than 'n_threads' ('count' = " << this->count
                << " , 'n_threads' = " << n_threads << ").";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
}

void
Thread_barrier_Jluna_v2::reset(const size_t tid)
{
    jluna::unsafe::call(this->jl_unsafe_lock, this->jl_unsafe_mtx);
    this->count = 0;
    jluna::unsafe::call(this->jl_unsafe_unlock, this->jl_unsafe_mtx);
}

void
Thread_barrier_Jluna_v2::wait(const size_t tid)
{
    while (this->count != this->n_threads)
    {
        // /!\ this is very important for Julia user threads that can be locked
        jluna::unsafe::call(this->jl_unsafe_yield);
    }
    this->reset();
}

}
}
