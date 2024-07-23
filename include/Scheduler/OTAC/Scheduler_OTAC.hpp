/*!
 * \file
 * \brief Class sched::Scheduler_OTAC.
 */
#ifndef SCHEDULER_OTAC_HPP__
#define SCHEDULER_OTAC_HPP__

#include <thread>
#include "Scheduler/Scheduler.hpp"

namespace spu
{
namespace sched
{

class Scheduler_OTAC : public Scheduler
{
  protected:
    const size_t R; /**< The maximum number of resources allowed to perform the scheduling. */
    double P;       /**< The period or the reciprocal throughput of the pipeline. */

  public:
    Scheduler_OTAC(runtime::Sequence& sequence, const size_t R = std::thread::hardware_concurrency());
    Scheduler_OTAC(runtime::Sequence* sequence, const size_t R = std::thread::hardware_concurrency());
    ~Scheduler_OTAC() = default;
    virtual void schedule() override;
    double get_period() const;
    virtual void reset() override;

  private:
    /**
     * Find if there is a solution.
     * @param chain An input vector of tasks descriptor with profiled time.
     * @param R The input number of resources available.
     * @param P The output periodicity (reciprocal of pipeline throughput).
     * @param solution The output solution in the form of a vector of (n,r) pairs.
     * @return A boolean depending if there is a solution or not.
     */
    void SOLVE(const std::vector<task_desc_t>& chain,
               const size_t R,
               double& P,
               std::vector<std::pair<size_t, size_t>>& solution);

    /**
     * Compute a solution or determine if there is no solution.
     * @param chain An input vector of tasks descriptor with profiled time.
     * @param R The input number of available resources.
     * @param P The input periodicity (reciprocal of pipeline throughput), this parameter can be updated to a new
     *          periodicity value.
     * @param solution The output solution in the form of a vector of (n,r) pairs.
     * @return A boolean depending if there is a solution or not.
     */
    bool PROBE(const std::vector<task_desc_t>& chain,
               const size_t R,
               double& P,
               std::vector<std::pair<size_t, size_t>>& solution);
};
} // namespace sched
} // namespace spu

#endif // SCHEDULER_OTAC_HPP__