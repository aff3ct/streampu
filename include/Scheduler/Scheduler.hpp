/*!
 * \file
 * \brief Class sched::Scheduler.
 */
#ifndef SCHEDULER_HPP__
#define SCHEDULER_HPP__

#include "Runtime/Pipeline/Pipeline.hpp"
#include "Runtime/Sequence/Sequence.hpp"
#include "Tools/Interface/Interface_reset.hpp"
#include <chrono>
#include <iostream>
#include <utility>
#include <vector>

namespace spu
{
namespace sched
{
struct task_desc_t
{
    runtime::Task* tptr;
    std::chrono::duration<double, std::nano> exec_duration;
};
class Scheduler : public tools::Interface_reset
{
  private:
    runtime::Sequence* sequence;
    std::vector<task_desc_t> tasks_desc;
    std::vector<std::pair<size_t, size_t>> solution;

  protected:
    Scheduler(runtime::Sequence& sequence);
    Scheduler(runtime::Sequence* sequence);

  public:
    void profile();
    void print_profiling(std::ostream& stream = std::cout);
    const std::vector<task_desc_t>& get_profiling();
    virtual ~Scheduler() = default;
    runtime::Pipeline* generate_pipeline();
    std::vector<std::pair<size_t, size_t>> get_solution();
    virtual void reset();
    virtual void schedule(const std::vector<task_desc_t>& tasks_desc,
                          std::vector<std::pair<size_t, size_t>>& solution) = 0;

  private:
    runtime::Pipeline* instantiate_pipeline();
};
} // namespace sched
} // namespace spu

#endif // SCHEDULER_HPP__