#ifdef SPU_JULIA

#ifndef THREAD_POOL_JULIA_HPP_
#define THREAD_POOL_JULIA_HPP_

#include <pthread.h>
#include <semaphore.h>

#include "Tools/Thread/Thread_pool/Thread_pool.hpp"

namespace spu
{
namespace tools
{
class Thread_pool_Julia : public Thread_pool
{
  private:
    struct _args
    {
        size_t tid;
        Thread_pool_Julia* tp;
        pthread_mutex_t lock;
        sem_t sem;
    };

    static void* thread_code(void* args);

    pthread_barrier_t barrier;
    pthread_barrier_t cond;
    std::vector<pthread_t> pool;

  public:
    Thread_pool_Julia(const size_t n_threads, std::function<void(const size_t)>& func_init);
    Thread_pool_Julia(const size_t n_threads);
    Thread_pool_Julia(const Thread_pool_Julia& oth);
    virtual ~Thread_pool_Julia();

    virtual void init(const bool async = false);
    virtual void run(const bool async = false);
    virtual void run(std::function<void(const size_t)>& func_exec, const bool async = false);
    virtual void wait();
};
}
}

#endif /* THREAD_POOL_JULIA_HPP_ */

#endif /* SPU_JULIA */
