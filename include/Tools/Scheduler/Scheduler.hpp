/*!
 * \file
 * \brief Class tools::Scheduler.
 */
#ifndef SCHEDULER_HPP__
#define SCHEDULER_HPP__

#include "Runtime/Pipeline/Pipeline.hpp"
#include "Runtime/Sequence/Sequence.hpp"
#include "Tools/Interface/Interface_reset.hpp"
#include <chrono>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace spu
{
namespace tools
{
struct task_desc_t
{
    runtime::Task* tptr;
    std::chrono::duration<double, std::nano> exec_duration;
};
class Scheduler : public Interface_reset
{
    // friend runtime::Sequence;
  public:
    Scheduler(runtime::Sequence& sequence);
    Scheduler(runtime::Sequence* sequence);
    Scheduler(std::unique_ptr<spu::runtime::Sequence> sequence);
    void profile();
    void print_profiling();

  protected:
    runtime::Sequence* sequence;
    std::vector<task_desc_t> tasks_desc;

  public:
    virtual ~Scheduler() = default;
    virtual runtime::Pipeline* generate_pipeline() = 0;
    virtual void reset();
};
} // namespace tools
} // namespace spu

#endif // SCHEDULER_HPP__