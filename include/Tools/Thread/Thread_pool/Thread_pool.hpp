#ifndef THREAD_POOL_HPP_
#define THREAD_POOL_HPP_

#include <condition_variable>
#include <cstdint>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

#include "Tools/Thread/Thread_barrier/Thread_barrier.hpp"

namespace spu
{
namespace tools
{

class Thread_pool
{
  private:
    const size_t n_threads;
    bool initialized;
    std::vector<std::thread> pool;
    std::vector<std::mutex> mtx;
    std::vector<std::condition_variable> cnd;
    std::function<void(const size_t)> func_init;
    std::function<void(const size_t)> func_deinit;
    std::function<void(const size_t)> func_exec;
    Thread_barrier barrier;
    bool stop_threads;

  public:
    Thread_pool(const size_t n_threads, std::function<void(const size_t)>& func_init);
    Thread_pool(const size_t n_threads);
    Thread_pool(const Thread_pool& other);
    virtual ~Thread_pool();
    void init(const bool async = false);
    void set_func_init(std::function<void(const size_t)>& func_init);
    void set_func_deinit(std::function<void(const size_t)>& func_deinit);
    void set_func_exec(std::function<void(const size_t)>& func_exec);
    void unset_func_exec();
    void run(const bool async = false);
    void run(std::function<void(const size_t)>& func_exec, const bool async = false);
    void wait();

  protected:
    void _start_thread(const size_t tid);
};

}
}

#endif /* THREAD_POOL_HPP_ */
