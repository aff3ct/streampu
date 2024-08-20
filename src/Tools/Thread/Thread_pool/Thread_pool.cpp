#include "Tools/Thread/Thread_pool/Thread_pool.hpp"

using namespace spu;
using namespace spu::tools;

Thread_pool::Thread_pool(const size_t n_threads, std::function<void(const size_t)>& func_init)
  : n_threads(n_threads)
  , initialized(false)
  , pool(new std::vector<std::thread>(n_threads))
  , mtx(new std::vector<std::mutex>(n_threads))
  , cnd(new std::vector<std::condition_variable>(n_threads))
  , func_init(func_init)
  , func_deinit([](const size_t) {})
  , func_exec([](const size_t) { throw tools::unimplemented_error(__FILE__, __LINE__, __func__); })
  , barrier(n_threads)
  , stop_threads(false)
{
    this->init();
}

Thread_pool::Thread_pool(const size_t n_threads)
  : n_threads(n_threads)
  , initialized(false)
  , pool(new std::vector<std::thread>(n_threads))
  , mtx(new std::vector<std::mutex>(n_threads))
  , cnd(new std::vector<std::condition_variable>(n_threads))
  , func_init([](const size_t) {})
  , func_deinit([](const size_t) {})
  , func_exec([](const size_t) { throw tools::unimplemented_error(__FILE__, __LINE__, __func__); })
  , barrier(n_threads)
  , stop_threads(false)
{
}

Thread_pool::Thread_pool(const Thread_pool& other)
  : n_threads(other.n_threads)
  , initialized(false)
  , pool(new std::vector<std::thread>(other.n_threads))
  , mtx(new std::vector<std::mutex>(other.n_threads))
  , cnd(new std::vector<std::condition_variable>(other.n_threads))
  , func_init(other.func_init)
  , func_deinit(other.func_deinit)
  , func_exec(other.func_deinit)
  , barrier(other.barrier)
  , stop_threads(other.stop_threads)
{
    // if (other.initialized) this->init();
}

void
Thread_pool::init(const bool async)
{
    if (this->initialized)
    {
        std::stringstream message;
        message << "This pool of threads has already been initialized and cannot be initialized twice.";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    for (size_t tid = 0; tid < n_threads; tid++)
        (*this->pool)[tid] = std::thread(&Thread_pool::_start_thread, this, tid);

    if (!async)
    {
        this->barrier.wait();
        this->initialized = true;
    }
}

void
Thread_pool::_start_thread(const size_t tid)
{
    this->func_init(tid);

    while (!this->stop_threads)
    {
        std::unique_lock<std::mutex> lock((*this->mtx)[tid]);
        this->barrier.arrive();
        (*this->cnd)[tid].wait(lock);

        if (!this->stop_threads) this->func_exec(tid);
    }

    this->func_deinit(tid);
}

Thread_pool::~Thread_pool()
{
    // stop the threads pool
    this->stop_threads = true;
    for (size_t tid = 0; tid < this->n_threads; tid++)
    {
        std::lock_guard<std::mutex> lock((*this->mtx)[tid]);
        (*this->cnd)[tid].notify_one();
    }
    for (size_t tid = 0; tid < this->n_threads; tid++)
        (*this->pool)[tid].join();
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

void
Thread_pool::run(const bool async)
{
    if (!this->initialized)
    {
        std::stringstream message;
        message << "This pool of threads cannot be run because it has not been initialized.";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    for (size_t tid = 0; tid < n_threads; tid++)
    {
        std::lock_guard<std::mutex> lock((*this->mtx)[tid]);
        (*this->cnd)[tid].notify_one();
    }

    if (!async) this->barrier.wait();
}

void
Thread_pool::run(std::function<void(const size_t)>& set_func_exec, const bool async)
{
    this->set_func_exec(set_func_exec);
    this->run(async);
}

void
Thread_pool::wait()
{
    this->barrier.wait();
    this->initialized = true;
}
