/*!
 * \file
 * \brief Class sched::Scheduler_from_file.
 */
#ifndef SCHEDULER_FROM_FILE_HPP__
#define SCHEDULER_FROM_FILE_HPP__

#include "Scheduler/Scheduler.hpp"

namespace spu
{
namespace sched
{

class Scheduler_from_file : public Scheduler
{
  protected:
    std::vector<std::pair<size_t, size_t>> solution_from_file;
    std::vector<std::vector<size_t>> puids_from_file;

  public:
    Scheduler_from_file(runtime::Sequence& sequence, const std::string filename);
    Scheduler_from_file(runtime::Sequence* sequence, const std::string filename);
    ~Scheduler_from_file() = default;
    virtual void schedule() override;

    std::string perform_threads_mapping() const override;
    std::vector<bool> get_threads_pinning() const override;
};
} // namespace sched
} // namespace spu

#endif // SCHEDULER_FROM_FILE_HPP__