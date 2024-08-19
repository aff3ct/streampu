#ifndef BARRIER_HPP_
#define BARRIER_HPP_

#include <atomic>
#include <cstdint>

namespace spu
{
namespace tools
{

class Barrier
{
  private:
    const uint32_t n_threads;
    std::shared_ptr<std::atomic<uint32_t>> count;

  public:
    inline Barrier(const uint32_t n_threads);
    inline Barrier(const Barrier& other);
    inline ~Barrier() = default;
    inline void arrive();
    inline void reset();
    inline void wait();
};

}
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "Tools/Threading/Barrier/Barrier.hxx"
#endif

#endif /* BARRIER_HPP_ */
