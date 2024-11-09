#ifndef THREAD_POOL_HPP_
#define THREAD_POOL_HPP_

#include <cstdint>
#include <functional>

#include <vector>

namespace spu
{
namespace tools
{

class Thread_pool
{
  protected:
    const size_t n_threads;
    bool initialized;
    std::function<void(const size_t)> func_init;
    std::function<void(const size_t)> func_deinit;
    std::function<void(const size_t)> func_exec;
    bool stop_threads;

  public:
    Thread_pool(const size_t n_threads);
    Thread_pool(const Thread_pool& other);
    virtual ~Thread_pool() = default;
    void set_func_init(std::function<void(const size_t)>& func_init);
    void set_func_deinit(std::function<void(const size_t)>& func_deinit);
    void set_func_exec(std::function<void(const size_t)>& func_exec);
    void unset_func_exec();
    virtual void run(const bool async = false) = 0;
    virtual void run(std::function<void(const size_t)>& func_exec, const bool async = false) = 0;
    virtual void wait() = 0;
    virtual void init(const bool async = false) = 0;
};

}
}

#endif /* THREAD_POOL_HPP_ */
