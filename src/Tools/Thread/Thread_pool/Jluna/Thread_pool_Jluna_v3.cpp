#ifdef SPU_JLUNA

#include "Tools/Thread/Thread_pool/Jluna/Thread_pool_Jluna_v3.hpp"
#include <iostream>

using namespace spu;
using namespace spu::tools;

Thread_pool_Jluna_v3::Thread_pool_Jluna_v3(const size_t n_threads)
  : Thread_pool(n_threads)
  , pool()
  , jl_cnd(n_threads, jluna::Base.safe_eval("return Base.Threads.Condition()"))
  , jl_unsafe_lock(jluna::unsafe::get_function(jluna::Base, jluna::Symbol("lock")))
  , jl_unsafe_unlock(jluna::unsafe::get_function(jluna::Base, jluna::Symbol("unlock")))
  , jl_unsafe_wait(jluna::unsafe::get_function(jluna::Base, jluna::Symbol("wait")))
  , jl_unsafe_notify(jluna::unsafe::get_function(jluna::Base, jluna::Symbol("notify")))
  , barrier(n_threads)
{
}

Thread_pool_Jluna_v3::Thread_pool_Jluna_v3(const size_t n_threads, std::function<void(const size_t)>& func_init)
  : Thread_pool_Jluna_v3(n_threads)
{
    this->set_func_init(func_init);
    this->init();
}

Thread_pool_Jluna_v3::Thread_pool_Jluna_v3(const Thread_pool_Jluna_v3& other)
  : Thread_pool(other)
  , pool()
  , jl_cnd(n_threads, jluna::Base.safe_eval("return Base.Threads.Condition()"))
  , jl_unsafe_lock(jluna::unsafe::get_function(jluna::Base, jluna::Symbol("lock")))
  , jl_unsafe_unlock(jluna::unsafe::get_function(jluna::Base, jluna::Symbol("unlock")))
  , jl_unsafe_wait(jluna::unsafe::get_function(jluna::Base, jluna::Symbol("wait")))
  , jl_unsafe_notify(jluna::unsafe::get_function(jluna::Base, jluna::Symbol("notify")))
  , barrier(n_threads)
{
}

Thread_pool_Jluna_v3::~Thread_pool_Jluna_v3()
{
    // stop the threads pool
    this->stop_threads = true;
    for (size_t tid = 0; tid < this->n_threads; tid++)
    {
        jluna::unsafe::call(this->jl_unsafe_lock, static_cast<jluna::unsafe::Value*>(this->jl_cnd[tid]));
        jluna::unsafe::call(this->jl_unsafe_notify, static_cast<jluna::unsafe::Value*>(this->jl_cnd[tid]));
        jluna::unsafe::call(this->jl_unsafe_unlock, static_cast<jluna::unsafe::Value*>(this->jl_cnd[tid]));
    }

    for (size_t tid = 0; tid < n_threads; tid++)
        this->pool[tid].join();
}

void
Thread_pool_Jluna_v3::_start_thread(const size_t tid)
{
    this->func_init(tid);
    while (!this->stop_threads)
    {
        jluna::unsafe::call(this->jl_unsafe_lock, static_cast<jluna::unsafe::Value*>(this->jl_cnd[tid]));
        this->barrier.arrive(tid + 1);
        jluna::unsafe::call(this->jl_unsafe_wait, static_cast<jluna::unsafe::Value*>(this->jl_cnd[tid]));

        if (!this->stop_threads) this->func_exec(tid);
    }
    this->func_deinit(tid);
}

void
Thread_pool_Jluna_v3::init(const bool async)
{
    if (this->initialized)
    {
        std::stringstream message;
        message << "This pool of threads has already been initialized and cannot be initialized twice.";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    for (size_t tid = 0; tid < n_threads; tid++)
    {
        std::function<void(size_t)> jl_fstart = [this](const size_t tid) -> void { this->_start_thread(tid); };
        this->pool.push_back(jluna::ThreadPool::create(jl_fstart, tid));
        this->pool.back().schedule();
    }

    if (!async)
    {
        this->barrier.wait();
        this->initialized = true;
    }
}

void
Thread_pool_Jluna_v3::run(const bool async)
{
    if (!this->initialized)
    {
        std::stringstream message;
        message << "This pool of threads cannot be run because it has not been initialized.";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    for (size_t tid = 0; tid < n_threads; tid++)
    {
        jluna::unsafe::call(this->jl_unsafe_lock, static_cast<jluna::unsafe::Value*>(this->jl_cnd[tid]));
        jluna::unsafe::call(this->jl_unsafe_notify, static_cast<jluna::unsafe::Value*>(this->jl_cnd[tid]));
        jluna::unsafe::call(this->jl_unsafe_unlock, static_cast<jluna::unsafe::Value*>(this->jl_cnd[tid]));
    }

    if (!async) this->barrier.wait();
}

void
Thread_pool_Jluna_v3::run(std::function<void(const size_t)>& set_func_exec, const bool async)
{
    this->set_func_exec(set_func_exec);
    this->run(async);
}

void
Thread_pool_Jluna_v3::wait()
{
    this->barrier.wait();
    this->initialized = true;
}

#endif /* SPU_JLUNA */
