#ifndef THREAD_POOL_JULIA_HPP_
#define THREAD_POOL_JULIA_HPP_

#ifdef SPU_JULIA

#include <condition_variable>
#include <mutex>
#include <thread>

#include "Tools/Thread/Thread_barrier/Standard/Thread_barrier_standard.hpp"
#include "Tools/Thread/Thread_pool/Thread_pool.hpp"

namespace spu
{
namespace tools
{
class Thread_pool_Julia : public Thread_pool
{
  private:
    static std::mutex jl_adopt_mutex;

    std::vector<std::thread> pool;
    std::vector<std::mutex> mtx;
    std::vector<std::condition_variable> cnd;
    Thread_barrier_standard barrier;
    std::mutex mtx_init;
    std::condition_variable cnd_init;
    bool ready_init;
    bool all_adopted;
    int gc_old_state;

  public:
    Thread_pool_Julia(const size_t n_threads, std::function<void(const size_t)>& func_init);
    Thread_pool_Julia(const size_t n_threads);
    Thread_pool_Julia(const Thread_pool_Julia& other);
    virtual ~Thread_pool_Julia();

    virtual void init(const bool async = false);
    virtual void run(const bool async = false);
    virtual void run(std::function<void(const size_t)>& func_exec, const bool async = false);
    virtual void wait();

  protected:
    void _start_thread(const size_t tid);
};
}
}

#endif /* SPU_JULIA */

#endif /* THREAD_POOL_JULIA_HPP_ */
