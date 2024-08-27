#ifdef SPU_JLUNA

#ifndef THREAD_POOL_JLUNA_V2_HPP_
#define THREAD_POOL_JLUNA_V2_HPP_

#include <condition_variable>
#include <jluna.hpp>
#include <mutex>

#include "Tools/Thread/Thread_barrier/Jluna/Thread_barrier_Jluna_v1.hpp"
#include "Tools/Thread/Thread_pool/Thread_pool.hpp"

namespace spu
{
namespace tools
{

class Thread_pool_Jluna_v2 : public Thread_pool
{
  private:
    std::vector<jluna::Task<void>> pool;
    std::vector<jluna::Proxy> jl_cnd;

    jluna::Proxy jl_lock;
    jluna::Proxy jl_unlock;
    jluna::Proxy jl_wait;
    jluna::Proxy jl_notify;

    Thread_barrier_Jluna_v1 barrier;

  public:
    Thread_pool_Jluna_v2(const size_t n_threads, std::function<void(const size_t)>& func_init);
    Thread_pool_Jluna_v2(const size_t n_threads);
    Thread_pool_Jluna_v2(const Thread_pool_Jluna_v2& other);
    virtual ~Thread_pool_Jluna_v2();

    virtual void init(const bool async = false);
    virtual void run(const bool async = false);
    virtual void run(std::function<void(const size_t)>& func_exec, const bool async = false);
    virtual void wait();

  protected:
    void _start_thread(const size_t tid);
};

}
}

#endif /* THREAD_POOL_JLUNA_V2_HPP_ */

#endif /* SPU_JLUNA */