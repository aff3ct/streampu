#ifdef SPU_JLUNA

#ifndef THREAD_BARRIER_JLUNA_V1_HPP_
#define THREAD_BARRIER_JLUNA_V1_HPP_

#include <atomic>
#include <cstdint>
#include <jluna.hpp>

namespace spu
{
namespace tools
{

class Thread_barrier_Jluna_v1
{
  private:
    const uint32_t n_threads;
    std::atomic<uint32_t> count;
    jluna::Proxy jl_yield;
    jluna::Proxy jl_sleep;

  public:
    inline Thread_barrier_Jluna_v1(const uint32_t n_threads);
    inline Thread_barrier_Jluna_v1(const Thread_barrier_Jluna_v1& other);
    inline ~Thread_barrier_Jluna_v1() = default;
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
#include "Tools/Thread/Thread_barrier/Jluna/Thread_barrier_Jluna_v1.hxx"
#endif

#endif /* THREAD_BARRIER_JLUNA_V1_HPP_ */

#endif /* SPU_JLUNA */
