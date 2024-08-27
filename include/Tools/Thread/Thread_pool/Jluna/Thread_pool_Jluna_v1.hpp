#ifdef SPU_JLUNA

#ifndef THREAD_POOL_JLUNA_V1_HPP_
#define THREAD_POOL_JLUNA_V1_HPP_

#include <condition_variable>
#include <jluna.hpp>
#include <mutex>

#include "Tools/Thread/Thread_barrier/Jluna/Thread_barrier_Jluna_v1.hpp"
#include "Tools/Thread/Thread_pool/Thread_pool.hpp"

namespace spu
{
namespace tools
{

class Thread_pool_Jluna_v1 : public Thread_pool
{
  private:
    std::vector<jluna::Task<void>> pool;

  public:
    Thread_pool_Jluna_v1(const size_t n_threads, std::function<void(const size_t)>& func_init);
    Thread_pool_Jluna_v1(const size_t n_threads);
    Thread_pool_Jluna_v1(const Thread_pool_Jluna_v1& other);
    virtual ~Thread_pool_Jluna_v1();

    virtual void init(const bool async = false);
    virtual void run(const bool async = false);
    virtual void run(std::function<void(const size_t)>& func_exec, const bool async = false);
    virtual void wait();
};

}
}

#endif /* THREAD_POOL_JLUNA_V1_HPP_ */

#endif /* SPU_JLUNA */