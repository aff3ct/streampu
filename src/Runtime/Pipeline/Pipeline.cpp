#include <cassert>
#include <fstream>
#include <thread>
#include <tuple>
#include <utility>

#include "Module/Stateful/Adaptor/Adaptor_m_to_n.hpp"
#include "Runtime/Pipeline/Pipeline.hpp"
#include "Tools/Exception/exception.hpp"
#include "Tools/Interface/Interface_waiting.hpp"
#include "Tools/Thread/Thread_pinning/Thread_pinning_utils.hpp"
#include "Tools/Thread/Thread_pool/Standard/Thread_pool_standard.hpp"

using namespace spu;
using namespace spu::runtime;

// Pipeline
// ::Pipeline(const runtime::Task &first,
//            const runtime::Task &last,
//            const std::vector<std::pair<std::vector<const runtime::Task*>, std::vector<const runtime::Task*>>>
//            &sep_stages, const std::vector<size_t> &n_threads, const std::vector<size_t> &synchro_buffer_sizes, const
//            std::vector<bool> &synchro_active_waiting, const std::vector<bool> &thread_pinning, const
//            std::vector<std::vector<size_t>> &puids)
// : original_sequence(first, last, 1),
//   stages(sep_stages.size()),
//   adaptors(sep_stages.size() -1),
//   saved_firsts_tasks_id(sep_stages.size()),
//   saved_lasts_tasks_id(sep_stages.size()),
//   bound_adaptors(false)
// {
// 	this->init<const runtime::Task>(first,
// 	                                &last,
// 	                                sep_stages,
// 	                                n_threads,
// 	                                synchro_buffer_sizes,
// 	                                synchro_active_waiting,
// 	                                thread_pinning,
// 	                                puids);
// }

// Pipeline
// ::Pipeline(const runtime::Task &first,
//            const std::vector<std::pair<std::vector<const runtime::Task*>, std::vector<const runtime::Task*>>>
//            &sep_stages, const std::vector<size_t> &n_threads, const std::vector<size_t> &synchro_buffer_sizes, const
//            std::vector<bool> &synchro_active_waiting, const std::vector<bool> &thread_pinning, const
//            std::vector<std::vector<size_t>> &puids)
// : original_sequence(first, 1),
//   stages(sep_stages.size()),
//   adaptors(sep_stages.size() -1),
//   saved_firsts_tasks_id(sep_stages.size()),
//   saved_lasts_tasks_id(sep_stages.size()),
//   bound_adaptors(false)
// {
// 	const runtime::Task* last = nullptr;
// 	this->init<const runtime::Task>(first,
// 	                                last,
// 	                                sep_stages,
// 	                                n_threads,
// 	                                synchro_buffer_sizes,
// 	                                synchro_active_waiting,
// 	                                thread_pinning,
// 	                                puids);
// }

Pipeline
::Pipeline(const std::vector<runtime::Task*> &firsts,
           const std::vector<runtime::Task*> &lasts,
           const std::vector<std::tuple<std::vector<runtime::Task*>, std::vector<runtime::Task*>, std::vector<runtime::Task*>>> &sep_stages,
           const std::vector<size_t> &n_threads,
           const std::vector<size_t> &synchro_buffer_sizes,
           const std::vector<bool> &synchro_active_waiting,
           const std::vector<bool> &thread_pinning,
           const std::vector<std::vector<size_t>> &puids/*,
           const std::vector<bool> &tasks_inplace*/)
: original_sequence(firsts, lasts, 1),
  stages(sep_stages.size()),
  adaptors(sep_stages.size() -1),
  saved_firsts_tasks_id(sep_stages.size()),
  saved_lasts_tasks_id(sep_stages.size()),
  bound_adaptors(false),
  auto_stop(true)
{
    this->init<runtime::Task>(
      firsts, lasts, sep_stages, n_threads, synchro_buffer_sizes, synchro_active_waiting, thread_pinning, puids
      /*, tasks_inplace*/);
}

Pipeline
::Pipeline(const std::vector<runtime::Task*> &firsts,
           const std::vector<runtime::Task*> &lasts,
           const std::vector<std::pair<std::vector<runtime::Task*>, std::vector<runtime::Task*>>> &sep_stages,
           const std::vector<size_t> &n_threads,
           const std::vector<size_t> &synchro_buffer_sizes,
           const std::vector<bool> &synchro_active_waiting,
           const std::vector<bool> &thread_pinning,
           const std::vector<std::vector<size_t>> &puids/*,
           const std::vector<bool> &tasks_inplace*/)
: original_sequence(firsts, lasts, 1),
  stages(sep_stages.size()),
  adaptors(sep_stages.size() -1),
  saved_firsts_tasks_id(sep_stages.size()),
  saved_lasts_tasks_id(sep_stages.size()),
  bound_adaptors(false),
  auto_stop(true)
{
    std::vector<std::tuple<std::vector<runtime::Task*>, std::vector<runtime::Task*>, std::vector<runtime::Task*>>>
      sep_stages_bis;
    for (auto& sep_stage : sep_stages)
        sep_stages_bis.push_back(std::make_tuple(sep_stage.first, sep_stage.second, std::vector<runtime::Task*>()));

    this->init<runtime::Task>(firsts,
	                          lasts,
	                          sep_stages_bis,
	                          n_threads,
	                          synchro_buffer_sizes,
	                          synchro_active_waiting,
	                          thread_pinning, puids/*,
	                          tasks_inplace*/);
}

Pipeline
::Pipeline(const std::vector<runtime::Task*> &firsts,
           const std::vector<std::tuple<std::vector<runtime::Task*>, std::vector<runtime::Task*>, std::vector<runtime::Task*>>> &sep_stages,
           const std::vector<size_t> &n_threads,
           const std::vector<size_t> &synchro_buffer_sizes,
           const std::vector<bool> &synchro_active_waiting,
           const std::vector<bool> &thread_pinning,
           const std::vector<std::vector<size_t>> &puids/*,
           const std::vector<bool> &tasks_inplace*/)
: Pipeline(firsts,
           {},
           sep_stages,
           n_threads,
           synchro_buffer_sizes,
           synchro_active_waiting,
           thread_pinning, puids/*,
           tasks_inplace*/)
{
}

Pipeline
::Pipeline(const std::vector<runtime::Task*> &firsts,
           const std::vector<std::pair<std::vector<runtime::Task*>, std::vector<runtime::Task*>>> &sep_stages,
           const std::vector<size_t> &n_threads,
           const std::vector<size_t> &synchro_buffer_sizes,
           const std::vector<bool> &synchro_active_waiting,
           const std::vector<bool> &thread_pinning,
           const std::vector<std::vector<size_t>> &puids/*,
           const std::vector<bool> &tasks_inplace*/)
: Pipeline(firsts,
           {},
           sep_stages,
           n_threads,
           synchro_buffer_sizes,
           synchro_active_waiting,
           thread_pinning, puids/*,
           tasks_inplace*/)
{
}

Pipeline
::Pipeline(runtime::Task &first,
           runtime::Task &last,
           const std::vector<std::pair<std::vector<runtime::Task*>, std::vector<runtime::Task*>>> &sep_stages,
           const std::vector<size_t> &n_threads,
           const std::vector<size_t> &synchro_buffer_sizes,
           const std::vector<bool> &synchro_active_waiting,
           const std::vector<bool> &thread_pinning,
           const std::vector<std::vector<size_t>> &puids/*,
           const std::vector<bool> &tasks_inplace*/)
: Pipeline({&first},
           {&last},
           sep_stages,
           n_threads,
           synchro_buffer_sizes,
           synchro_active_waiting,
           thread_pinning, puids/*,
           tasks_inplace*/)
{
}

Pipeline
::Pipeline(runtime::Task &first,
           const std::vector<std::pair<std::vector<runtime::Task*>, std::vector<runtime::Task*>>> &sep_stages,
           const std::vector<size_t> &n_threads,
           const std::vector<size_t> &synchro_buffer_sizes,
           const std::vector<bool> &synchro_active_waiting,
           const std::vector<bool> &thread_pinning,
           const std::vector<std::vector<size_t>> &puids/*,
           const std::vector<bool> &tasks_inplace*/)
: Pipeline({&first},
           sep_stages,
           n_threads,
           synchro_buffer_sizes,
           synchro_active_waiting,
           thread_pinning, puids/*,
           tasks_inplace*/)
{
}

