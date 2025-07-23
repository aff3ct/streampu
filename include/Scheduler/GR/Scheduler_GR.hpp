/*!
 * \file
 * \brief Class sched::Scheduler_GR.
 */
#ifndef SCHEDULER_GR_HPP__
#define SCHEDULER_GR_HPP__

#include "Scheduler/Scheduler.hpp"
#include <thread>

namespace spu
{
namespace sched
{

class Scheduler_GR : public Scheduler
{
  protected:
    const size_t R; /**< The maximum number of replicate per stage. */

  public:
    Scheduler_GR(runtime::Sequence& sequence, const size_t R = std::thread::hardware_concurrency());
    Scheduler_GR(runtime::Sequence* sequence, const size_t R = std::thread::hardware_concurrency());
    ~Scheduler_GR() = default;
    virtual void schedule() override;
};
} // namespace sched
} // namespace spu

#endif // SCHEDULER_GR_HPP__