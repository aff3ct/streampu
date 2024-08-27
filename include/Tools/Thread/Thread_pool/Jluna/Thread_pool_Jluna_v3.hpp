#ifdef SPU_JLUNA

#ifndef THREAD_POOL_JLUNA_V3_HPP_
#define THREAD_POOL_JLUNA_V3_HPP_

#include <condition_variable>
#include <jluna.hpp>
#include <mutex>

#include "Tools/Thread/Thread_barrier/Jluna/Thread_barrier_Jluna_v2.hpp"
#include "Tools/Thread/Thread_pool/Thread_pool.hpp"

namespace spu
{
namespace tools
{

class Thread_pool_Jluna_v3 : public Thread_pool
{
  private:
    std::vector<jluna::Task<void>> pool;
    std::vector<jluna::Proxy> jl_cnd;

    jluna::unsafe::Function* jl_unsafe_lock;
    jluna::unsafe::Function* jl_unsafe_unlock;
    jluna::unsafe::Function* jl_unsafe_wait;
    jluna::unsafe::Function* jl_unsafe_notify;

    Thread_barrier_Jluna_v2 barrier;

  public:
    Thread_pool_Jluna_v3(const size_t n_threads, std::function<void(const size_t)>& func_init);
    Thread_pool_Jluna_v3(const size_t n_threads);
    Thread_pool_Jluna_v3(const Thread_pool_Jluna_v3& other);
    virtual ~Thread_pool_Jluna_v3();

    virtual void init(const bool async = false);
    virtual void run(const bool async = false);
    virtual void run(std::function<void(const size_t)>& func_exec, const bool async = false);
    virtual void wait();

  protected:
    void _start_thread(const size_t tid);
};

}
}

#endif /* THREAD_POOL_JLUNA_V3_HPP_ */

#endif /* SPU_JLUNA */