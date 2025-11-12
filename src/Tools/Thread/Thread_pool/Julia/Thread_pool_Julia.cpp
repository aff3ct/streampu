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

void*
Thread_pool_Julia::thread_code(void* args)
{
    struct _args* data = (struct _args*)args;
    size_t tid = data->tid;
    Thread_pool_Julia* tp = data->tp;
    sem_post(&data->sem); // args and data cesases to exist

    pthread_mutex_lock(&data->lock);
    jl_gcframe_t** gc_struct = jl_adopt_thread();
    pthread_mutex_unlock(&data->lock);
    jl_task_t* ct = jl_current_task;
    int8_t gc_state = jl_gc_safe_enter(ct->ptls);

    if (module != NULL) Ref_keeper::init(256);

    tp->func_init(tid);

    while (!tp->stop_threads)
    {
        std::unique_lock<std::mutex> lock(tp->mtx[tid]);
        tp->barrier.arrive();
        tp->cnd[tid].wait(lock);

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

    struct _args args; // Some args for thread initialization
    args.tp = this;
    pthread_mutex_init(&args.lock, NULL);
    sem_init(&args.sem, 0, 0);
    for (size_t i = 0; i < n_threads; i++)
    {
        args.tid = i;
        pthread_create(&pool[i], NULL, thread_code, (void*)&args);
        sem_wait(&args.sem);
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

    if (!async) this->barrier.wait();
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
    this->barrier.wait();
    if(!this->initialized) {
		this->initialized = true;
		jl_gc_enable(this->gc_old_state);
	}
}

}
}

#endif /* SPU_JULIA */