//========================== Pipeline constructors with new version thread pinning =====================================
Pipeline
::Pipeline(const std::vector<runtime::Task*> &firsts,
           const std::vector<runtime::Task*> &lasts,
           const std::vector<std::tuple<std::vector<runtime::Task*>, std::vector<runtime::Task*>, std::vector<runtime::Task*>>> &sep_stages,
           const std::vector<size_t> &n_threads,
           const std::vector<size_t> &synchro_buffer_sizes,
           const std::vector<bool> &synchro_active_waiting,
           const std::vector<bool> &thread_pinning,
           const std::string &pipeline_pinning_policy/*,
           const std::vector<bool> &tasks_inplace*/)
: original_sequence(firsts, lasts, 1),
  stages(sep_stages.size()),
  adaptors(sep_stages.size() -1),
  saved_firsts_tasks_id(sep_stages.size()),
  saved_lasts_tasks_id(sep_stages.size()),
  bound_adaptors(false),
  auto_stop(true)
{
    this->init<runtime::Task>(firsts,
	                          lasts,
	                          sep_stages,
	                          n_threads,
	                          synchro_buffer_sizes,
	                          synchro_active_waiting,
	                          thread_pinning, {}, pipeline_pinning_policy/*,
	                          tasks_inplace*/);
}

Pipeline
::Pipeline(const std::vector<runtime::Task*> &firsts,
           const std::vector<runtime::Task*> &lasts,
           const std::vector<std::pair<std::vector<runtime::Task*>, std::vector<runtime::Task*>>> &sep_stages,
           const std::vector<size_t> &n_threads,
           const std::vector<size_t> &synchro_buffer_sizes,
           const std::vector<bool> &synchro_active_waiting,
           const std::vector<bool> &thread_pinning,
           const std::string &pipeline_pinning_policy/*,
           const std::vector<bool> &tasks_inplace*/)
: original_sequence(firsts, lasts, 1),
  stages(sep_stages.size()),
  adaptors(sep_stages.size() -1),
  saved_firsts_tasks_id(sep_stages.size()),
  saved_lasts_tasks_id(sep_stages.size()),
  bound_adaptors(false),
  auto_stop(true)
{
    std::vector<std::tuple<std::vector<runtime::Task*>, std::vector<runtime::Task*>, std::vector<runtime::Task*>>>
      sep_stages_bis;
    for (auto& sep_stage : sep_stages)
        sep_stages_bis.push_back(std::make_tuple(sep_stage.first, sep_stage.second, std::vector<runtime::Task*>()));

    this->init<runtime::Task>(firsts,
	                          lasts,
	                          sep_stages_bis,
	                          n_threads,
	                          synchro_buffer_sizes,
	                          synchro_active_waiting,
	                          thread_pinning, {}, pipeline_pinning_policy/*,
	                          tasks_inplace*/);
}

Pipeline
::Pipeline(const std::vector<runtime::Task*> &firsts,
           const std::vector<std::tuple<std::vector<runtime::Task*>, std::vector<runtime::Task*>, std::vector<runtime::Task*>>> &sep_stages,
           const std::vector<size_t> &n_threads,
           const std::vector<size_t> &synchro_buffer_sizes,
           const std::vector<bool> &synchro_active_waiting,
           const std::vector<bool> &thread_pinning,
           const std::string &pipeline_pinning_policy/*,
           const std::vector<bool> &tasks_inplace*/)
: Pipeline(firsts,
           {},
           sep_stages,
           n_threads,
           synchro_buffer_sizes,
           synchro_active_waiting,
           thread_pinning, pipeline_pinning_policy/*,
           tasks_inplace*/)
{
}

Pipeline
::Pipeline(const std::vector<runtime::Task*> &firsts,
           const std::vector<std::pair<std::vector<runtime::Task*>, std::vector<runtime::Task*>>> &sep_stages,
           const std::vector<size_t> &n_threads,
           const std::vector<size_t> &synchro_buffer_sizes,
           const std::vector<bool> &synchro_active_waiting,
           const std::vector<bool> &thread_pinning,
           const std::string &pipeline_pinning_policy/*,
           const std::vector<bool> &tasks_inplace*/)
: Pipeline(firsts,
           {},
           sep_stages,
           n_threads,
           synchro_buffer_sizes,
           synchro_active_waiting,
           thread_pinning, pipeline_pinning_policy/*,
           tasks_inplace*/)
{
}

Pipeline
::Pipeline(runtime::Task &first,
           runtime::Task &last,
           const std::vector<std::pair<std::vector<runtime::Task*>, std::vector<runtime::Task*>>> &sep_stages,
           const std::vector<size_t> &n_threads,
           const std::vector<size_t> &synchro_buffer_sizes,
           const std::vector<bool> &synchro_active_waiting,
           const std::vector<bool> &thread_pinning,
           const std::string &pipeline_pinning_policy/*,
           const std::vector<bool> &tasks_inplace*/)
: Pipeline({&first},
           {&last},
           sep_stages,
           n_threads,
           synchro_buffer_sizes,
           synchro_active_waiting,
           thread_pinning, pipeline_pinning_policy/*,
           tasks_inplace*/)
{
}

Pipeline
::Pipeline(runtime::Task &first,
           const std::vector<std::pair<std::vector<runtime::Task*>, std::vector<runtime::Task*>>> &sep_stages,
           const std::vector<size_t> &n_threads,
           const std::vector<size_t> &synchro_buffer_sizes,
           const std::vector<bool> &synchro_active_waiting,
           const std::vector<bool> &thread_pinning,
           const std::string &pipeline_pinning_policy/*,
           const std::vector<bool> &tasks_inplace*/)
: Pipeline({&first},
           sep_stages,
           n_threads,
           synchro_buffer_sizes,
           synchro_active_waiting,
           thread_pinning,
           pipeline_pinning_policy/*,
           tasks_inplace*/)
{
}

Pipeline::~Pipeline()
{
    this->unbind_adaptors();
}

std::vector<Sequence*>
Pipeline::get_stages()
{
    std::vector<Sequence*> stages;
    for (auto& stage : this->stages)
        stages.push_back(stage.get());
    return stages;
}

Sequence&
Pipeline::operator[](const size_t stage_id)
{
    assert(stage_id < this->stages.size());
    return *this->stages[stage_id];
}

template<class TA>
runtime::Sequence*
create_sequence(const std::vector<TA*>& firsts,
                const std::vector<TA*>& lasts,
                const std::vector<TA*>& exclusions,
                const size_t& n_threads,
                const bool& thread_pinning,
                const std::vector<size_t>& puids,
                const bool& tasks_inplace)
{
    throw tools::unimplemented_error(__FILE__, __LINE__, __func__);
}

template<>
runtime::Sequence*
create_sequence<const runtime::Task>(const std::vector<const runtime::Task*>& firsts,
                                     const std::vector<const runtime::Task*>& lasts,
                                     const std::vector<const runtime::Task*>& exclusions,
                                     const size_t& n_threads,
                                     const bool& thread_pinning,
                                     const std::vector<size_t>& puids,
                                     const bool& /*tasks_inplace*/)
{
    return new runtime::Sequence(firsts, lasts, exclusions, n_threads, thread_pinning, puids, false);
}

template<>
Sequence*
create_sequence<runtime::Task>(const std::vector<runtime::Task*>& firsts,
                               const std::vector<runtime::Task*>& lasts,
                               const std::vector<runtime::Task*>& exclusions,
                               const size_t& n_threads,
                               const bool& thread_pinning,
                               const std::vector<size_t>& puids,
                               const bool& tasks_inplace)
{
    return new runtime::Sequence(firsts, lasts, exclusions, n_threads, thread_pinning, puids, tasks_inplace, false);
}

// Init and sequence creation for second pinning version
template<class TA>
runtime::Sequence*
create_sequence(const std::vector<TA*>& firsts,
                const std::vector<TA*>& lasts,
                const std::vector<TA*>& exclusions,
                const size_t& n_threads,
                const bool& thread_pinning,
                const std::string& pipeline_pinning_policy,
                const bool& tasks_inplace)
{
    throw tools::unimplemented_error(__FILE__, __LINE__, __func__);
}

