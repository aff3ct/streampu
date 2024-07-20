/*!
 * \file
 * \brief Class sched::Scheduler_OTAC.
 */
#ifndef SCHEDULER_OTAC_HPP__
#define SCHEDULER_OTAC_HPP__

#include "Scheduler/Scheduler.hpp"

namespace spu
{
namespace sched
{

class Scheduler_OTAC : public Scheduler
{
  public:
    Scheduler_OTAC(runtime::Sequence& sequence, const size_t R);
    Scheduler_OTAC(runtime::Sequence* sequence, const size_t R);

  protected:
    const size_t R;

  public:
    ~Scheduler_OTAC() = default;
    runtime::Pipeline* generate_pipeline() override;
};
} // namespace sched
} // namespace spu

#endif // SCHEDULER_OTAC_HPP__