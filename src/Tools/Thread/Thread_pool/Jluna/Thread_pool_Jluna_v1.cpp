#ifdef SPU_JLUNA

#include "Tools/Thread/Thread_pool/Jluna/Thread_pool_Jluna_v1.hpp"
#include <iostream>

using namespace spu;
using namespace spu::tools;

Thread_pool_Jluna_v1::Thread_pool_Jluna_v1(const size_t n_threads)
  : Thread_pool(n_threads)
  , pool()
{
}

Thread_pool_Jluna_v1::Thread_pool_Jluna_v1(const size_t n_threads, std::function<void(const size_t)>& func_init)
  : Thread_pool_Jluna_v1(n_threads)
{
    this->set_func_init(func_init);
    this->init();
}

Thread_pool_Jluna_v1::Thread_pool_Jluna_v1(const Thread_pool_Jluna_v1& other)
  : Thread_pool(other)
  , pool()
{
}

Thread_pool_Jluna_v1::~Thread_pool_Jluna_v1()
{
    this->pool.clear();
    for (size_t tid = 0; tid < n_threads; tid++)
    {
        std::function<void(size_t)> jl_fdeinit = [this](const size_t tid) -> void { this->func_deinit(tid); };
        this->pool.push_back(jluna::ThreadPool::create(jl_fdeinit, tid));
        this->pool.back().schedule();
    }

    for (size_t tid = 0; tid < n_threads; tid++)
        this->pool[tid].join();
}

void
Thread_pool_Jluna_v1::init(const bool async)
{
    this->pool.clear();
    for (size_t tid = 0; tid < n_threads; tid++)
    {
        std::function<void(size_t)> jl_finit = [this](const size_t tid) -> void { this->func_init(tid); };
        this->pool.push_back(jluna::ThreadPool::create(jl_finit, tid));
        this->pool.back().schedule();
    }

    if (!async)
    {
        for (size_t tid = 0; tid < n_threads; tid++)
            this->pool[tid].join();
        this->initialized = true;
    }
}

void
Thread_pool_Jluna_v1::run(const bool async)
{
    if (!this->initialized)
    {
        std::stringstream message;
        message << "This pool of threads cannot be run because it has not been initialized.";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    this->pool.clear();
    for (size_t tid = 0; tid < n_threads; tid++)
    {
        std::function<void(size_t)> jl_fexec = [this](const size_t tid) -> void { this->func_exec(tid); };
        this->pool.push_back(jluna::ThreadPool::create(jl_fexec, tid));
        this->pool.back().schedule();
    }

    if (!async)
        for (size_t tid = 0; tid < n_threads; tid++)
            this->pool[tid].join();
}

void
Thread_pool_Jluna_v1::run(std::function<void(const size_t)>& set_func_exec, const bool async)
{
    this->set_func_exec(set_func_exec);
    this->run(async);
}

void
Thread_pool_Jluna_v1::wait()
{
    for (size_t tid = 0; tid < n_threads; tid++)
        this->pool[tid].join();
    this->initialized = true;
}

#endif /* SPU_JLUNA */
