/*!
 * \file
 * \brief Class sched::Scheduler_from_file.
 */
#ifndef SCHEDULER_FROM_FILE_HPP__
#define SCHEDULER_FROM_FILE_HPP__

#include "Scheduler/Scheduler.hpp"
#include <nlohmann/json.hpp>

namespace spu
{
namespace sched
{

class Scheduler_from_file : public Scheduler
{
  protected:
    // Construct policy function for V1 file
    std::vector<std::pair<size_t, size_t>> solution_from_file;
    std::vector<size_t> sync_buff_sizes_from_file;
    std::vector<bool> sync_active_waitings_from_file;
    std::vector<std::vector<size_t>> puids_from_file;
    void contsruct_policy_v1(nlohmann::json& data, runtime::Sequence& sequence);

    // Construct policy function for V2 file
    std::vector<std::vector<std::string>> p_core_pu_list;
    std::vector<std::vector<std::string>> e_core_pu_list;
    void contsruct_policy_v2(nlohmann::json& data, runtime::Sequence& sequence);

  public:
    Scheduler_from_file(runtime::Sequence& sequence, const std::string filename, uint8_t file_version = 1);
    Scheduler_from_file(runtime::Sequence* sequence, const std::string filename, uint8_t file_version = 1);
    ~Scheduler_from_file() = default;
    virtual void schedule() override;

    uint8_t file_version;
    std::vector<bool> get_thread_pinnings() const override;
    std::vector<size_t> get_sync_buff_sizes() const override;
    std::vector<bool> get_sync_active_waitings() const override;
    std::string get_threads_mapping() const override;
};
} // namespace sched
} // namespace spu

#endif // SCHEDULER_FROM_FILE_HPP__