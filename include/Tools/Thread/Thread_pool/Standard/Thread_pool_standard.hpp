#ifndef THREAD_POOL_STANDARD_HPP_
#define THREAD_POOL_STANDARD_HPP_

#include <condition_variable>
#include <mutex>
#include <thread>

#include "Tools/Thread/Thread_barrier/Standard/Thread_barrier_standard.hpp"
#include "Tools/Thread/Thread_pool/Thread_pool.hpp"

namespace spu
{
namespace tools
{

class Thread_pool_standard : public Thread_pool
{
  private:
    std::vector<std::thread> pool;
    std::vector<std::mutex> mtx;
    std::vector<std::condition_variable> cnd;
    Thread_barrier_standard barrier;

  public:
    Thread_pool_standard(const size_t n_threads, std::function<void(const size_t)>& func_init);
    Thread_pool_standard(const size_t n_threads);
    Thread_pool_standard(const Thread_pool_standard& other);
    virtual ~Thread_pool_standard();

    virtual void init(const bool async = false);
    virtual void run(const bool async = false);
    virtual void run(std::function<void(const size_t)>& func_exec, const bool async = false);
    virtual void wait();

  protected:
    void _start_thread(const size_t tid);
};

}
}

#endif /* THREAD_POOL_STANDARD_HPP_ */
