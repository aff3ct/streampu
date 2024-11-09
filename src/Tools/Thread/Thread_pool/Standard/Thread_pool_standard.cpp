#include "Tools/Thread/Thread_pool/Standard/Thread_pool_standard.hpp"

using namespace spu;
using namespace spu::tools;

Thread_pool_standard::Thread_pool_standard(const size_t n_threads)
  : Thread_pool(n_threads)
  , pool(n_threads)
  , mtx(n_threads)
  , cnd(n_threads)
  , barrier(n_threads)
{
}

Thread_pool_standard::Thread_pool_standard(const size_t n_threads, std::function<void(const size_t)>& func_init)
  : Thread_pool_standard(n_threads)
{
    this->set_func_init(func_init);
    this->init();
}

Thread_pool_standard::Thread_pool_standard(const Thread_pool_standard& other)
  : Thread_pool(other)
  , pool(other.n_threads)
  , mtx(other.n_threads)
  , cnd(other.n_threads)
  , barrier(other.barrier)
{
}

void
Thread_pool_standard::init(const bool async)
{
    if (this->initialized)
    {
        std::stringstream message;
        message << "This pool of threads has already been initialized and cannot be initialized twice.";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    for (size_t tid = 0; tid < n_threads; tid++)
        this->pool[tid] = std::thread(&Thread_pool_standard::_start_thread, this, tid);

    if (!async)
    {
        this->barrier.wait();
        this->initialized = true;
    }
}

Thread_pool_standard::~Thread_pool_standard()
{
    // stop the threads pool
    this->stop_threads = true;
    for (size_t tid = 0; tid < this->n_threads; tid++)
    {
        std::lock_guard<std::mutex> lock(this->mtx[tid]);
        this->cnd[tid].notify_one();
    }

    for (size_t tid = 0; tid < n_threads; tid++)
        this->pool[tid].join();
}

void
Thread_pool_standard::run(const bool async)
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
Thread_pool_standard::run(std::function<void(const size_t)>& set_func_exec, const bool async)
{
    this->set_func_exec(set_func_exec);
    this->run(async);
}

void
Thread_pool_standard::wait()
{
    this->barrier.wait();
    this->initialized = true;
}

void
Thread_pool_standard::_start_thread(const size_t tid)
{
    this->func_init(tid);

    while (!this->stop_threads)
    {
        std::unique_lock<std::mutex> lock(this->mtx[tid]);
        this->barrier.arrive();
        this->cnd[tid].wait(lock);

        if (!this->stop_threads) this->func_exec(tid);
    }

    this->func_deinit(tid);
}