template<>
runtime::Sequence*
create_sequence<const runtime::Task>(const std::vector<const runtime::Task*>& firsts,
                                     const std::vector<const runtime::Task*>& lasts,
                                     const std::vector<const runtime::Task*>& exclusions,
                                     const size_t& n_threads,
                                     const bool& thread_pinning,
                                     const std::string& pipeline_pinning_policy,
                                     const bool& /*tasks_inplace*/)
{
    return new runtime::Sequence(firsts, lasts, exclusions, n_threads, thread_pinning, pipeline_pinning_policy, false);
}

template<>
Sequence*
create_sequence<runtime::Task>(const std::vector<runtime::Task*>& firsts,
                               const std::vector<runtime::Task*>& lasts,
                               const std::vector<runtime::Task*>& exclusions,
                               const size_t& n_threads,
                               const bool& thread_pinning,
                               const std::string& pipeline_pinning_policy,
                               const bool& tasks_inplace)
{
    return new runtime::Sequence(
      firsts, lasts, exclusions, n_threads, thread_pinning, pipeline_pinning_policy, tasks_inplace, false);
}

template <class TA>
void Pipeline
::init(const std::vector<TA*> &/*firsts*/,
       const std::vector<TA*> &/*lasts*/,
       const std::vector<std::tuple<std::vector<TA*>,std::vector<TA*>,std::vector<TA*>>> &sep_stages,
       const std::vector<size_t> &n_threads,
       const std::vector<size_t> &synchro_buffer_sizes,
       const std::vector<bool> &synchro_active_waiting,
       const std::vector<bool> &thread_pinning,
       const std::vector<std::vector<size_t>> &puids,
       const std::string &pipeline_pinning_policy/*,
       const std::vector<bool> &tasks_inplace*/)
{
    if (sep_stages.size() != n_threads.size() && n_threads.size() != 0)
    {
        std::stringstream message;
        message << "'n_threads.size()' has to be equal to 'sep_stages.size()' or equal to '0' ('n_threads.size()' = "
                << n_threads.size() << " , 'sep_stages.size()' = " << sep_stages.size() << ").";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    if (sep_stages.size() != synchro_buffer_sizes.size() + 1 && synchro_buffer_sizes.size() != 0)
    {
        std::stringstream message;
        message << "'synchro_buffer_sizes.size()' has to be equal to 'sep_stages.size() -1' or equal to '0' "
                << "('synchro_buffer_sizes.size()' = " << synchro_buffer_sizes.size()
                << " , 'sep_stages.size()' = " << sep_stages.size() << ").";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    if (sep_stages.size() != synchro_active_waiting.size() + 1 && synchro_active_waiting.size() != 0)
    {
        std::stringstream message;
        message << "'synchro_active_waiting.size()' has to be equal to 'sep_stages.size() -1' or equal to '0' "
                << "('synchro_active_waiting.size()' = " << synchro_active_waiting.size()
                << " , 'sep_stages.size()' = " << sep_stages.size() << ").";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    if (sep_stages.size() != thread_pinning.size() && thread_pinning.size() != 0)
    {
        std::stringstream message;
        message << "'thread_pinning.size()' has to be equal to 'sep_stages.size()' or equal to '0' ("
                << "'thread_pinning.size()' = " << thread_pinning.size()
                << " , 'sep_stages.size()' = " << sep_stages.size() << ").";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    if (sep_stages.size() != puids.size() && puids.size() != 0)
    {
        std::stringstream message;
        message << "'puids.size()' has to be equal to 'sep_stages.size()' or equal to '0' ('puids.size()' = "
                << puids.size() << " , 'sep_stages.size()' = " << sep_stages.size() << ").";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    // if (sep_stages.size() != tasks_inplace.size() && tasks_inplace.size() != 0)
    // {
    // 	std::stringstream message;
    // 	message << "'tasks_inplace.size()' has to be equal to 'sep_stages.size()' or equal to '0' ('"
    // 	        << "tasks_inplace.size()' = " << tasks_inplace.size() << " , 'sep_stages.size()' = "
    // 	        << sep_stages.size() << ").";
    // 	throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    // }

    // Creating a vector of pinning policies for each sequence
    std::vector<std::string> sequences_pinning_policies;
    if (!pipeline_pinning_policy.empty())
        sequences_pinning_policies =
          tools::Thread_pinning_utils::pipeline_parser_unpacker(pipeline_pinning_policy, sep_stages.size());

    for (size_t s = 0; s < sep_stages.size(); s++)
    {
        const std::vector<TA*>& stage_firsts = std::get<0>(sep_stages[s]);
        const std::vector<TA*>& stage_lasts = std::get<1>(sep_stages[s]);
        const std::vector<TA*>& stage_exclusions = std::get<2>(sep_stages[s]);
        const size_t stage_n_threads = n_threads.size() ? n_threads[s] : 1;
        const bool stage_thread_pinning = thread_pinning.size() ? thread_pinning[s] : false;
        const std::vector<size_t> stage_puids = puids.size() ? puids[s] : std::vector<size_t>();
        const std::string sequence_pinning_policy =
          sequences_pinning_policies.size() ? sequences_pinning_policies[s] : "";
        const bool stage_tasks_inplace = /*tasks_inplace.size() ? tasks_inplace[s] :*/ true;
        try
        {
            if (pipeline_pinning_policy.empty())
                this->stages[s].reset(create_sequence<TA>(stage_firsts,
                                                          stage_lasts,
                                                          stage_exclusions,
                                                          stage_n_threads,
                                                          stage_thread_pinning,
                                                          stage_puids,
                                                          stage_tasks_inplace));
            else
                this->stages[s].reset(create_sequence<TA>(stage_firsts,
                                                          stage_lasts,
                                                          stage_exclusions,
                                                          stage_n_threads,
                                                          stage_thread_pinning,
                                                          sequence_pinning_policy,
                                                          stage_tasks_inplace));
        }
        catch (const tools::control_flow_error& e)
        {
            std::stringstream message;
            message << "Invalid control flow error on stage " << s
                    << " (perhaps a switcher's tasks were separated between different stages)." << std::endl
                    << e.what();
            throw tools::control_flow_error(__FILE__, __LINE__, __func__, message.str());
        }
        this->stages[s]->is_part_of_pipeline = true;
    }

    // verify that the sequential sequence is equivalent to the pipeline sequence
    auto ref_tasks = this->original_sequence.get_tasks_per_threads()[0];
    auto cur_tasks = this->get_tasks_per_threads()[0];

    if (ref_tasks.size() != cur_tasks.size())
    {
        std::ofstream f1("dbg_ref_sequence.dot");
        this->original_sequence.export_dot(f1);
        std::ofstream f2("dbg_cur_pipeline.dot");
        this->export_dot(f2);

        std::stringstream message;
        message << "'ref_tasks.size()' has to be equal to 'cur_tasks.size()' ('ref_tasks.size()' = " << ref_tasks.size()
                << ", 'cur_tasks.size()' = " << cur_tasks.size() << ").";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    for (size_t ta = 0; ta < cur_tasks.size(); ta++)
    {
        if (std::find(ref_tasks.begin(), ref_tasks.end(), cur_tasks[ta]) == ref_tasks.end())
        {
            std::ofstream f1("dbg_ref_sequence.dot");
            this->original_sequence.export_dot(f1);
            std::ofstream f2("dbg_cur_pipeline.dot");
            this->export_dot(f2);

            std::stringstream message;
            message << "'cur_tasks[ta]' is not contained in the 'ref_tasks' vector ('ta' = " << ta
                    << ", 'cur_tasks[ta]' = " << +cur_tasks[ta]
                    << ", 'cur_tasks[ta]->get_name()' = " << cur_tasks[ta]->get_name() << ").";
            throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
        }
    }

    // Adding adaptors to pipeline stages
    this->create_adaptors(synchro_buffer_sizes, synchro_active_waiting);
    this->bind_adaptors();

    // Allocating memory for stages
    for (auto stage : this->stages)
    {
        stage->allocate_this_sequence_memory();
    }

    this->thread_pool.reset(new tools::Thread_pool_standard(this->stages.size() - 1));
    this->thread_pool->init(); // threads are spawned here
}

void
Pipeline::create_adaptors(const std::vector<size_t>& synchro_buffer_sizes,
                          const std::vector<bool>& synchro_active_waiting)
{
    //                     sck out addr     occ     stage   tsk id  sck id
    std::vector<std::tuple<runtime::Socket*, size_t, size_t, size_t, size_t>> out_sck_orphans;

    // for all the stages in the pipeline
    for (size_t sta = 0; sta < this->stages.size() - 1; sta++)
    {
        // ------------------------------------------------------------------------------------------------------------
        // --------------------------------------------------------------- collect orphan output sockets in stage 'sta'
        // ------------------------------------------------------------------------------------------------------------
        std::vector<std::vector<runtime::Task*>> tasks_per_threads = this->stages[sta]->get_tasks_per_threads();
        // for all the threads in the current stage
        // for (size_t t = 0; t < tasks_per_threads.size(); t++)
        size_t t = 0;
        {
            // for all the tasks in the stage
            for (size_t tsk_id = 0; tsk_id < tasks_per_threads[t].size(); tsk_id++)
            {
                auto tsk = tasks_per_threads[t][tsk_id];
                // for all the sockets of the tasks
                for (size_t sck_id = 0; sck_id < tsk->sockets.size(); sck_id++)
                {
                    auto sck = tsk->sockets[sck_id];
                    // if the current socket is an output or forward socket type
                    if (sck->get_type() == socket_t::SOUT || sck->get_type() == socket_t::SFWD)
                    {
                        // for all the bounded sockets to the current socket
                        for (auto bsck : sck->get_bound_sockets())
                        {
                            // check if the task of the bounded socket is not in the current stage
                            if (std::find(tasks_per_threads[t].begin(),
                                          tasks_per_threads[t].end(),
                                          &bsck->get_task()) == tasks_per_threads[t].end())
                            {
                                // check the position of the socket in the orphans
                                size_t pos = 0;
                                for (; pos < out_sck_orphans.size(); pos++)
                                    if (std::get<0>(out_sck_orphans[pos]) == sck.get()) break;

                                if (pos == out_sck_orphans.size())
                                    out_sck_orphans.push_back(std::make_tuple(sck.get(), 1, sta, tsk_id, sck_id));
                                else
                                    std::get<1>(out_sck_orphans[pos])++;
                            }
                        }
                    }
                }
            }
        }

        // ------------------------------------------------------------------------------------------------------------
        // -------------------------------------- collect orphan input sockets in stage 'sta +1' and create connections
        // ------------------------------------------------------------------------------------------------------------
        tasks_per_threads = this->stages[sta + 1]->get_tasks_per_threads();
        // for all the threads in the current stage
        // for (size_t t = 0; t < tasks_per_threads.size(); t++)
        {
            // for all the tasks in the stage
            for (size_t tsk_id = 0; tsk_id < tasks_per_threads[t].size(); tsk_id++)
            {
                auto tsk = tasks_per_threads[t][tsk_id];
                // ----------------------------------------- manage socket to socket bindings (with fake input sockets)
                // for all the sockets of the tasks
                for (size_t sck_id = 0; sck_id < tsk->sockets.size(); sck_id++)
                {
                    auto sck = tsk->sockets[sck_id];
                    // if the current socket is an input or forward socket type
                    if (sck->get_type() == socket_t::SIN || sck->get_type() == socket_t::SFWD)
                    {
                        runtime::Socket* bsck = nullptr;
                        try
                        {
                            // get output bounded socket
                            bsck = &sck->get_bound_socket(); // can throw if there is no bounded socket
                        }
                        catch (const std::exception&)
                        {
                        }
                        if (bsck != nullptr)
                        {
                            // check if the task of the bounded socket is not in the current stage
                            if (std::find(tasks_per_threads[t].begin(),
                                          tasks_per_threads[t].end(),
                                          &bsck->get_task()) == tasks_per_threads[t].end())
                            {
                                // check the position of the bounded socket in the orphans
                                size_t pos = 0;
                                for (; pos < out_sck_orphans.size(); pos++)
                                    if (std::get<0>(out_sck_orphans[pos]) == bsck) break;

                                if (pos < out_sck_orphans.size())
                                {
                                    auto sck_out = std::get<0>(out_sck_orphans[pos]);
                                    auto sck_in = sck.get();
                                    auto unbind_sout_pos = std::distance(sck_out->get_bound_sockets().begin(),
                                                                         std::find(sck_out->get_bound_sockets().begin(),
                                                                                   sck_out->get_bound_sockets().end(),
                                                                                   sck_in));
                                    this->sck_orphan_binds.push_back(
                                      std::make_pair(std::make_tuple(std::get<0>(out_sck_orphans[pos]),
                                                                     std::get<2>(out_sck_orphans[pos]),
                                                                     std::get<3>(out_sck_orphans[pos]),
                                                                     std::get<4>(out_sck_orphans[pos]),
                                                                     unbind_sout_pos),
                                                     std::make_tuple(sck_in, sta + 1, tsk_id, sck_id, nullptr)));
                                }
                            }
                        }
                    }
                }
                // ------------------------------------------- manage socket to task bindings (with fake input sockets)
                for (size_t sck_id = 0; sck_id < tsk->fake_input_sockets.size(); sck_id++)
                {
                    auto sck = tsk->fake_input_sockets[sck_id];
                    runtime::Socket* bsck = nullptr;
                    try
                    {
                        // get output bounded socket
                        bsck = &sck->get_bound_socket(); // can throw if there is no bounded socket
                    }
                    catch (const std::exception&)
                    {
                    }
                    if (bsck != nullptr)
                    {
                        // check if the task of the bounded socket is not in the current stage
                        if (std::find(tasks_per_threads[t].begin(), tasks_per_threads[t].end(), &bsck->get_task()) ==
                            tasks_per_threads[t].end())
                        {
                            // check the position of the bounded socket in the orphans
                            size_t pos = 0;
                            for (; pos < out_sck_orphans.size(); pos++)
                                if (std::get<0>(out_sck_orphans[pos]) == bsck) break;

                            if (pos < out_sck_orphans.size())
                            {
                                auto sck_out = std::get<0>(out_sck_orphans[pos]);
                                auto sck_in = sck.get();
                                auto unbind_sout_pos = std::distance(sck_out->get_bound_sockets().begin(),
                                                                     std::find(sck_out->get_bound_sockets().begin(),
                                                                               sck_out->get_bound_sockets().end(),
                                                                               sck_in));
                                this->sck_orphan_binds.push_back(
                                  std::make_pair(std::make_tuple(std::get<0>(out_sck_orphans[pos]),
                                                                 std::get<2>(out_sck_orphans[pos]),
                                                                 std::get<3>(out_sck_orphans[pos]),
                                                                 std::get<4>(out_sck_orphans[pos]),
                                                                 unbind_sout_pos),
                                                 std::make_tuple(nullptr, sta + 1, tsk_id, sck_id, tsk)));
                            }
                        }
                    }
                }
                // ----------------------------------------------------------------------------------------------------
            }
        }
    }

    // ----------------------------------------------------------------------------------------------------------------
    // ----------------------------------------------------------------------------------------------- prints for debug
    // ----------------------------------------------------------------------------------------------------------------
    // std::cout << "Orphan output sockets list:" << std::endl;
    // for (auto &sck : out_sck_orphans)
    // {
    //  auto sck_out_name = std::get<0>(sck)->get_name();
    //  auto tsk_out_name = std::get<0>(sck)->get_task().get_name();
    //  auto sck_out_occ  = std::get<1>(sck);
    //  auto tsk_out_sta  = std::get<2>(sck);
    //  auto tsk_out_id   = std::get<3>(sck);
    //  auto sck_out_id   = std::get<4>(sck);

    //  std::cout << "  " << tsk_out_name << "[" << sck_out_name << "] (stage " << tsk_out_sta << ", " << sck_out_occ
    //            << " occurrences, tsk id = " << tsk_out_id << ", sck id = " << sck_out_id << ")" << std::endl;
    // }

    // std::cout << std::endl << "Detected socket binds:" << std::endl;
    // for (auto &bind : this->sck_orphan_binds)
    // {
    //  auto sck_out_name = std::get<0>(bind.first)->get_name();
    //  auto tsk_out_name = std::get<0>(bind.first)->get_task().get_name();
    //  auto tsk_out_sta  = std::get<1>(bind.first);
    //  auto tsk_out_id   = std::get<2>(bind.first);
    //  auto sck_out_id   = std::get<3>(bind.first);
    //  auto sck_out_ubp  = std::get<4>(bind.first);

    //  auto sck_in_name = std::get<0>(bind.second)->get_name();
    //  auto tsk_in_name = std::get<0>(bind.second)->get_task().get_name();
    //  auto tsk_in_sta  = std::get<1>(bind.second);
    //  auto tsk_in_id   = std::get<2>(bind.second);
    //  auto sck_in_id   = std::get<3>(bind.second);

    //  std::cout << "  " << tsk_out_name << "[" << sck_out_name << "] (stage " << tsk_out_sta << ", tsk id = "
    //                    << tsk_out_id << ", sck id = " << sck_out_id << ", ubp = " << sck_out_ubp << ")"  << " -> "
    //                    << tsk_in_name  << "[" << sck_in_name  << "] (stage " << tsk_in_sta  << ", tsk id = "
    //                    << tsk_in_id  << ", sck id = " << sck_in_id  << ")" << std::endl;
    // }

    // ----------------------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------ create adaptors
    // ----------------------------------------------------------------------------------------------------------------
    auto sck_orphan_binds_cpy = this->sck_orphan_binds;
    module::Adaptor_m_to_n* adp = nullptr;
    std::map<runtime::Socket*, size_t> sck_to_adp_sck_id;
    for (size_t sta = 0; sta < this->stages.size(); sta++)
    {
        const auto n_threads = this->stages[sta]->get_n_threads();
        std::vector<std::vector<runtime::Task*>> tasks_per_threads = this->stages[sta]->get_tasks_per_threads();

        // ------------------------------------------------------------------------------------------------------------
        // ----------------------------------------------------------------------------------------------- pull adaptor
        // ------------------------------------------------------------------------------------------------------------
        if (sta > 0)
        {
            assert(adp != nullptr);
            //                               sck out addr      stage   tsk id  sck id  unbind_pos
            std::vector<std::pair<std::tuple<runtime::Socket*, size_t, size_t, size_t, size_t>,
                                  //                               sck in addr       stage   tsk id  sck id  tsk in addr
                                  std::tuple<runtime::Socket*, size_t, size_t, size_t, runtime::Task*>>>
              sck_orphan_binds_new;

            for (size_t t = 0; t < n_threads; t++)
            {
                module::Adaptor_m_to_n* cur_adp = (t == 0) ? adp : adp->clone();
                if (t > 0) cur_adp->add_puller();

                for (auto& t : cur_adp->tasks)
                    t->set_fast(true);
                if (t > 0)
                {
                    this->adaptors[sta - 1].second.push_back(std::unique_ptr<module::Adaptor_m_to_n>(cur_adp));
                    cur_adp->set_custom_name("Adp_m_to_n_" + std::to_string(sta - 1));
                }

                auto task_pull = &(*cur_adp)("pull");

                sck_orphan_binds_new.clear();
                for (auto& bind : sck_orphan_binds_cpy)
                {
                    auto tsk_out_sta = std::get<1>(bind.first);
                    if (tsk_out_sta < sta)
                    {
                        auto tsk_in_sta = std::get<1>(bind.second);
                        if (tsk_in_sta == sta)
                        {
                            auto sck_out_ptr = std::get<0>(bind.first);
                            auto priority = std::get<4>(bind.first);
                            auto tsk_in_id = std::get<2>(bind.second);
                            auto sck_in_id = std::get<3>(bind.second);
                            runtime::Socket* sck_in = nullptr;
                            runtime::Task* tsk_in = nullptr;
                            if (std::get<0>(bind.second) != nullptr) // if socket to socket binding
                                sck_in = tasks_per_threads[t][tsk_in_id]->sockets[sck_in_id].get();
                            else // if socket to task binding
                                tsk_in = tasks_per_threads[t][tsk_in_id];
                            this->adaptors_binds.push_back(std::make_tuple(
                              task_pull->sockets[sck_to_adp_sck_id[sck_out_ptr]].get(), sck_in, priority, tsk_in));
                        }
                        else
                            sck_orphan_binds_new.push_back(bind);
                    }
                    else
                        sck_orphan_binds_new.push_back(bind);
                }

                if (t > 0) this->stages[sta]->all_modules[t].push_back(cur_adp);
            }
            this->saved_firsts_tasks_id[sta] = this->stages[sta]->firsts_tasks_id;
            sck_orphan_binds_cpy = sck_orphan_binds_new;

            adp->alloc_buffers();
        }

        // ------------------------------------------------------------------------------------------------------------
        // ----------------------------------------------------------------------------------------------- push adaptor
        // ------------------------------------------------------------------------------------------------------------
        std::map<runtime::Socket*, size_t> sck_to_adp_sck_id_new;
        if (sta < this->stages.size() - 1)
        {
            std::vector<size_t> adp_n_elmts;
            std::vector<std::type_index> adp_datatype;
            size_t adp_buffer_size = synchro_buffer_sizes.size() ? synchro_buffer_sizes[sta] : 1;
            bool adp_active_waiting = synchro_active_waiting.size() ? synchro_active_waiting[sta] : false;
            size_t adp_n_frames = 1;

            // a map to remember if a passed socket points already to the same memory space
            std::map<void*, size_t> fwd_source;

            std::vector<runtime::Socket*> passed_scks_out;
            for (auto& bind : sck_orphan_binds_cpy)
            {
                auto tsk_out_sta = std::get<1>(bind.first);
                if (tsk_out_sta <= sta)
                {
                    auto sck_out = std::get<0>(bind.first);
                    if (std::find(passed_scks_out.begin(), passed_scks_out.end(), sck_out) == passed_scks_out.end())
                    {
                        // avoid the creation of new adaptor sockets for forward sockets pointing to the same memory
                        // space
                        auto sck_out_dptr = (void*)sck_out->get_dataptr<int8_t>();
                        assert(sck_out_dptr != nullptr);
                        if (fwd_source.find(sck_out_dptr) == fwd_source.end())
                        {
                            fwd_source[sck_out_dptr] = 1;
                            adp_n_frames = sck_out->get_task().get_module().get_n_frames();
                            adp_n_elmts.push_back(sck_out->get_n_elmts() / adp_n_frames);
                            adp_datatype.push_back(sck_out->get_datatype());
                        }
                        passed_scks_out.push_back(sck_out);
                    }
                }
            }
            passed_scks_out.clear();

            // allocate the adaptor for the first thread
            adp = new module::Adaptor_m_to_n(adp_n_elmts, adp_datatype, adp_buffer_size, adp_active_waiting);
            adp->set_n_frames(adp_n_frames);

            for (size_t t = 0; t < n_threads; t++)
            {
                module::Adaptor_m_to_n* cur_adp = (t == 0) ? adp : adp->clone();
                cur_adp->set_custom_name("Adp_m_to_n_" + std::to_string(sta));
                if (t > 0) cur_adp->add_pusher();
                this->adaptors[sta].first.push_back(std::unique_ptr<module::Adaptor_m_to_n>(cur_adp));
                auto task_push = &(*cur_adp)("push");

                std::map<void*, size_t> fwd_source;
                sck_to_adp_sck_id_new.clear();
                size_t adp_sck_id = 0;
                for (auto& bind : sck_orphan_binds_cpy) // bind standard task to last adaptor
                {
                    auto tsk_out_sta = std::get<1>(bind.first);

                    if (tsk_out_sta <= sta)
                    {
                        auto sck_out_ptr = std::get<0>(bind.first);
                        auto sck_out_dptr = (void*)sck_out_ptr->get_dataptr<int8_t>();
                        assert(sck_out_dptr != nullptr);

                        if (std::find(passed_scks_out.begin(),
                                      passed_scks_out.end(),
                                      sck_out_ptr) == passed_scks_out.end() &&
                            fwd_source.find(sck_out_dptr) == fwd_source.end()) // <= the latest condition is here to
                                                                               //    avoid to bind adaptor sockets two
                                                                               //    times the same memory space
                                                                               //    (usefull in the case of multiple
                                                                               //    fwd sockets pointing to the same
                                                                               //    memory address)
                        {
                            if (tsk_out_sta == sta)
                            {
                                auto tsk_out_id = std::get<2>(bind.first);
                                auto sck_out_id = std::get<3>(bind.first);
                                auto sck_out = tasks_per_threads[t][tsk_out_id]->sockets[sck_out_id];
                                auto priority = std::get<4>(bind.first);
                                sck_to_adp_sck_id_new[sck_out_ptr] = adp_sck_id;
                                this->adaptors_binds.push_back(
                                  std::make_tuple(sck_out.get(),
                                                  task_push->sockets[adp_sck_id++].get(),
                                                  priority,
                                                  nullptr)); // <= only socket to socket binding is possible here
                            }
                            else // if (tsk_out_sta < sta) // bind prev. adaptor to last adaptor
                            {
                                auto tsk_out_id = 1;
                                auto sck_out_id = sck_to_adp_sck_id[sck_out_ptr];
                                sck_to_adp_sck_id_new[sck_out_ptr] = adp_sck_id;
                                auto adp_prev =
                                  t == 0 ? this->adaptors[sta - 1].first[0] : this->adaptors[sta - 1].second[t - 1];
                                auto sck_out = (*adp_prev)[tsk_out_id].sockets[sck_out_id];
                                auto priority = std::get<4>(bind.first);
                                this->adaptors_binds.push_back(
                                  std::make_tuple(sck_out.get(),
                                                  task_push->sockets[adp_sck_id++].get(),
                                                  priority,
                                                  nullptr)); // <= only socket to socket binding is possible here
                            }

                            fwd_source[sck_out_dptr] = 1; // remember that this memory space has been connected to the
                                                          // adaptor once
                            passed_scks_out.push_back(sck_out_ptr);
                        }
                    }
                }
                passed_scks_out.clear();
            }
            this->saved_lasts_tasks_id[sta] = this->stages[sta]->lasts_tasks_id;
        }
        sck_to_adp_sck_id = sck_to_adp_sck_id_new;
    }
}

void
Pipeline::bind_adaptors()
{
    this->_bind_adaptors(true);
}

void
Pipeline::_bind_adaptors(const bool bind_adaptors)
{
    if (!this->bound_adaptors)
    {
        for (size_t sta = 0; sta < this->stages.size(); sta++)
        {
            const auto n_threads = this->stages[sta]->get_n_threads();

            // --------------------------------------------------------------------------------------------------------
            // ------------------------------------------------------------------------------------------- pull adaptor
            // --------------------------------------------------------------------------------------------------------
            if (sta > 0)
            {
                for (size_t t = 0; t < n_threads; t++)
                {
                    module::Adaptor_m_to_n* cur_adp =
                      t > 0 ? adaptors[sta - 1].second[t - 1].get() : adaptors[sta - 1].first[0].get();

                    if (t > 0 || sta == this->stages.size() - 1) // add the adaptor to the current stage
                        this->stages[sta]->all_modules[t].push_back(cur_adp);

                    auto task_pull = &(*cur_adp)("pull");

                    auto ss = this->stages[sta]->sequences[t]->get_contents();
                    assert(ss != nullptr);
                    ss->tasks.insert(ss->tasks.begin(), task_pull);
                    ss->processes.insert(ss->processes.begin(),
                                         [task_pull]() -> const int*
                                         {
                                             task_pull->exec();
                                             const int* status = task_pull->sockets.back()->get_dataptr<const int>();
                                             return status;
                                         });
                    this->stages[sta]->update_tasks_id(t);
                }
                this->stages[sta]->firsts_tasks_id.clear();
                this->stages[sta]->firsts_tasks_id.push_back(0);
                this->stages[sta]->n_tasks++;
            }

            // --------------------------------------------------------------------------------------------------------
            // ------------------------------------------------------------------------------------------- push adaptor
            // --------------------------------------------------------------------------------------------------------
            if (sta < this->stages.size() - 1)
            {
                size_t last_task_id = 0;
                for (size_t t = 0; t < n_threads; t++)
                {
                    module::Adaptor_m_to_n* cur_adp = adaptors[sta].first[t].get();

                    // add the adaptor to the current stage
                    this->stages[sta]->all_modules[t].push_back(cur_adp);

                    auto task_push = &(*cur_adp)("push");

                    auto ss = this->stages[sta]->get_last_subsequence(t);
                    assert(ss != nullptr);
                    ss->tasks.push_back(task_push);
                    ss->processes.push_back(
                      [task_push]() -> const int*
                      {
                          task_push->exec();
                          const int* status = task_push->sockets.back()->get_dataptr<const int>();
                          return status;
                      });
                    last_task_id = ss->tasks_id[ss->tasks_id.size() - 1] + 1;
                    ss->tasks_id.push_back(last_task_id);
                }
                this->stages[sta]->lasts_tasks_id.clear();
                this->stages[sta]->lasts_tasks_id.push_back(last_task_id);
                this->stages[sta]->n_tasks++;
            }
            this->stages[sta]->update_firsts_and_lasts_tasks();
        }

        // ------------------------------------------------------------------------------------------------------------
        // ---------------------------------------------------------------------------------------------- bind adaptors
        // ------------------------------------------------------------------------------------------------------------
        for (auto& bind : this->sck_orphan_binds)
        {
            auto sck_out = std::get<0>(bind.first);
            auto sck_in = std::get<0>(bind.second);
            if (sck_in != nullptr) // if socket to socket unbinding
                sck_in->unbind(*sck_out);
            else // if socket to task unbinding
            {
                auto tsk_in = std::get<4>(bind.second);
                assert(tsk_in != nullptr);
                tsk_in->unbind(*sck_out);
            }
        }

        if (bind_adaptors)
        {
            for (auto& bind : this->adaptors_binds)
            {
                auto sck_out = std::get<0>(bind);
                auto sck_in = std::get<1>(bind);
                auto priority = std::get<2>(bind);
                if (sck_in != nullptr) // if socket to socket binding
                    sck_in->_bind(*sck_out, priority);
                else // if socket to task binding
                {
                    auto tsk_in = std::get<3>(bind);
                    assert(tsk_in != nullptr);
                    tsk_in->_bind(*sck_out, priority);
                }
            }
        }

        this->bound_adaptors = true;
    }
}

void
Pipeline::unbind_adaptors()
{
    this->_unbind_adaptors(true);
}

void
Pipeline::_unbind_adaptors(const bool bind_orphans)
{
    if (this->bound_adaptors)
    {
        for (size_t sta = 0; sta < this->stages.size(); sta++)
        {
            const auto n_threads = this->stages[sta]->get_n_threads();

            // --------------------------------------------------------------------------------------------------------
            // ------------------------------------------------------------------------------------------- pull adaptor
            // --------------------------------------------------------------------------------------------------------
            if (sta > 0)
            {
                for (size_t t = 0; t < n_threads; t++)
                {
                    if (t > 0 || sta == this->stages.size() - 1) // rm the adaptor to the current stage
                        this->stages[sta]->all_modules[t].pop_back();

                    auto ss = this->stages[sta]->sequences[t]->get_contents();
                    assert(ss != nullptr);
                    ss->tasks.erase(ss->tasks.begin());
                    ss->processes.erase(ss->processes.begin());
                    this->stages[sta]->update_tasks_id(t);
                }
                this->stages[sta]->firsts_tasks_id = this->saved_firsts_tasks_id[sta];
                this->stages[sta]->n_tasks--;
            }

            // --------------------------------------------------------------------------------------------------------
            // ------------------------------------------------------------------------------------------- push adaptor
            // --------------------------------------------------------------------------------------------------------
            if (sta < this->stages.size() - 1)
            {
                for (size_t t = 0; t < n_threads; t++)
                {
                    // rm the adaptor to the current stage
                    this->stages[sta]->all_modules[t].pop_back();

                    auto ss = this->stages[sta]->get_last_subsequence(t);
                    assert(ss != nullptr);
                    ss->tasks.pop_back();
                    ss->processes.pop_back();
                    ss->tasks_id.pop_back();
                }
                this->stages[sta]->lasts_tasks_id = this->saved_lasts_tasks_id[sta];
                this->stages[sta]->n_tasks--;
            }
            this->stages[sta]->update_firsts_and_lasts_tasks();
        }

        // ------------------------------------------------------------------------------------------------------------
        // -------------------------------------------------------------------------------------------- unbind adaptors
        // ------------------------------------------------------------------------------------------------------------
        for (auto& bind : this->adaptors_binds)
        {
            auto sck_out = std::get<0>(bind);
            auto sck_in = std::get<1>(bind);
            if (sck_in != nullptr) // if socket to socket unbinding
                sck_in->unbind(*sck_out);
            else // if socket to task unbinding
            {
                auto tsk_in = std::get<3>(bind);
                assert(tsk_in != nullptr);
                tsk_in->unbind(*sck_out);
            }
        }

        if (bind_orphans)
        {
            for (auto& bind : this->sck_orphan_binds)
            {
                auto sck_out = std::get<0>(bind.first);
                auto priority = std::get<4>(bind.first);
                auto sck_in = std::get<0>(bind.second);
                if (sck_in != nullptr) // if socket to socket binding
                    sck_in->_bind(*sck_out, priority);
                else // if socket to task binding
                {
                    auto tsk_in = std::get<4>(bind.second);
                    assert(tsk_in != nullptr);
                    tsk_in->_bind(*sck_out, priority);
                }
            }
        }

        this->bound_adaptors = false;
    }
}

void
Pipeline::exec(const std::vector<std::function<bool(const std::vector<const int*>&)>>& stop_conditions)
{
    if (stop_conditions.size() != this->stages.size() && stop_conditions.size() != 1)
    {
        std::stringstream message;
        message << "'stop_conditions.size()' has to be equal to 'stages.size()' or to 1 ('stop_conditions.size()' = "
                << stop_conditions.size() << ", 'stages.size()' = " << stages.size() << ").";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    if (!this->bound_adaptors)
    {
        std::stringstream message;
        message << "'bound_adaptors' has to be true to execute the pipeline.";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    // ----------------------------------------------------------------------------------------------------------------
    auto& stages = this->stages;
    std::vector<const std::function<bool(const std::vector<const int*>&)>*> stop_condition_vec(stages.size() - 1,
                                                                                               nullptr);
    if (stop_conditions.size() == stages.size())
        for (size_t s = 0; s < stages.size() - 1; s++)
            stop_condition_vec[s] = &stop_conditions[s];

    std::function<void(const size_t)> func_exec = [&stages, &stop_condition_vec](const size_t tid)
    {
        size_t s = tid;
        if (stop_condition_vec[s])
            stages[s]->exec(*(stop_condition_vec[s]));
        else
            stages[s]->exec();

        // send the signal to stop the next stage
        const auto& tasks = stages[s + 1]->get_tasks_per_threads();
        for (size_t th = 0; th < tasks.size(); th++)
            for (size_t ta = 0; ta < tasks[th].size(); ta++)
            {
                auto m = dynamic_cast<module::Adaptor_m_to_n*>(&tasks[th][ta]->get_module());
                if (m != nullptr)
                    if (tasks[th][ta]->get_name().find("pull") != std::string::npos) m->cancel_waiting();
            }
    };

    this->thread_pool->run(func_exec, true);

    stages[stages.size() - 1]->exec(stop_conditions[stop_conditions.size() - 1]);

    // stop all the stages before
    for (size_t notify_s = 0; notify_s < stages.size() - 1; notify_s++)
        for (auto& m : stages[notify_s]->get_modules<tools::Interface_waiting>())
            m->cancel_waiting();

    this->thread_pool->wait();
    this->thread_pool->unset_func_exec();
    // ----------------------------------------------------------------------------------------------------------------

    // this is NOT made in the tools::Sequence::exec() to correctly flush the pipeline before restoring buffers
    // initial configuration
    for (auto& stage : this->stages)
        if (stage->is_no_copy_mode())
        {
            stage->reset_no_copy_mode();
            stage->gen_processes(false);
        }

    for (auto& padps : this->adaptors)
    {
        for (auto& adp : padps.first)
            adp->reset();
        for (auto& adp : padps.second)
            adp->reset();
    }
}

void
Pipeline::exec(const std::vector<std::function<bool()>>& stop_conditions)
{
    if (stop_conditions.size() != this->stages.size() && stop_conditions.size() != 1)
    {
        std::stringstream message;
        message << "'stop_conditions.size()' has to be equal to 'stages.size()' or to 1 ('stop_conditions.size()' = "
                << stop_conditions.size() << ", 'stages.size()' = " << stages.size() << ").";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    if (!this->bound_adaptors)
    {
        std::stringstream message;
        message << "'bound_adaptors' has to be true to execute the pipeline.";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    // ----------------------------------------------------------------------------------------------------------------
    auto& stages = this->stages;
    std::vector<const std::function<bool()>*> stop_condition_vec(stages.size() - 1, nullptr);
    if (stop_conditions.size() == stages.size())
        for (size_t s = 0; s < stages.size() - 1; s++)
            stop_condition_vec[s] = &stop_conditions[s];

    std::function<void(const size_t)> func_exec = [&stages, &stop_condition_vec](const size_t tid)
    {
        size_t s = tid;
        if (stop_condition_vec[s])
            stages[s]->exec(*(stop_condition_vec[s]));
        else
            stages[s]->exec();

        // send the signal to stop the next stage
        const auto& tasks = stages[s + 1]->get_tasks_per_threads();
        for (size_t th = 0; th < tasks.size(); th++)
            for (size_t ta = 0; ta < tasks[th].size(); ta++)
            {
                auto m = dynamic_cast<module::Adaptor_m_to_n*>(&tasks[th][ta]->get_module());
                if (m != nullptr)
                    if (tasks[th][ta]->get_name().find("pull") != std::string::npos) m->cancel_waiting();
            }
    };

    this->thread_pool->run(func_exec, true);
    stages[stages.size() - 1]->exec(stop_conditions[stop_conditions.size() - 1]);

    // stop all the stages before
    for (size_t notify_s = 0; notify_s < stages.size() - 1; notify_s++)
        for (auto& m : stages[notify_s]->get_modules<tools::Interface_waiting>())
            m->cancel_waiting();

    this->thread_pool->wait();
    this->thread_pool->unset_func_exec();
    // ----------------------------------------------------------------------------------------------------------------

    // this is NOT made in the tools::Sequence::exec() to correctly flush the pipeline before restoring buffers
    // initial configuration
    for (auto& stage : this->stages)
        if (stage->is_no_copy_mode())
        {
            stage->reset_no_copy_mode();
            stage->gen_processes(false);
        }

    for (auto& padps : this->adaptors)
    {
        for (auto& adp : padps.first)
            adp->reset();
        for (auto& adp : padps.second)
            adp->reset();
    }
}

void
Pipeline::exec(std::function<bool(const std::vector<const int*>&)> stop_condition)
{
    this->exec(std::vector<std::function<bool(const std::vector<const int*>&)>>(1, stop_condition));
}

void
Pipeline::exec(std::function<bool()> stop_condition)
{
    this->exec(std::vector<std::function<bool()>>(1, stop_condition));
}

void
Pipeline::exec()
{
    this->exec([]() { return false; });
}

std::vector<std::vector<module::Module*>>
Pipeline::get_modules_per_threads() const
{
    std::vector<std::vector<module::Module*>> modules_per_threads;
    for (auto& stage : this->stages)
    {
        auto modules_per_threads_add = stage->get_modules_per_threads();
        if (modules_per_threads_add.size() > modules_per_threads.size())
            modules_per_threads.resize(modules_per_threads_add.size());

        for (size_t t = 0; t < modules_per_threads_add.size(); t++)
            modules_per_threads[t].insert(
              modules_per_threads[t].end(), modules_per_threads_add[t].begin(), modules_per_threads_add[t].end());
    }
    return modules_per_threads;
}

std::vector<std::vector<module::Module*>>
Pipeline::get_modules_per_types() const
{
    std::vector<std::vector<module::Module*>> modules_per_types;
    for (auto& stage : this->stages)
    {
        auto modules_per_types_add = stage->get_modules_per_types();
        modules_per_types.insert(modules_per_types.end(), modules_per_types_add.begin(), modules_per_types_add.end());
    }
    return modules_per_types;
}

std::vector<std::vector<module::Module*>>
Pipeline::get_original_modules() const
{
    return this->original_sequence.get_modules_per_types();
}

std::vector<std::vector<runtime::Task*>>
Pipeline::get_tasks_per_threads() const
{
    std::vector<std::vector<runtime::Task*>> tasks_per_threads;
    for (auto& stage : this->stages)
    {
        auto tasks_per_threads_add = stage->get_tasks_per_threads();
        if (tasks_per_threads_add.size() > tasks_per_threads.size())
            tasks_per_threads.resize(tasks_per_threads_add.size());

        for (size_t t = 0; t < tasks_per_threads_add.size(); t++)
            tasks_per_threads[t].insert(
              tasks_per_threads[t].end(), tasks_per_threads_add[t].begin(), tasks_per_threads_add[t].end());
    }
    return tasks_per_threads;
}

std::vector<std::vector<runtime::Task*>>
Pipeline::get_tasks_per_types() const
{
    std::vector<std::vector<runtime::Task*>> tasks_per_types;
    for (auto& stage : this->stages)
    {
        auto tasks_per_types_add = stage->get_tasks_per_types();
        tasks_per_types.insert(tasks_per_types.end(), tasks_per_types_add.begin(), tasks_per_types_add.end());
    }
    return tasks_per_types;
}

void
Pipeline::export_dot(std::ostream& stream) const
{
    std::function<void(tools::Digraph_node<Sub_sequence>*,
                       const size_t,
                       const std::string&,
                       std::ostream&,
                       std::vector<tools::Digraph_node<Sub_sequence>*>&)>
      export_dot_subsequences_recursive = [&export_dot_subsequences_recursive,
                                           this](tools::Digraph_node<Sub_sequence>* cur_node,
                                                 const size_t sta,
                                                 const std::string& tab,
                                                 std::ostream& stream,
                                                 std::vector<tools::Digraph_node<Sub_sequence>*>& already_parsed_nodes)
    {
        if (cur_node != nullptr &&
            std::find(already_parsed_nodes.begin(), already_parsed_nodes.end(), cur_node) == already_parsed_nodes.end())
        {
            already_parsed_nodes.push_back(cur_node);
            this->stages[sta]->export_dot_subsequence(cur_node->get_c()->tasks,
                                                      cur_node->get_c()->tasks_id,
                                                      cur_node->get_c()->type,
                                                      "Sub-sequence " + std::to_string(cur_node->get_c()->id) +
                                                        " (depth = " + std::to_string(cur_node->get_depth()) + ")",
                                                      tab,
                                                      stream);

            for (auto c : cur_node->get_children())
                export_dot_subsequences_recursive(c, sta, tab, stream, already_parsed_nodes);
        }
    };

    std::function<void(tools::Digraph_node<Sub_sequence>*,
                       const size_t,
                       const std::string&,
                       std::ostream&,
                       std::vector<tools::Digraph_node<Sub_sequence>*>&)>
      export_dot_connections_recursive =
        [&export_dot_connections_recursive, this](tools::Digraph_node<Sub_sequence>* cur_node,
                                                  const size_t sta,
                                                  const std::string& tab,
                                                  std::ostream& stream,
                                                  std::vector<tools::Digraph_node<Sub_sequence>*>& already_parsed_nodes)
    {
        if (cur_node != nullptr &&
            std::find(already_parsed_nodes.begin(), already_parsed_nodes.end(), cur_node) == already_parsed_nodes.end())
        {
            already_parsed_nodes.push_back(cur_node);
            this->stages[sta]->export_dot_connections(cur_node->get_c()->tasks, tab, stream);

            for (auto c : cur_node->get_children())
                export_dot_connections_recursive(c, sta, tab, stream, already_parsed_nodes);
        }
    };

    std::string tab = "\t";
    stream << "digraph Pipeline {" << std::endl;
    stream << tab << "compound=true;" << std::endl;

    for (size_t sta = 0; sta < this->stages.size(); sta++)
    {
        std::vector<tools::Digraph_node<Sub_sequence>*> already_parsed_nodes;
        const auto n_threads = this->stages[sta]->get_n_threads();
        stream << tab << "subgraph \"cluster_Stage " << sta << "\" {" << std::endl;
        stream << tab << tab << "node [style=filled];" << std::endl;
        export_dot_subsequences_recursive(this->stages[sta]->sequences[0], sta, tab, stream, already_parsed_nodes);
        stream << tab << tab << "label=\"Pipeline stage " << sta << " (" << n_threads << " thread(s))\";" << std::endl;
        std::string color = "blue";
        stream << tab << tab << "color=" << color << ";" << std::endl;
        stream << tab << "}" << std::endl;
    }

    for (size_t sta = 0; sta < this->stages.size(); sta++)
    {
        std::vector<tools::Digraph_node<Sub_sequence>*> already_parsed_nodes;
        export_dot_connections_recursive(this->stages[sta]->sequences[0], sta, tab, stream, already_parsed_nodes);
        if (this->bound_adaptors)
        {
            if (sta > 0)
            {
                auto tsk1 = this->stages[sta - 1]->get_lasts_tasks()[0].back();
                auto tsk2 = this->stages[sta + 0]->get_firsts_tasks()[0][0];

                auto sck1 = tsk1->sockets[0];
                auto sck2 = tsk2->sockets[0];

                stream << tab << "\"" << +sck1.get() << "\" -> \"" << +sck2.get() << "\" [ltail=\"cluster_"
                       << +&tsk1->get_module() << "_" << +tsk1 << "\" lhead=\"cluster_" << +&tsk2->get_module() << "_"
                       << +tsk2 << "\" color=\"green\" style=\"dashed\"];" << std::endl;
            }
        }
    }

    stream << "}" << std::endl;
}

bool
Pipeline::is_bound_adaptors() const
{
    return this->bound_adaptors;
}

void
Pipeline::set_auto_stop(const bool auto_stop)
{
    this->auto_stop = auto_stop;
    for (auto stage : this->stages)
        stage->set_auto_stop(auto_stop);
}

bool
Pipeline::is_auto_stop() const
{
    return this->auto_stop;
}

size_t
Pipeline::get_n_frames() const
{
    const auto n_frames = this->stages[0]->get_n_frames();

    for (auto& sta : this->stages)
        if (sta->get_n_frames() != n_frames)
        {
            std::stringstream message;
            message << "All the stages/sequences do not have the same 'n_frames' value ('sta->get_n_frames()' = "
                    << sta->get_n_frames() << ", 'n_frames' = " << n_frames << ").";
            throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
        }

    return n_frames;
}

void
Pipeline::set_n_frames(const size_t n_frames)
{
    const auto save_bound_adaptors = this->is_bound_adaptors();
    if (!save_bound_adaptors) this->bind_adaptors();
    this->_unbind_adaptors(false);

    // set the new "n_frames" val in the sequences
    std::vector<std::vector<std::pair<runtime::Socket*, runtime::Socket*>>> unbind_sockets(this->stages.size());
    std::vector<std::vector<std::pair<runtime::Task*, runtime::Socket*>>> unbind_tasks(this->stages.size());
    std::vector<bool> skip(this->stages.size());
    for (size_t s = 0; s < this->stages.size(); s++)
        skip[s] = this->stages[s]->get_n_frames() == n_frames;
    for (size_t s = 0; s < this->stages.size(); s++)
        if (!skip[s]) this->stages[s]->_set_n_frames_unbind(unbind_sockets[s], unbind_tasks[s]);
    for (size_t s = 0; s < this->stages.size(); s++)
        if (!skip[s]) this->stages[s]->_set_n_frames(n_frames);
    for (size_t s = 0; s < this->stages.size(); s++)
        if (!skip[s]) this->stages[s]->_set_n_frames_rebind(unbind_sockets[s], unbind_tasks[s]);

    // set the new "n_frames" val in the adaptors
    for (auto& adps : this->adaptors)
    {
        for (auto& adp : adps.first)
            adp->set_n_frames(n_frames);
        for (auto& adp : adps.second)
            adp->set_n_frames(n_frames);
    }

    // bind orphans to complete the unbind of the adaptors
    for (auto& bind : this->sck_orphan_binds)
    {
        auto sck_out = std::get<0>(bind.first);
        auto priority = std::get<4>(bind.first);
        auto sck_in = std::get<0>(bind.second);
        if (sck_in != nullptr)
            sck_in->_bind(*sck_out, priority);
        else
        {
            auto tsk_in = std::get<4>(bind.second);
            assert(tsk_in != nullptr);
            tsk_in->_bind(*sck_out, priority);
        }
    }

    if (save_bound_adaptors) this->bind_adaptors();
}
