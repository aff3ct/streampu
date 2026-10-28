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
#include <string>
#include <utility>
#include <vector>

namespace spu
{
namespace sched
{
struct task_desc_t
{
    runtime::Task* tptr;
    std::vector<std::chrono::duration<double, std::nano>> exec_duration;
};
class Scheduler : public tools::Interface_reset
{
  private:
    runtime::Sequence* sequence;

  protected:
    std::vector<task_desc_t> tasks_desc;
    std::vector<size_t> profiled_puids;
    std::vector<std::string> profiling_summary;
    std::vector<std::pair<size_t, size_t>> solution;

    Scheduler(runtime::Sequence& sequence);
    Scheduler(runtime::Sequence* sequence);

    void _profile(const int puid, const size_t n_exec);

  public:
    void profile(const size_t n_exec = 100);
    void profile(const std::vector<size_t>& puids, const size_t n_exec = 100);
    void print_profiling(std::ostream& stream = std::cout);
    const std::vector<task_desc_t>& get_profiling();
    virtual ~Scheduler() = default;
    runtime::Pipeline* generate_pipeline();
    std::vector<std::pair<size_t, size_t>> get_solution();
    virtual void reset() override;
    virtual void schedule() = 0;
    runtime::Pipeline* instantiate_pipeline(const std::vector<size_t> synchro_buffer_sizes,
                                            const std::vector<bool> synchro_active_waitings,
                                            const std::vector<bool> thread_pinings,
                                            const std::string& pinning_policy);
    runtime::Pipeline* instantiate_pipeline(const size_t buffer_size = 1,
                                            const bool active_wait = false,
                                            const bool thread_pining = false,
                                            const std::string& pinning_policy = "");
    virtual std::vector<bool> get_thread_pinnings() const;
    virtual std::vector<size_t> get_sync_buff_sizes() const;
    virtual std::vector<bool> get_sync_active_waitings() const;
    virtual std::string get_threads_mapping() const;
    size_t get_n_alloc_ressources() const;
    virtual double get_throughput_est() const; // return the estimated number of streams per second
};
} // namespace sched
} // namespace spu

#endif // SCHEDULER_HPP__