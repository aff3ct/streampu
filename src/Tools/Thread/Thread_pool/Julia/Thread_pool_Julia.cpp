#ifdef SPU_JULIA

#include <julia.h>
#include <sstream>

#include "Tools/Exception/runtime_error/runtime_error.hpp"
#include "Tools/Julia/Julia.hpp"
#include "Tools/Julia/Ref_keeper.hpp"
#include "Tools/Thread/Thread_pool/Julia/Thread_pool_Julia.hpp"
#include "Tools/Thread/Thread_pool/Thread_pool.hpp"

namespace spu
{
namespace tools
{

std::mutex Thread_pool_Julia::jl_adopt_mutex;

Thread_pool_Julia::Thread_pool_Julia(const size_t n_threads)
  : Thread_pool(n_threads)
  , pool(n_threads)
  , mtx(n_threads)
  , cnd(n_threads)
  , barrier(n_threads)
  , ready_init(false)
  , all_adopted(false)
  , gc_old_state(0)
{
}

Thread_pool_Julia::Thread_pool_Julia(const size_t n_threads, std::function<void(const size_t)>& func_init)
  : Thread_pool_Julia(n_threads)
{
    this->set_func_init(func_init);
    this->init();
}

Thread_pool_Julia::Thread_pool_Julia(const Thread_pool_Julia& other)
  : Thread_pool(other)
  , pool(other.n_threads)
  , mtx(other.n_threads)
  , cnd(other.n_threads)
  , barrier(other.barrier)
  , ready_init(false)
  , all_adopted(false)
  , gc_old_state(0)
{
}

Thread_pool_Julia::~Thread_pool_Julia()
{
    this->stop_threads = true;
    for (size_t tid = 0; tid < this->n_threads; tid++)
    {
        std::lock_guard<std::mutex> lock(this->mtx[tid]);
        this->cnd[tid].notify_one();
    }

    for (size_t tid = 0; tid < this->n_threads; tid++)
        if (this->pool[tid].joinable())
            this->pool[tid].join();
}

void
Thread_pool_Julia::init(const bool async)
{
    if (this->initialized)
    {
        std::stringstream message;
        message << "This pool of threads has already been initialized and cannot be initialized twice.";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    this->gc_old_state = jl_gc_enable(0);

    this->all_adopted = false;
    for (size_t tid = 0; tid < this->n_threads; tid++)
    {
        this->ready_init = false;
        this->pool[tid] = std::thread(&Thread_pool_Julia::_start_thread, this, tid);
        std::unique_lock<std::mutex> lock(this->mtx_init);
        this->cnd_init.wait(lock, [this] { return this->ready_init; });
    }

    {
        std::lock_guard<std::mutex> lock(this->mtx_init);
        this->all_adopted = true;
        this->cnd_init.notify_all();
    }

    if (!async)
    { // WARNING: jl_gc_enable must be set manually if async
        this->wait();
        jl_gc_enable(this->gc_old_state);
    }
}

void
Thread_pool_Julia::run(const bool async)
{
    if (!this->initialized)
    {
        std::stringstream message;
        message << "This pool of threads cannot be run because it has not been initialized.";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    for (size_t tid = 0; tid < n_threads; tid++)
    {
        std::lock_guard<std::mutex> lock(this->mtx[tid]);
        this->cnd[tid].notify_one();
    }

    if (!async)
    {
        jl_task_t* ct = jl_current_task;
        int8_t gc_state = 0;
        const bool can_safe = (ct != nullptr && ct->ptls != nullptr);
        if (can_safe)
            gc_state = jl_gc_safe_enter(ct->ptls);

        this->barrier.wait();

        if (can_safe)
            jl_gc_safe_leave(ct->ptls, gc_state);
    }
}

void
Thread_pool_Julia::run(std::function<void(const size_t)>& func_exec, const bool async)
{
    this->set_func_exec(func_exec);
    this->run(async);
}

void
Thread_pool_Julia::wait()
{
    jl_task_t* ct = jl_current_task;
    int8_t gc_state = 0;
    const bool can_safe = (ct != nullptr && ct->ptls != nullptr);
    if (can_safe)
        gc_state = jl_gc_safe_enter(ct->ptls);

    this->barrier.wait();

    if (can_safe)
        jl_gc_safe_leave(ct->ptls, gc_state);

    if (!this->initialized)
    {
        this->initialized = true;
        jl_gc_enable(this->gc_old_state);
    }
}

void
Thread_pool_Julia::_start_thread(const size_t tid)
{
    {
        std::lock_guard<std::mutex> adopt_lock(Thread_pool_Julia::jl_adopt_mutex);
        jl_gcframe_t** gc_struct = jl_adopt_thread();
        (void)gc_struct;
        if (Julia::is_init()) Ref_keeper::init(Julia::get_n_proxies());
    }

    {
        std::unique_lock<std::mutex> lock(this->mtx_init);
        this->ready_init = true;
        this->cnd_init.notify_all();
        this->cnd_init.wait(lock, [this] { return this->all_adopted; });
    }

    jl_task_t* ct = jl_current_task;

    this->func_init(tid);

    while (!this->stop_threads)
    {
        int8_t gc_state = jl_gc_safe_enter(ct->ptls);
        {
            std::unique_lock<std::mutex> lock(this->mtx[tid]);
            this->barrier.arrive();
            this->cnd[tid].wait(lock);
        }
        jl_gc_safe_leave(ct->ptls, gc_state);

        if (!this->stop_threads) this->func_exec(tid);
    }

    this->func_deinit(tid);
}

}
}

#endif /* SPU_JULIA */
