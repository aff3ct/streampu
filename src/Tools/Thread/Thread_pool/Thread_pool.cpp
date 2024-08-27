#include <iostream>

#include "Tools/Exception/exception.hpp"
#include "Tools/Thread/Thread_pool/Thread_pool.hpp"

using namespace spu;
using namespace spu::tools;

Thread_pool::Thread_pool(const size_t n_threads)
  : n_threads(n_threads)
  , initialized(false)
  , func_init([](const size_t) {})
  , func_deinit([](const size_t) {})
  , func_exec([](const size_t) { throw tools::unimplemented_error(__FILE__, __LINE__, __func__); })
  , stop_threads(false)
{
}

Thread_pool::Thread_pool(const Thread_pool& other)
  : n_threads(other.n_threads)
  , initialized(false)
  , func_init(other.func_init)
  , func_deinit(other.func_deinit)
  , func_exec(other.func_deinit)
  , stop_threads(other.stop_threads)
{
}

void
Thread_pool::set_func_init(std::function<void(const size_t)>& func_init)
{
    this->func_init = func_init;
}

void
Thread_pool::set_func_deinit(std::function<void(const size_t)>& func_deinit)
{
    this->func_deinit = func_deinit;
}

void
Thread_pool::set_func_exec(std::function<void(const size_t)>& func_exec)
{
    this->func_exec = func_exec;
}

void
Thread_pool::unset_func_exec()
{
    this->func_exec = [](const size_t) { throw tools::unimplemented_error(__FILE__, __LINE__, __func__); };
}
