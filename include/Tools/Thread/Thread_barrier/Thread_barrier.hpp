#ifndef THREAD_BARRIER_HPP_
#define THREAD_BARRIER_HPP_

#include <atomic>
#include <cstdint>

namespace spu
{
namespace tools
{

// implementations exist in pthread lib and C++20 stdlib but not in C++11 stdlib :-(
class Thread_barrier
{
  private:
    const uint32_t n_threads;
    std::shared_ptr<std::atomic<uint32_t>> count;

  public:
    inline Thread_barrier(const uint32_t n_threads);
    inline Thread_barrier(const Thread_barrier& other);
    inline ~Thread_barrier() = default;
    inline void arrive();
    inline void reset();
    inline void wait();
};

}
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "Tools/Thread/Thread_barrier/Thread_barrier.hxx"
#endif

#endif /* THREAD_BARRIER_HPP_ */
