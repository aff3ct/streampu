/*!
 * \file
 * \brief Class tools::Scheduler_OTAC.
 */
#ifndef SCHEDULER_OTAC_HPP__
#define SCHEDULER_OTAC_HPP__

#include "Tools/Scheduler/Scheduler.hpp"

namespace spu
{
namespace tools
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
} // namespace tools
} // namespace spu

#endif // SCHEDULER_OTAC_HPP__