#ifdef SPU_JULIA

#include <julia.h>
#include <sstream>

#include "Tools/Exception/runtime_error/runtime_error.hpp"
#include "Tools/Julia/Ref_keeper.hpp"
#include "Tools/Julia/utils.hpp"
#include "Tools/Thread/Thread_pool/Julia/Thread_pool_Julia.hpp"
#include "Tools/Thread/Thread_pool/Thread_pool.hpp"

namespace spu
{
namespace tools
{

std::mutex Thread_pool_Julia::jl_adopt_mutex;

void*
Thread_pool_Julia::thread_code(void* args)
{
    Thread_arg* data = (Thread_arg*)args;
    size_t tid = data->tid;
    Thread_pool_Julia* tp = data->tp;

    {
        std::lock_guard<std::mutex> adopt_lock(Thread_pool_Julia::jl_adopt_mutex);
        jl_gcframe_t** gc_struct = jl_adopt_thread();
        (void)gc_struct;
    }

    {
        // Notify while holding 'data->mtx' so 'data' is never accessed after 'data->ready = true' becomes visible.
        std::lock_guard<std::mutex> lk(data->mtx);
        data->ready = true;
        data->cv.notify_one();
    }

    jl_task_t* ct = jl_current_task;

    if (module != NULL) Ref_keeper::init(256);

    tp->func_init(tid);

    while (!tp->stop_threads)
    {
        int8_t gc_state = jl_gc_safe_enter(ct->ptls);
        {
            std::unique_lock<std::mutex> lock(tp->mtx[tid]);
            tp->barrier.arrive();
            tp->cnd[tid].wait(lock);
        }
        jl_gc_safe_leave(ct->ptls, gc_state);

        if (!tp->stop_threads) tp->func_exec(tid);
    }

    tp->func_deinit(tid);
    return NULL;
}

Thread_pool_Julia::Thread_pool_Julia(const size_t n_threads)
  : Thread_pool(n_threads)
  , pool(n_threads)
  , mtx(n_threads)
  , cnd(n_threads)
  , barrier(n_threads)
{
}

Thread_pool_Julia::Thread_pool_Julia(size_t n_threads, std::function<void(const size_t)>& func_init)
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

    void* ret;
    for (auto& t : pool)
    {
        pthread_join(t, &ret);
    }
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

    // Keep 'arg' alive outside the loop so 'arg.mtx' and 'arg.cv' are not destroyed and recreated at the same
    // stack address between iterations while the previous thread is still unlocking 'arg.mtx'.
    Thread_arg arg;
    arg.tp = this;
    for (size_t i = 0; i < n_threads; i++)
    {
        arg.tid = i;
        arg.ready = false;
        pthread_create(&pool[i], NULL, thread_code, (void*)&arg);
        std::unique_lock<std::mutex> lk(arg.mtx);
        arg.cv.wait(lk, [&arg] { return arg.ready; });
    }

    if (!async)
    { // WARNING: jl_gc_enable must be set manually if async
        this->wait();
        jl_gc_enable(this->gc_old_state);
    }

    // return old_state; ?
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
Thread_pool_Julia::run(std::function<void(const size_t)>& func_exec, bool async)
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

}
}

#endif /* SPU_JULIA */
