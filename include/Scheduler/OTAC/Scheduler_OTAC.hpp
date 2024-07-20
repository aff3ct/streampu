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
    std::vector<std::pair<int, int>> solution;

  public:
    ~Scheduler_OTAC() = default;
    runtime::Pipeline* generate_pipeline() override;
    std::vector<std::pair<int, int>> get_solution() override;

  protected:
    // Find if there is a solution
    //// Inputs: - chain
    ////         - R (number of resources)
    //// Outputs: - P updated if there is a solution
    ////          - a composition solution (n,r)
    //            - True/False if there is/is no solution
    void SOLVE(const std::vector<task_desc_t>& chain,
               const size_t R,
               double& P,
               std::vector<std::pair<int, int>>& solution);

    // Compute a solution or determine if there is no solution
    // Inputs: - chain
    //         - R (number of resources)
    //         - P period (reciprocal throughput)
    // Outputs: - P updated if there is a solution
    //          - a composition solution (n,r) updated
    //          - True/False if there is/is no solution
    bool PROBE(const std::vector<task_desc_t>& chain,
               const size_t R,
               double& P,
               std::vector<std::pair<int, int>>& solution);
};
} // namespace sched
} // namespace spu

#endif // SCHEDULER_OTAC_HPP__