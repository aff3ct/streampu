#ifdef SPU_JLUNA

#ifndef THREAD_BARRIER_JLUNA_V2_HPP_
#define THREAD_BARRIER_JLUNA_V2_HPP_

#include <atomic>
#include <cstdint>
#include <jluna.hpp>

namespace spu
{
namespace tools
{

class Thread_barrier_Jluna_v2
{
  private:
    const uint32_t n_threads;
    uint32_t count;
    jluna::unsafe::Function* jl_unsafe_yield;
    jluna::Proxy jl_mtx;
    jluna::unsafe::Value* jl_unsafe_mtx;
    jluna::unsafe::Function* jl_unsafe_lock;
    jluna::unsafe::Function* jl_unsafe_unlock;
    jluna::unsafe::Function* jl_unsafe_sleep;

  public:
    inline Thread_barrier_Jluna_v2(const uint32_t n_threads);
    inline Thread_barrier_Jluna_v2(const Thread_barrier_Jluna_v2& other);
    inline ~Thread_barrier_Jluna_v2() = default;
    inline void arrive();
    inline void reset();
    inline void wait();

    inline void arrive(const size_t tid);
    inline void reset(const size_t tid);
    inline void wait(const size_t tid);
};

}
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "Tools/Thread/Thread_barrier/Jluna/Thread_barrier_Jluna_v2.hxx"
#endif

#endif /* THREAD_BARRIER_JLUNA_V2_HPP_ */

#endif /* SPU_JLUNA */
