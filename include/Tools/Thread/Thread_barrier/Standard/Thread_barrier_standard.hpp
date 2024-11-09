#ifndef THREAD_BARRIER_STANDARD_HPP_
#define THREAD_BARRIER_STANDARD_HPP_

#include <atomic>
#include <cstdint>

namespace spu
{
namespace tools
{

// implementations exist in pthread lib and C++20 stdlib but not in C++11 stdlib :-(
class Thread_barrier_standard
{
  private:
    const uint32_t n_threads;
    std::atomic<uint32_t> count;

  public:
    inline Thread_barrier_standard(const uint32_t n_threads);
    inline Thread_barrier_standard(const Thread_barrier_standard& other);
    inline ~Thread_barrier_standard() = default;
    inline void arrive();
    inline void reset();
    inline void wait();
};

}
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "Tools/Thread/Thread_barrier/Standard/Thread_barrier_standard.hxx"
#endif

#endif /* THREAD_BARRIER_STANDARD_HPP_ */
