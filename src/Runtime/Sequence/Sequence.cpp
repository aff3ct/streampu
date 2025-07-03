#include <algorithm>
#include <cstring>
#include <exception>
#include <fstream>
#include <numeric>
#include <set>
#include <sstream>
#include <thread>
#include <utility>

#include "Module/Module.hpp"
#include "Module/Stateful/Adaptor/Adaptor_m_to_n.hpp"
#include "Module/Stateful/Probe/Probe.hpp"
#include "Module/Stateful/Switcher/Switcher.hpp"
#include "Runtime/Sequence/Sequence.hpp"
#include "Runtime/Socket/Socket.hpp"
#include "Runtime/Task/Task.hpp"
#include "Tools/Display/rang_format/rang_format.h"
#include "Tools/Exception/exception.hpp"
#include "Tools/Signal_handler/Signal_handler.hpp"
#include "Tools/Thread/Thread_pinning/Thread_pinning.hpp"
#include "Tools/Thread/Thread_pinning/Thread_pinning_utils.hpp"
#include "Tools/Thread/Thread_pool/Standard/Thread_pool_standard.hpp"

using namespace spu;
using namespace spu::runtime;

Sequence::Sequence(const std::vector<const runtime::Task*>& firsts,
                   const std::vector<const runtime::Task*>& lasts,
                   const std::vector<const runtime::Task*>& exclusions,
                   const size_t n_threads,
                   const bool thread_pinning,
                   const std::vector<size_t>& puids,
                   const bool memory_allocation)
  : n_threads(n_threads)
  , sequences(n_threads, nullptr)
  , modules(n_threads)
  , all_modules(n_threads)
  , mtx_exception(new std::mutex())
  , force_exit_loop(new std::atomic<bool>(false))
  , tasks_inplace(false)
  , thread_pinning(thread_pinning)
  , puids(puids)
  , no_copy_mode(true)
  , saved_exclusions(exclusions)
  , switchers_reset(n_threads)
  , auto_stop(true)
  , is_part_of_pipeline(false)
  , next_round_is_over(n_threads, false)
  , cur_task_id(n_threads, 0)
  , cur_ss(n_threads, nullptr)
  , memory_allocation(memory_allocation)
{
#ifndef SPU_HWLOC
    if (thread_pinning)
        std::clog << rang::tag::warning
                  << "StreamPU has not been linked with the 'hwloc' library, the 'thread_pinning' "
                     "option of the 'runtime::Sequence' will have no effect."
                  << std::endl;
#endif

    if (thread_pinning && puids.size() < n_threads)
    {
        std::stringstream message;
        message << "'puids.size()' has to be greater or equal to 'n_threads' ('puids.size()' = " << puids.size()
                << " , 'n_threads' = " << n_threads << ").";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    this->init<runtime::Sub_sequence_const, const runtime::Task>(firsts, lasts, exclusions);
}

Sequence::Sequence(const std::vector<const runtime::Task*>& firsts,
                   const std::vector<const runtime::Task*>& lasts,
                   const size_t n_threads,
                   const bool thread_pinning,
                   const std::vector<size_t>& puids,
                   const bool memory_allocation)
  : Sequence(firsts, lasts, {}, n_threads, thread_pinning, puids, memory_allocation)
{
}

Sequence::Sequence(const std::vector<const runtime::Task*>& firsts,
                   const size_t n_threads,
                   const bool thread_pinning,
                   const std::vector<size_t>& puids,
                   const bool memory_allocation)
  : Sequence(firsts, {}, {}, n_threads, thread_pinning, puids, memory_allocation)
{
}

Sequence::Sequence(const runtime::Task& first,
                   const runtime::Task& last,
                   const size_t n_threads,
                   const bool thread_pinning,
                   const std::vector<size_t>& puids,
                   const bool memory_allocation)
  : Sequence({ &first }, { &last }, n_threads, thread_pinning, puids, memory_allocation)
{
}

Sequence::Sequence(const runtime::Task& first,
                   const size_t n_threads,
                   const bool thread_pinning,
                   const std::vector<size_t>& puids,
                   const bool memory_allocation)
  : Sequence({ &first }, n_threads, thread_pinning, puids, memory_allocation)
{
}

std::vector<const runtime::Task*>
exclusions_convert_to_const(const std::vector<runtime::Task*>& exclusions)
{
    std::vector<const runtime::Task*> exclusions_const;
    for (auto exception : exclusions)
        exclusions_const.push_back(exception);
    return exclusions_const;
}

Sequence::Sequence(const std::vector<runtime::Task*>& firsts,
                   const std::vector<runtime::Task*>& lasts,
                   const std::vector<runtime::Task*>& exclusions,
                   const size_t n_threads,
                   const bool thread_pinning,
                   const std::vector<size_t>& puids,
                   const bool tasks_inplace,
                   const bool memory_allocation)
  : n_threads(n_threads)
  , sequences(n_threads, nullptr)
  , modules(tasks_inplace ? n_threads - 1 : n_threads)
  , all_modules(n_threads)
  , mtx_exception(new std::mutex())
  , force_exit_loop(new std::atomic<bool>(false))
  , tasks_inplace(tasks_inplace)
  , thread_pinning(thread_pinning)
  , puids(puids)
  , no_copy_mode(true)
  , saved_exclusions(exclusions_convert_to_const(exclusions))
  , switchers_reset(n_threads)
  , auto_stop(true)
  , is_part_of_pipeline(false)
  , next_round_is_over(n_threads, false)
  , cur_task_id(n_threads, 0)
  , cur_ss(n_threads, nullptr)
  , memory_allocation(memory_allocation)
{
    if (thread_pinning && puids.size() < n_threads)
    {
        std::stringstream message;
        message << "'puids.size()' has greater or equal to 'n_threads' ('puids.size()' = " << puids.size()
                << " , 'n_threads' = " << n_threads << ").";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    if (tasks_inplace)
        this->init<runtime::Sub_sequence, runtime::Task>(firsts, lasts, exclusions);
    else
    {
        std::vector<const runtime::Task*> firsts_bis;
        for (auto first : firsts)
            firsts_bis.push_back(first);
        std::vector<const runtime::Task*> lasts_bis;
        for (auto last : lasts)
            lasts_bis.push_back(last);
        std::vector<const runtime::Task*> exclusions_bis;
        for (auto exception : exclusions)
            exclusions_bis.push_back(exception);
        this->init<runtime::Sub_sequence_const, const runtime::Task>(firsts_bis, lasts_bis, exclusions_bis);
    }
}

Sequence::Sequence(const std::vector<runtime::Task*>& firsts,
                   const std::vector<runtime::Task*>& lasts,
                   const size_t n_threads,
                   const bool thread_pinning,
                   const std::vector<size_t>& puids,
                   const bool tasks_inplace,
                   const bool memory_allocation)
  : Sequence(firsts, lasts, {}, n_threads, thread_pinning, puids, tasks_inplace, memory_allocation)
{
}

Sequence::Sequence(const std::vector<runtime::Task*>& firsts,
                   const size_t n_threads,
                   const bool thread_pinning,
                   const std::vector<size_t>& puids,
                   const bool tasks_inplace,
                   const bool memory_allocation)
  : Sequence(firsts, {}, {}, n_threads, thread_pinning, puids, tasks_inplace, memory_allocation)
{
}

Sequence::Sequence(runtime::Task& first,
                   runtime::Task& last,
                   const size_t n_threads,
                   const bool thread_pinning,
                   const std::vector<size_t>& puids,
                   const bool tasks_inplace,
                   const bool memory_allocation)
  : Sequence({ &first }, { &last }, n_threads, thread_pinning, puids, tasks_inplace, memory_allocation)
{
}

Sequence::Sequence(runtime::Task& first,
                   const size_t n_threads,
                   const bool thread_pinning,
                   const std::vector<size_t>& puids,
                   const bool tasks_inplace,
                   const bool memory_allocation)
  : Sequence({ &first }, n_threads, thread_pinning, puids, tasks_inplace, memory_allocation)
{
}
//=======================================New pinning version constructors===============================================
Sequence::Sequence(const std::vector<const runtime::Task*>& firsts,
                   const std::vector<const runtime::Task*>& lasts,
                   const std::vector<const runtime::Task*>& exclusions,
                   const size_t n_threads,
                   const bool thread_pinning,
                   const std::string& sequence_pinning_policy,
                   const bool memory_allocation)
  : n_threads(n_threads)
  , sequences(n_threads, nullptr)
  , modules(n_threads)
  , all_modules(n_threads)
  , mtx_exception(new std::mutex())
  , force_exit_loop(new std::atomic<bool>(false))
  , tasks_inplace(false)
  , thread_pinning(thread_pinning)
  , puids({})
  , no_copy_mode(true)
  , saved_exclusions(exclusions)
  , switchers_reset(n_threads)
  , auto_stop(true)
  , is_part_of_pipeline(false)
  , next_round_is_over(n_threads, false)
  , cur_task_id(n_threads, 0)
  , cur_ss(n_threads, nullptr)
  , memory_allocation(memory_allocation)
{
#ifndef SPU_HWLOC
    if (thread_pinning)
        std::clog << rang::tag::warning
                  << "StreamPU has not been linked with the 'hwloc' library, the 'thread_pinning' "
                     "option of the 'runtime::Sequence' will have no effect."
                  << std::endl;
#endif

    if (thread_pinning && !sequence_pinning_policy.empty())
    {
        pin_objects_per_thread = tools::Thread_pinning_utils::stage_parser_unpacker(sequence_pinning_policy, n_threads);
    }
    else if (thread_pinning && sequence_pinning_policy.empty())
    {
        std::stringstream message;
        message << "Pinning is activated but there is no specified policy." << std::endl;
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    this->init<runtime::Sub_sequence_const, const runtime::Task>(firsts, lasts, exclusions);
}

Sequence::Sequence(const std::vector<const runtime::Task*>& firsts,
                   const std::vector<const runtime::Task*>& lasts,
                   const size_t n_threads,
                   const bool thread_pinning,
                   const std::string& sequence_pinning_policy,
                   const bool memory_allocation)
  : Sequence(firsts, lasts, {}, n_threads, thread_pinning, sequence_pinning_policy, memory_allocation)
{
}

Sequence::Sequence(const std::vector<const runtime::Task*>& firsts,
                   const size_t n_threads,
                   const bool thread_pinning,
                   const std::string& sequence_pinning_policy,
                   const bool memory_allocation)
  : Sequence(firsts, {}, {}, n_threads, thread_pinning, sequence_pinning_policy, memory_allocation)
{
}

Sequence::Sequence(const runtime::Task& first,
                   const runtime::Task& last,
                   const size_t n_threads,
                   const bool thread_pinning,
                   const std::string& sequence_pinning_policy,
                   const bool memory_allocation)
  : Sequence({ &first }, { &last }, n_threads, thread_pinning, sequence_pinning_policy, memory_allocation)
{
}

Sequence::Sequence(const runtime::Task& first,
                   const size_t n_threads,
                   const bool thread_pinning,
                   const std::string& sequence_pinning_policy,
                   const bool memory_allocation)
  : Sequence({ &first }, n_threads, thread_pinning, sequence_pinning_policy, memory_allocation)
{
}

Sequence::Sequence(const std::vector<runtime::Task*>& firsts,
                   const std::vector<runtime::Task*>& lasts,
                   const std::vector<runtime::Task*>& exclusions,
                   const size_t n_threads,
                   const bool thread_pinning,
                   const std::string& sequence_pinning_policy,
                   const bool tasks_inplace,
                   const bool memory_allocation)
  : n_threads(n_threads)
  , sequences(n_threads, nullptr)
  , modules(tasks_inplace ? n_threads - 1 : n_threads)
  , all_modules(n_threads)
  , mtx_exception(new std::mutex())
  , force_exit_loop(new std::atomic<bool>(false))
  , tasks_inplace(tasks_inplace)
  , thread_pinning(thread_pinning)
  , puids({})
  , no_copy_mode(true)
  , saved_exclusions(exclusions_convert_to_const(exclusions))
  , switchers_reset(n_threads)
  , auto_stop(true)
  , is_part_of_pipeline(false)
  , next_round_is_over(n_threads, false)
  , cur_task_id(n_threads, 0)
  , cur_ss(n_threads, nullptr)
  , memory_allocation(memory_allocation)
{
    if (thread_pinning && !sequence_pinning_policy.empty())
    {
        pin_objects_per_thread = tools::Thread_pinning_utils::stage_parser_unpacker(sequence_pinning_policy, n_threads);
    }
    else if (thread_pinning && sequence_pinning_policy.empty())
    {
        std::stringstream message;
        message << "Pinning is activated but there is no specified policy." << std::endl;
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    if (tasks_inplace)
        this->init<runtime::Sub_sequence, runtime::Task>(firsts, lasts, exclusions);
    else
    {
        std::vector<const runtime::Task*> firsts_bis;
        for (auto first : firsts)
            firsts_bis.push_back(first);
        std::vector<const runtime::Task*> lasts_bis;

        for (auto last : lasts)
            lasts_bis.push_back(last);
        std::vector<const runtime::Task*> exclusions_bis;

        for (auto exception : exclusions)
            exclusions_bis.push_back(exception);
        this->init<runtime::Sub_sequence_const, const runtime::Task>(firsts_bis, lasts_bis, exclusions_bis);
    }
}

Sequence::Sequence(const std::vector<runtime::Task*>& firsts,
                   const std::vector<runtime::Task*>& lasts,
                   const size_t n_threads,
                   const bool thread_pinning,
                   const std::string& sequence_pinning_policy,
                   const bool tasks_inplace,
                   const bool memory_allocation)
  : Sequence(firsts,
             lasts,
             {},
             n_threads,
             thread_pinning,
             sequence_pinning_policy,
             tasks_inplace,
             memory_allocation)
{
}

Sequence::Sequence(const std::vector<runtime::Task*>& firsts,
                   const size_t n_threads,
                   const bool thread_pinning,
                   const std::string& sequence_pinning_policy,
                   const bool tasks_inplace,
                   const bool memory_allocation)
  : Sequence(firsts,
             {},
             {},
             n_threads,
             thread_pinning,
             sequence_pinning_policy,
             tasks_inplace,
             memory_allocation)
{
}

Sequence::Sequence(runtime::Task& first,
                   runtime::Task& last,
                   const size_t n_threads,
                   const bool thread_pinning,
                   const std::string& sequence_pinning_policy,
                   const bool tasks_inplace,
                   const bool memory_allocation)
  : Sequence({ &first },
             { &last },
             n_threads,
             thread_pinning,
             sequence_pinning_policy,
             tasks_inplace,
             memory_allocation)
{
}

Sequence::Sequence(runtime::Task& first,
                   const size_t n_threads,
                   const bool thread_pinning,
                   const std::string& sequence_pinning_policy,
                   const bool tasks_inplace,
                   const bool memory_allocation)
  : Sequence({ &first }, n_threads, thread_pinning, sequence_pinning_policy, tasks_inplace, memory_allocation)
{
}

// ====================================================================================================================

Sequence::~Sequence()
{
    std::vector<tools::Digraph_node<Sub_sequence>*> already_deleted_nodes;
    for (auto s : this->sequences)
        this->delete_tree(s, already_deleted_nodes);
}

template<class SS, class TA>
void
Sequence::init(const std::vector<TA*>& firsts, const std::vector<TA*>& lasts, const std::vector<TA*>& exclusions)
{
    if (this->is_thread_pinning())
    {
        if (!this->puids.empty())
            tools::Thread_pinning::pin(this->puids[0]);
        else
            tools::Thread_pinning::pin(this->pin_objects_per_thread[0]);
    }

    if (firsts.size() == 0)
    {
        std::stringstream message;
        message << "'firsts.size()' has to be strictly greater than 0.";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    if (this->n_threads == 0)
    {
        std::stringstream message;
        message << "'n_threads' has to be strictly greater than 0.";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    for (auto exclusion : exclusions)
    {
        if (std::find(firsts.begin(), firsts.end(), exclusion) != firsts.end())
        {
            std::stringstream message;
            message << "'exclusion' can't be contained in the 'firsts' vector ("
                    << "'exclusion'"
                    << " = " << +exclusion << ", "
                    << "'exclusion->get_name()'"
                    << " = " << exclusion->get_name() << ").";
            throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
        }

        if (std::find(lasts.begin(), lasts.end(), exclusion) != lasts.end())
        {
            std::stringstream message;
            message << "'exclusion' can't be contained in the 'lasts' vector ("
                    << "'exclusion'"
                    << " = " << +exclusion << ", "
                    << "'exclusion->get_name()'"
                    << " = " << exclusion->get_name() << ").";
            throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
        }
    }
    for (auto t : lasts)
    {
        if (dynamic_cast<const module::Switcher*>(&t->get_module()) && t->get_name() == "commute")
        {
            std::stringstream message;
            message << "A sequence cannot end with a 'commute' task.";
            throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
        }
    }

    auto root = new tools::Digraph_node<SS>({}, {}, nullptr, 0);
    root->set_contents(nullptr);
    size_t ssid = 0, taid = 0;
    std::vector<TA*> switchers;
    std::vector<std::pair<TA*, tools::Digraph_node<SS>*>> selectors;
    std::vector<TA*> real_lasts;

    this->lasts_tasks_id.clear();
    this->firsts_tasks_id.clear();
    auto last_subseq = root;
    std::map<TA*, unsigned> in_sockets_feed;
    for (auto first : firsts)
    {
        std::map<TA*, std::pair<tools::Digraph_node<SS>*, size_t>> task_subseq;
        auto contents = last_subseq->get_contents();
        this->firsts_tasks_id.push_back(contents ? contents->tasks_id[contents->tasks_id.size() - 1] : 0);
        last_subseq = this->init_recursive<SS, TA>(last_subseq,
                                                   ssid,
                                                   taid,
                                                   selectors,
                                                   switchers,
                                                   *first,
                                                   *first,
                                                   lasts,
                                                   exclusions,
                                                   this->lasts_tasks_id,
                                                   real_lasts,
                                                   in_sockets_feed,
                                                   task_subseq);
    }

    std::stringstream real_lasts_ss;
    for (size_t rl = 0; rl < real_lasts.size(); rl++)
        real_lasts_ss << "'real_lasts"
                      << "[" << rl << "]'"
                      << " = " << +real_lasts[rl] << ", "
                      << "'real_lasts"
                      << "[" << rl << "]->get_name()'"
                      << " = " << real_lasts[rl]->get_name() << ((rl < real_lasts.size() - 1) ? ", " : "");

    for (auto last : lasts)
    {
        if (std::find(real_lasts.begin(), real_lasts.end(), last) == real_lasts.end())
        {
            std::stringstream message;
            message << "'last' is not contained in the 'real_lasts[" << real_lasts.size() << "]' vector ("
                    << "'last'"
                    << " = " << +last << ", "
                    << "'last->get_name()'"
                    << " = " << last->get_name() << ", " << real_lasts_ss.str() << ").";
            throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
        }
    }

    this->n_tasks = taid;
    // this->check_ctrl_flow(root); // /!\ this check has been commented because it is known to do not work in the
    //     general case
    this->_init<SS>(root);
    this->update_firsts_and_lasts_tasks();
    this->gen_processes();
    this->donners = get_modules<tools::Interface_is_done>(true);

    for (size_t tid = 0; tid < this->n_threads; tid++)
        for (auto& mdl : this->all_modules[tid])
            if (auto swi = dynamic_cast<module::Switcher*>(mdl))
                this->switchers_reset[tid].push_back(dynamic_cast<tools::Interface_reset*>(swi));

    for (size_t tid = 0; tid < this->sequences.size(); tid++)
    {
        this->cur_ss[tid] = this->sequences[tid];
    }

    // Allocating Memory if requested
    if (this->memory_allocation)
    {
        this->allocation_function.allocate_sequence_memory(this);
    }

    this->thread_pool.reset(new tools::Thread_pool_standard(this->n_threads - 1));
    this->thread_pool->init(); // threads are spawned here
}

Sequence*
Sequence::clone() const
{
    auto c = new Sequence(*this);

    c->tasks_inplace = false;
    c->modules.resize(c->get_n_threads());

    std::vector<const runtime::Task*> firsts_tasks;
    for (auto ta : this->get_firsts_tasks()[0])
        firsts_tasks.push_back(ta);

    std::vector<const runtime::Task*> lasts_tasks;
    for (auto ta : this->get_lasts_tasks()[0])
        lasts_tasks.push_back(ta);

    c->init<runtime::Sub_sequence_const, const runtime::Task>(firsts_tasks, lasts_tasks, this->saved_exclusions);
    c->mtx_exception.reset(new std::mutex());
    c->force_exit_loop.reset(new std::atomic<bool>(false));
    return c;
}

void
Sequence::set_thread_pinning(const bool thread_pinning, const std::vector<size_t>& puids)
{
    if (thread_pinning && puids.size() < n_threads)
    {
        std::stringstream message;
        message << "'puids.size()' has greater or equal to 'n_threads' ('puids.size()' = " << puids.size()
                << " , 'n_threads' = " << n_threads << ").";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    this->thread_pinning = thread_pinning;
    this->puids = puids;
    this->pin_objects_per_thread = {};
}

void
Sequence::set_thread_pinning(const bool thread_pinning, const std::string& sequence_pinning_policy)
{
    this->thread_pinning = thread_pinning;
    this->puids = {};
    this->pin_objects_per_thread =
      tools::Thread_pinning_utils::stage_parser_unpacker(sequence_pinning_policy, n_threads);
}

bool
Sequence::is_thread_pinning()
{
    return this->thread_pinning;
}

std::vector<std::vector<module::Module*>>
Sequence::get_modules_per_threads() const
{
    std::vector<std::vector<module::Module*>> modules_per_threads(this->all_modules.size());
    size_t tid = 0;
    for (auto& e : this->all_modules)
    {
        for (auto& ee : e)
            modules_per_threads[tid].push_back(ee);
        tid++;
    }
    return modules_per_threads;
}

std::vector<std::vector<module::Module*>>
Sequence::get_modules_per_types() const
{
    std::vector<std::vector<module::Module*>> modules_per_types(this->all_modules[0].size());
    for (auto& e : this->all_modules)
    {
        size_t mid = 0;
        for (auto& ee : e)
            modules_per_types[mid++].push_back(ee);
    }
    return modules_per_types;
}

std::vector<std::vector<runtime::Task*>>
Sequence::get_tasks_per_threads() const
{
    std::vector<std::vector<runtime::Task*>> tasks_per_threads(this->n_threads);

    std::function<void(
      tools::Digraph_node<Sub_sequence>*, const size_t, std::vector<tools::Digraph_node<Sub_sequence>*>&)>
      get_tasks_recursive = [&](tools::Digraph_node<Sub_sequence>* cur_ss,
                                const size_t tid,
                                std::vector<tools::Digraph_node<Sub_sequence>*>& already_parsed_nodes)
    {
        if (std::find(already_parsed_nodes.begin(), already_parsed_nodes.end(), cur_ss) == already_parsed_nodes.end())
        {
            already_parsed_nodes.push_back(cur_ss);
            tasks_per_threads[tid].insert(
              tasks_per_threads[tid].end(), cur_ss->get_c()->tasks.begin(), cur_ss->get_c()->tasks.end());

            for (auto c : cur_ss->get_children())
                get_tasks_recursive(c, tid, already_parsed_nodes);
        }
    };

    for (size_t tid = 0; tid < this->n_threads; tid++)
    {
        std::vector<tools::Digraph_node<Sub_sequence>*> already_parsed_nodes;
        get_tasks_recursive(this->sequences[tid], tid, already_parsed_nodes);
    }

    return tasks_per_threads;
}

std::vector<std::vector<runtime::Task*>>
Sequence::get_tasks_per_types() const
{
    std::vector<std::vector<runtime::Task*>> tasks_per_types(this->n_tasks);

    std::function<void(tools::Digraph_node<Sub_sequence>*, size_t&, std::vector<tools::Digraph_node<Sub_sequence>*>&)>
      get_tasks_recursive = [&](tools::Digraph_node<Sub_sequence>* cur_ss,
                                size_t& mid,
                                std::vector<tools::Digraph_node<Sub_sequence>*>& already_parsed_nodes)
    {
        if (std::find(already_parsed_nodes.begin(), already_parsed_nodes.end(), cur_ss) == already_parsed_nodes.end())
        {
            already_parsed_nodes.push_back(cur_ss);
            for (auto& t : cur_ss->get_c()->tasks)
                tasks_per_types[mid++].push_back(t);

            for (auto c : cur_ss->get_children())
                get_tasks_recursive(c, mid, already_parsed_nodes);
        }
    };

    for (size_t tid = 0; tid < this->n_threads; tid++)
    {
        size_t mid = 0;
        std::vector<tools::Digraph_node<Sub_sequence>*> already_parsed_nodes;
        get_tasks_recursive(this->sequences[tid], mid, already_parsed_nodes);
    }

    return tasks_per_types;
}

bool
Sequence::is_done() const
{
    for (auto donner : this->donners)
        if (donner->is_done()) return true;
    return false;
}

void
Sequence::allocate_outbuffers()
{
    if (!this->memory_allocation)
    {
        this->allocation_function.allocate_sequence_memory(this);
        this->memory_allocation = true;
    }
    else
    {
        std::stringstream message;
        message << "The memory is already allocated for this sequence";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }
}

void
Sequence::deallocate_outbuffers()
{
    if (this->memory_allocation)
    {
        this->allocation_function.deallocate_sequence_memory(this);
        this->memory_allocation = false;
    }
    else
    {
        std::stringstream message;
        message << "Sequence memory is not allocated";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }
}

void
Sequence::_exec(const size_t tid,
                std::function<bool(const std::vector<const int*>&)>& stop_condition,
                tools::Digraph_node<Sub_sequence>* sequence)
{
    tools::Signal_handler::reset_sigint();

    if (this->is_thread_pinning())
    {
        if (!puids.empty())
            tools::Thread_pinning::pin(this->puids[tid]);
        else
            tools::Thread_pinning::pin(this->pin_objects_per_thread[tid]);
    }

    std::function<void(tools::Digraph_node<Sub_sequence>*, std::vector<const int*>&)> exec_sequence =
      [&exec_sequence](tools::Digraph_node<Sub_sequence>* cur_ss, std::vector<const int*>& statuses)
    {
        auto type = cur_ss->get_c()->type;
        auto& tasks_id = cur_ss->get_c()->tasks_id;
        auto& processes = cur_ss->get_c()->processes;

        if (type == subseq_t::COMMUTE)
        {
            statuses[tasks_id[0]] = processes[0]();
            const int path = statuses[tasks_id[0]][0];
            if (cur_ss->get_children().size() > (size_t)path) exec_sequence(cur_ss->get_children()[path], statuses);
        }
        else
        {
            for (size_t p = 0; p < processes.size(); p++)
                statuses[tasks_id[p]] = processes[p]();
            for (auto c : cur_ss->get_children())
                exec_sequence(c, statuses);
        }
    };

    std::vector<const int*> statuses(this->n_tasks, nullptr);
    try
    {
        do
        {
            // force switchers reset to reinitialize the path to the last input socket
            for (size_t s = 0; s < this->switchers_reset[tid].size(); s++)
                this->switchers_reset[tid][s]->reset();

            std::fill(statuses.begin(), statuses.end(), nullptr);
            try
            {
                exec_sequence(sequence, statuses);
            }
            catch (tools::processing_aborted const&)
            {
                // do nothing, this is normal
            }
        } while (!*force_exit_loop && !stop_condition(statuses) && !tools::Signal_handler::is_sigint());
    }
    catch (tools::waiting_canceled const&)
    {
        // do nothing, this is normal
    }
    catch (std::exception const& e)
    {
        *force_exit_loop = true;

        this->mtx_exception->lock();

        auto save = tools::exception::no_stacktrace;
        tools::exception::no_stacktrace = true;
        std::string msg = e.what(); // get only the function signature
        tools::exception::no_stacktrace = save;

        if (std::find(this->prev_exception_messages.begin(), this->prev_exception_messages.end(), msg) ==
            this->prev_exception_messages.end())
        {
            this->prev_exception_messages.push_back(msg);                 // save only the function signature
            this->prev_exception_messages_to_display.push_back(e.what()); // with stacktrace if debug mode
        }
        else if (std::strlen(e.what()) > this->prev_exception_messages_to_display.back().size())
            this->prev_exception_messages_to_display[prev_exception_messages_to_display.size() - 1] = e.what();

        this->mtx_exception->unlock();
    }

    if (this->is_thread_pinning()) tools::Thread_pinning::unpin();
}

void
Sequence::_exec_without_statuses(const size_t tid,
                                 std::function<bool()>& stop_condition,
                                 tools::Digraph_node<Sub_sequence>* sequence)
{
    tools::Signal_handler::reset_sigint();

    if (this->is_thread_pinning())
    {
        if (!puids.empty())
            tools::Thread_pinning::pin(this->puids[tid]);
        else
            tools::Thread_pinning::pin(this->pin_objects_per_thread[tid]);
    }

    std::function<void(tools::Digraph_node<Sub_sequence>*)> exec_sequence =
      [&exec_sequence](tools::Digraph_node<Sub_sequence>* cur_ss)
    {
        auto type = cur_ss->get_c()->type;
        auto& processes = cur_ss->get_c()->processes;

        if (type == subseq_t::COMMUTE)
        {
            const int path = processes[0]()[0];
            if (cur_ss->get_children().size() > (size_t)path) exec_sequence(cur_ss->get_children()[path]);
        }
        else
        {
            for (auto& process : processes)
                process();
            for (auto c : cur_ss->get_children())
                exec_sequence(c);
        }
    };

    try
    {
        do
        {
            // force switchers reset to reinitialize the path to the last input socket
            for (size_t s = 0; s < this->switchers_reset[tid].size(); s++)
                this->switchers_reset[tid][s]->reset();

            try
            {
                exec_sequence(sequence);
            }
            catch (tools::processing_aborted const&)
            {
                // do nothing, this is normal
            }
        } while (!*force_exit_loop && !stop_condition() && !tools::Signal_handler::is_sigint());
    }
    catch (tools::waiting_canceled const&)
    {
        // do nothing, this is normal
    }
    catch (std::exception const& e)
    {
        *force_exit_loop = true;

        this->mtx_exception->lock();

        auto save = tools::exception::no_stacktrace;
        tools::exception::no_stacktrace = true;
        std::string msg = e.what(); // get only the function signature
        tools::exception::no_stacktrace = save;

        if (std::find(this->prev_exception_messages.begin(), this->prev_exception_messages.end(), msg) ==
            this->prev_exception_messages.end())
        {
            this->prev_exception_messages.push_back(msg);                 // save only the function signature
            this->prev_exception_messages_to_display.push_back(e.what()); // with stacktrace if debug mode
        }
        else if (std::strlen(e.what()) > this->prev_exception_messages_to_display.back().size())
            this->prev_exception_messages_to_display[prev_exception_messages_to_display.size() - 1] = e.what();

        this->mtx_exception->unlock();
    }

    if (this->is_thread_pinning()) tools::Thread_pinning::unpin();
}

void
Sequence::exec(std::function<bool(const std::vector<const int*>&)> stop_condition)
{
    if (this->is_no_copy_mode()) this->gen_processes(true);

    std::function<bool(const std::vector<const int*>&)> real_stop_condition;
    if (this->auto_stop)
        real_stop_condition = [this, stop_condition](const std::vector<const int*>& statuses)
        {
            bool res = stop_condition(statuses);
            return res || this->is_done();
        };
    else
        real_stop_condition = stop_condition;

    std::function<void(const size_t)> func_exec = [this, &real_stop_condition](const size_t tid)
    { this->Sequence::_exec(tid + 1, real_stop_condition, this->sequences[tid + 1]); };

    this->thread_pool->run(func_exec, true);
    this->_exec(0, real_stop_condition, this->sequences[0]);
    this->thread_pool->wait();

    this->thread_pool->unset_func_exec();

    if (this->is_no_copy_mode() && !this->is_part_of_pipeline)
    {
        this->reset_no_copy_mode();
        this->gen_processes(false);
    }

    if (!this->prev_exception_messages_to_display.empty())
    {
        *force_exit_loop = false;
        throw std::runtime_error(this->prev_exception_messages_to_display.back());
    }
}

void
Sequence::exec(std::function<bool()> stop_condition)
{
    if (this->is_no_copy_mode()) this->gen_processes(true);

    std::function<bool()> real_stop_condition;
    if (this->auto_stop)
        real_stop_condition = [this, stop_condition]()
        {
            bool res = stop_condition();
            return res || this->is_done();
        };
    else
        real_stop_condition = stop_condition;

    std::function<void(const size_t)> func_exec = [this, &real_stop_condition](const size_t tid)
    { this->Sequence::_exec_without_statuses(tid + 1, real_stop_condition, this->sequences[tid + 1]); };

    this->thread_pool->run(func_exec, true);
    this->_exec_without_statuses(0, real_stop_condition, this->sequences[0]);
    this->thread_pool->wait();

    this->thread_pool->unset_func_exec();

    if (this->is_no_copy_mode() && !this->is_part_of_pipeline)
    {
        this->reset_no_copy_mode();
        this->gen_processes(false);
    }

    if (!this->prev_exception_messages_to_display.empty())
    {
        *force_exit_loop = false;
        throw std::runtime_error(this->prev_exception_messages_to_display.back());
    }
}

void
Sequence::exec()
{
    this->exec([]() { return false; });
}

void
Sequence::exec_seq(const size_t tid, const int frame_id)
{
    if (tid >= this->sequences.size())
    {
        std::stringstream message;
        message << "'tid' has to be smaller than 'sequences.size()' ('tid' = " << tid
                << ", 'sequences.size()' = " << this->sequences.size() << ").";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    std::function<void(tools::Digraph_node<Sub_sequence>*)> exec_sequence =
      [&exec_sequence, frame_id](tools::Digraph_node<Sub_sequence>* cur_ss)
    {
        auto type = cur_ss->get_c()->type;
        auto& tasks = cur_ss->get_c()->tasks;
        if (type == subseq_t::COMMUTE)
        {
            const int path = tasks[0]->exec(frame_id)[0];
            if (cur_ss->get_children().size() > (size_t)path) exec_sequence(cur_ss->get_children()[path]);
        }
        else
        {
            for (size_t ta = 0; ta < tasks.size(); ta++)
                tasks[ta]->exec(frame_id);
            for (auto c : cur_ss->get_children())
                exec_sequence(c);
        }
    };

    exec_sequence(this->sequences[tid]);
}

runtime::Task*
Sequence::exec_step(const size_t tid, const int frame_id)
{
    if (tid >= this->sequences.size())
    {
        std::stringstream message;
        message << "'tid' has to be smaller than 'sequences.size()' ('tid' = " << tid
                << ", 'sequences.size()' = " << this->sequences.size() << ").";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    runtime::Task* executed_task = nullptr;
    if (this->next_round_is_over[tid])
    {
        this->next_round_is_over[tid] = false;
        this->cur_ss[tid] = this->sequences[tid];
        this->cur_task_id[tid] = 0;
    }
    else
    {
        executed_task = this->cur_ss[tid]->get_c()->tasks[cur_task_id[tid]];
        const std::vector<int>& ret = executed_task->exec(frame_id);

        auto type = this->cur_ss[tid]->get_c()->type;
        if (type == subseq_t::COMMUTE)
        {
            const size_t path = (size_t)ret[0];
            if (this->cur_ss[tid]->get_children().size() > path)
            {
                this->cur_ss[tid] = this->cur_ss[tid]->get_children()[path];
                this->cur_task_id[tid] = 0;

                if (this->cur_ss[tid]->get_c()->tasks.size() == 0)
                {
                    // skip nodes without tasks if any
                    while (this->cur_ss[tid]->get_children().size() > 0)
                    {
                        this->cur_ss[tid] = this->cur_ss[tid]->get_children()[0];
                        this->cur_task_id[tid] = 0;
                        if (this->cur_ss[tid]->get_c() && this->cur_ss[tid]->get_c()->tasks.size() > 0) break;
                    }
                    if (this->cur_task_id[tid] >= this->cur_ss[tid]->get_c()->tasks.size())
                        this->next_round_is_over[tid] = true;
                }
            }
            else
            {
                std::stringstream message;
                message << "This should never happen ('path' = " << path
                        << ", 'cur_ss[tid]->get_children().size()' = " << this->cur_ss[tid]->get_children().size()
                        << ", 'tid' = " << tid << ").";
                throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
            }
        }
        else
        {
            this->cur_task_id[tid]++;
            if (this->cur_task_id[tid] == (this->cur_ss[tid]->get_c()->tasks.size()))
            {
                // skip nodes without tasks if any
                while (this->cur_ss[tid]->get_children().size() > 0)
                {
                    this->cur_ss[tid] = this->cur_ss[tid]->get_children()[0];
                    this->cur_task_id[tid] = 0;
                    if (this->cur_ss[tid]->get_c() && this->cur_ss[tid]->get_c()->tasks.size() > 0) break;
                }
                if (this->cur_task_id[tid] >= this->cur_ss[tid]->get_c()->tasks.size())
                    this->next_round_is_over[tid] = true;
            }
        }
    }

    return executed_task;
}

template<class SS, class TA>
tools::Digraph_node<SS>*
Sequence::init_recursive(tools::Digraph_node<SS>* cur_subseq,
                         size_t& ssid,
                         size_t& taid,
                         std::vector<std::pair<TA*, tools::Digraph_node<SS>*>>& selectors,
                         std::vector<TA*>& switchers,
                         TA& first,
                         TA& current_task,
                         const std::vector<TA*>& lasts,
                         const std::vector<TA*>& exclusions,
                         std::vector<size_t>& real_lasts_id,
                         std::vector<TA*>& real_lasts,
                         std::map<TA*, unsigned>& in_sockets_feed,
                         std::map<TA*, std::pair<tools::Digraph_node<SS>*, size_t>>& task_subseq)
{

    if (dynamic_cast<const module::Adaptor_m_to_n*>(&current_task.get_module()) && !this->tasks_inplace)
    {
        std::stringstream message;
        message << "'module::Adaptor_m_to_n' objects are not supported if 'tasks_inplace' is set to false.";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    auto it = std::find(real_lasts.begin(), real_lasts.end(), &current_task);
    if (it != real_lasts.end())
    {
        real_lasts.erase(it);
        auto dist = std::distance(real_lasts.begin(), it);
        real_lasts_id.erase(real_lasts_id.begin() + dist);
    }

    if (cur_subseq->get_contents() == nullptr)
    {
        cur_subseq->set_contents(new SS());
        ssid++;
    }

    bool is_last = true;
    tools::Digraph_node<SS>* last_subseq = nullptr;

    if (auto switcher = dynamic_cast<const module::Switcher*>(&current_task.get_module()))
    {
        const auto current_task_name = current_task.get_name();
        if (current_task_name == "commute" && // -------------------------------------------------------------- COMMUTE
            std::find(switchers.begin(), switchers.end(), &current_task) == switchers.end())
        {
            switchers.push_back(&current_task);
            auto node_commute = new tools::Digraph_node<SS>({ cur_subseq }, {}, nullptr, cur_subseq->get_depth() + 1);

            node_commute->set_contents(new SS());
            node_commute->get_c()->tasks.push_back(&current_task);
            node_commute->get_c()->tasks_id.push_back(taid++);
            node_commute->get_c()->type = subseq_t::COMMUTE;
            ssid++;

            cur_subseq->add_child(node_commute);

            for (size_t sdo = 0; sdo < switcher->get_n_data_sockets(); sdo++)
            {
                auto node_commute_son =
                  new tools::Digraph_node<SS>({ node_commute }, {}, nullptr, node_commute->get_depth() + 1);

                node_commute_son->set_contents(new SS());
                ssid++;

                node_commute->add_child(node_commute_son);

                auto& bss = (*switcher)[module::swi::tsk::commute].sockets[sdo + 2]->get_bound_sockets();
                for (auto bs : bss)
                {
                    if (bs == nullptr) continue;
                    auto& t = bs->get_task();
                    if (std::find(exclusions.begin(), exclusions.end(), &t) == exclusions.end())
                    {
                        if (task_subseq.find(&t) == task_subseq.end() || task_subseq[&t].second < ssid)
                            task_subseq[&t] = { node_commute_son, ssid };

                        in_sockets_feed.find(&t) != in_sockets_feed.end() ? in_sockets_feed[&t]++
                                                                          : in_sockets_feed[&t] = 1;
                        bool t_is_select =
                          dynamic_cast<const module::Switcher*>(&(t.get_module())) && t.get_name() == "select";
                        if ((!t_is_select && in_sockets_feed[&t] >= (t.get_n_input_sockets() + t.get_n_fwd_sockets()) -
                                                                      t.get_n_static_input_sockets()) ||
                            (t_is_select && t.is_last_input_socket(*bs)))
                        {
                            is_last = false;
                            last_subseq = Sequence::init_recursive<SS, TA>(task_subseq[&t].first,
                                                                           task_subseq[&t].second,
                                                                           taid,
                                                                           selectors,
                                                                           switchers,
                                                                           first,
                                                                           t,
                                                                           lasts,
                                                                           exclusions,
                                                                           real_lasts_id,
                                                                           real_lasts,
                                                                           in_sockets_feed,
                                                                           task_subseq);
                        }
                        else if (t_is_select)
                        {
                            tools::Digraph_node<SS>* node_selector = nullptr;
                            for (auto& sel : selectors)
                                if (sel.first == &t)
                                {
                                    node_selector = sel.second;
                                    break;
                                }

                            if (!node_selector)
                            {
                                node_selector = new tools::Digraph_node<SS>(
                                  { node_commute_son }, {}, nullptr, node_commute_son->get_depth() + 1);
                                selectors.push_back({ &t, node_selector });
                            }
                            else
                                node_selector->add_parent(node_commute_son);
                            node_commute_son->add_child(node_selector);
                        }
                    }
                }
            }
            // exception for the status socket
            auto& bss =
              (*switcher)[module::swi::tsk::commute].sockets[switcher->get_n_data_sockets() + 2]->get_bound_sockets();
            for (auto bs : bss)
            {
                if (bs == nullptr) continue;
                auto& t = bs->get_task();
                if (std::find(exclusions.begin(), exclusions.end(), &t) == exclusions.end())
                {
                    if (task_subseq.find(&t) == task_subseq.end() || task_subseq[&t].second < ssid)
                        task_subseq[&t] = { node_commute, ssid };

                    in_sockets_feed.find(&t) != in_sockets_feed.end() ? in_sockets_feed[&t]++ : in_sockets_feed[&t] = 1;
                    bool t_is_select =
                      dynamic_cast<const module::Switcher*>(&(t.get_module())) && t.get_name() == "select";
                    if ((!t_is_select && in_sockets_feed[&t] >= (t.get_n_input_sockets() + t.get_n_fwd_sockets()) -
                                                                  t.get_n_static_input_sockets()) ||
                        (t_is_select && t.is_last_input_socket(*bs)))
                    {
                        is_last = false;
                        last_subseq = Sequence::init_recursive<SS, TA>(task_subseq[&t].first,
                                                                       task_subseq[&t].second,
                                                                       taid,
                                                                       selectors,
                                                                       switchers,
                                                                       first,
                                                                       t,
                                                                       lasts,
                                                                       exclusions,
                                                                       real_lasts_id,
                                                                       real_lasts,
                                                                       in_sockets_feed,
                                                                       task_subseq);
                    }
                }
            }
        }
        else if (current_task_name == "select") // ------------------------------------------------------------- SELECT
        {
            tools::Digraph_node<SS>* node_selector = nullptr;

            for (auto& sel : selectors)
                if (sel.first == &current_task)
                {
                    node_selector = sel.second;
                    break;
                }

            if (!node_selector)
            {
                node_selector = new tools::Digraph_node<SS>({ cur_subseq }, {}, nullptr, cur_subseq->get_depth() + 1);
                selectors.push_back({ &current_task, node_selector });
            }

            node_selector->set_contents(new SS());
            node_selector->get_c()->tasks.push_back(&current_task);
            node_selector->get_c()->tasks_id.push_back(taid++);
            node_selector->get_c()->type = subseq_t::SELECT;
            ssid++;

            cur_subseq->add_child(node_selector);

            auto node_selector_son =
              new tools::Digraph_node<SS>({ node_selector }, {}, nullptr, node_selector->get_depth() + 1);

            node_selector_son->set_contents(new SS());
            ssid++;

            node_selector->add_child(node_selector_son);

            for (auto& s : current_task.sockets)
            {
                if (!(s->get_type() == socket_t::SOUT)) continue;
                auto bss = s->get_bound_sockets();
                for (auto bs : bss)
                {
                    if (bs == nullptr) continue;
                    auto& t = bs->get_task();
                    if (std::find(exclusions.begin(), exclusions.end(), &t) == exclusions.end())
                    {
                        if (task_subseq.find(&t) == task_subseq.end() || task_subseq[&t].second < ssid)
                            task_subseq[&t] = { node_selector_son, ssid };

                        in_sockets_feed.find(&t) != in_sockets_feed.end() ? in_sockets_feed[&t]++
                                                                          : in_sockets_feed[&t] = 1;
                        bool t_is_select =
                          dynamic_cast<const module::Switcher*>(&(t.get_module())) && t.get_name() == "select";

                        if ((!t_is_select && in_sockets_feed[&t] >= (t.get_n_input_sockets() + t.get_n_fwd_sockets()) -
                                                                      t.get_n_static_input_sockets()) ||
                            (t_is_select && t.is_last_input_socket(*bs)))
                        {
                            is_last = false;
                            last_subseq = Sequence::init_recursive<SS, TA>(task_subseq[&t].first,
                                                                           task_subseq[&t].second,
                                                                           taid,
                                                                           selectors,
                                                                           switchers,
                                                                           first,
                                                                           t,
                                                                           lasts,
                                                                           exclusions,
                                                                           real_lasts_id,
                                                                           real_lasts,
                                                                           in_sockets_feed,
                                                                           task_subseq);
                        }
                        else if (t_is_select)
                        {
                            tools::Digraph_node<SS>* node_selector = nullptr;
                            for (auto& sel : selectors)
                                if (sel.first == &t)
                                {
                                    node_selector = sel.second;
                                    break;
                                }

                            if (!node_selector)
                            {
                                node_selector = new tools::Digraph_node<SS>(
                                  { node_selector_son }, {}, nullptr, node_selector_son->get_depth() + 1);
                                selectors.push_back({ &t, node_selector });
                            }
                            node_selector->add_parent(node_selector_son);
                            node_selector_son->add_child(node_selector);
                        }
                    }
                }
            }
        }
    }
    else // --------------------------------------------------------------------------------------------- STANDARD CASE
    {
        cur_subseq->get_c()->tasks.push_back(&current_task);
        cur_subseq->get_c()->tasks_id.push_back(taid++);

        if (std::find(lasts.begin(), lasts.end(), &current_task) == lasts.end())
        {
            for (auto& s : current_task.sockets)
            {
                if (s->get_type() == socket_t::SOUT || s->get_type() == socket_t::SFWD)
                {
                    auto bss = s->get_bound_sockets();
                    for (auto& bs : bss)
                    {
                        if (bs == nullptr) continue;
                        auto& t = bs->get_task();
                        if (std::find(exclusions.begin(), exclusions.end(), &t) == exclusions.end())
                        {
                            if (task_subseq.find(&t) == task_subseq.end() || task_subseq[&t].second < ssid)
                                task_subseq[&t] = { cur_subseq, ssid };

                            in_sockets_feed.find(&t) != in_sockets_feed.end() ? in_sockets_feed[&t]++
                                                                              : in_sockets_feed[&t] = 1;
                            bool t_is_select =
                              dynamic_cast<const module::Switcher*>(&(t.get_module())) && t.get_name() == "select";
                            if ((!t_is_select &&
                                 in_sockets_feed[&t] >= (t.get_n_input_sockets() + t.get_n_fwd_sockets()) -
                                                          t.get_n_static_input_sockets()) ||
                                (t_is_select && t.is_last_input_socket(*bs)))
                            {
                                is_last = false;
                                last_subseq = Sequence::init_recursive<SS, TA>(task_subseq[&t].first,
                                                                               task_subseq[&t].second,
                                                                               taid,
                                                                               selectors,
                                                                               switchers,
                                                                               first,
                                                                               t,
                                                                               lasts,
                                                                               exclusions,
                                                                               real_lasts_id,
                                                                               real_lasts,
                                                                               in_sockets_feed,
                                                                               task_subseq);
                            }
                            else if (t_is_select)
                            {
                                tools::Digraph_node<SS>* node_selector = nullptr;
                                for (auto& sel : selectors)
                                    if (sel.first == &t)
                                    {
                                        node_selector = sel.second;
                                        break;
                                    }

                                if (!node_selector)
                                {
                                    node_selector = new tools::Digraph_node<SS>(
                                      { cur_subseq }, {}, nullptr, cur_subseq->get_depth() + 1);
                                    selectors.push_back({ &t, node_selector });
                                }
                                node_selector->add_parent(cur_subseq);
                                cur_subseq->add_child(node_selector);
                            }
                        }
                    }
                }
                else if (s->get_type() == socket_t::SIN && s->get_bound_sockets().size() > 1)
                {
                    std::stringstream message;
                    message << "'s->get_bound_sockets().size()' has to be smaller or equal to 1 ("
                            << "'s->get_bound_sockets().size()'"
                            << " = " << s->get_bound_sockets().size() << ", "
                            << "'s->get_type()'"
                            << " = "
                            << "socket_t::SIN"
                            << ", "
                            << "'s->get_name()'"
                            << " = " << s->get_name() << ", "
                            << "'s->get_task().get_name()'"
                            << " = " << s->get_task().get_name() << ", "
                            << "'s->get_task().get_module().get_name()'"
                            << " = " << s->get_task().get_module().get_name() << ").";
                    throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
                }
            }
        }
    }

    if (is_last && std::find(real_lasts.begin(), real_lasts.end(), &current_task) == real_lasts.end())
    {
        real_lasts.push_back(&current_task);
        real_lasts_id.push_back(cur_subseq->get_contents()->tasks_id.back());
    }

    if (last_subseq)
        return last_subseq;
    else
        return cur_subseq;
}

template<class SS, class MO>
void
Sequence::replicate(const tools::Digraph_node<SS>* sequence)
{
    std::set<MO*> modules_set;
    std::vector<const runtime::Task*> tsks_vec; // get a vector of tasks included in the tasks graph
    std::function<void(const tools::Digraph_node<SS>*, std::vector<const tools::Digraph_node<SS>*>&)>
      collect_modules_list;
    collect_modules_list =
      [&](const tools::Digraph_node<SS>* node, std::vector<const tools::Digraph_node<SS>*>& already_parsed_nodes)
    {
        if (node != nullptr &&
            std::find(already_parsed_nodes.begin(), already_parsed_nodes.end(), node) == already_parsed_nodes.end())
        {
            already_parsed_nodes.push_back(node);
            tsks_vec.insert(tsks_vec.end(), node->get_c()->tasks.begin(), node->get_c()->tasks.end());
            if (node->get_c())
                for (auto ta : node->get_c()->tasks)
                    modules_set.insert(&ta->get_module());
            for (auto c : node->get_children())
                collect_modules_list(c, already_parsed_nodes);
        }
    };
    std::vector<const tools::Digraph_node<SS>*> already_parsed_nodes;
    collect_modules_list(sequence, already_parsed_nodes);

    // check if all the tasks of the sequence are replicable before to perform the modules clones
    if (this->n_threads - (this->tasks_inplace ? 1 : 0))
        for (auto& t : tsks_vec)
            if (!t->is_replicable())
            {
                std::stringstream message;
                message << "It is not possible to replicate this sequence because at least one of its tasks is not "
                        << "replicable (t->get_name() = '" << t->get_name() << "', t->get_module().get_name() = '"
                        << t->get_module().get_name() << "').";
                throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
            }

    std::vector<MO*> modules_vec;
    for (auto m : modules_set)
        modules_vec.push_back(m);

    // clone the modules
    for (size_t tid = 0; tid < this->n_threads - (this->tasks_inplace ? 1 : 0); tid++)
    {
        if (this->is_thread_pinning())
        {
            const auto real_tid = tid + (this->tasks_inplace ? 1 : 0);
            if (!this->puids.empty())
                tools::Thread_pinning::pin(this->puids[real_tid]);
            else
                tools::Thread_pinning::pin(this->pin_objects_per_thread[real_tid]);
        }

        this->modules[tid].resize(modules_vec.size());
        this->all_modules[tid + (this->tasks_inplace ? 1 : 0)].resize(modules_vec.size());
        for (size_t m = 0; m < modules_vec.size(); m++)
        {
            try
            {
                this->modules[tid][m].reset(modules_vec[m]->clone());
            }
            catch (std::exception& e)
            {
                std::stringstream message;
                message << "Module clone failed when trying to replicate the sequence (module name is '"
                        << modules_vec[m]->get_name() << "').";

                throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
            }
            this->all_modules[tid + (this->tasks_inplace ? 1 : 0)][m] = this->modules[tid][m].get();
        }

        if (this->is_thread_pinning()) tools::Thread_pinning::unpin();
    }

    auto get_module_id = [](const std::vector<MO*>& modules, const module::Module& module)
    {
        int m_id;
        for (m_id = 0; m_id < (int)modules.size(); m_id++)
            if (modules[m_id] == &module) return m_id;
        return -1;
    };

    auto get_task_id = [](const std::vector<std::shared_ptr<runtime::Task>>& tasks, const runtime::Task& task)
    {
        int t_id;
        for (t_id = 0; t_id < (int)tasks.size(); t_id++)
            if (tasks[t_id].get() == &task) return t_id;
        return -1;
    };

    auto get_socket_id = [](const std::vector<std::shared_ptr<runtime::Socket>>& sockets, const runtime::Socket& socket)
    {
        int s_id;
        for (s_id = 0; s_id < (int)sockets.size(); s_id++)
            if (sockets[s_id].get() == &socket) return s_id;
        return -1;
    };

    std::function<void(const tools::Digraph_node<SS>*,
                       tools::Digraph_node<Sub_sequence>*,
                       const size_t,
                       std::vector<const tools::Digraph_node<SS>*>&,
                       std::map<size_t, tools::Digraph_node<Sub_sequence>*>&)>
      replicate_sequence;

    replicate_sequence = [&](const tools::Digraph_node<SS>* sequence_ref,
                             tools::Digraph_node<Sub_sequence>* sequence_cpy,
                             const size_t thread_id,
                             std::vector<const tools::Digraph_node<SS>*>& already_parsed_nodes,
                             std::map<size_t, tools::Digraph_node<Sub_sequence>*>& allocated_nodes)
    {
        if (sequence_ref != nullptr && sequence_ref->get_c() &&
            std::find(already_parsed_nodes.begin(), already_parsed_nodes.end(), sequence_ref) ==
              already_parsed_nodes.end())
        {
            already_parsed_nodes.push_back(sequence_ref);

            auto ss_ref = sequence_ref->get_c();
            auto ss_cpy = new Sub_sequence();

            ss_cpy->type = ss_ref->type;
            ss_cpy->id = ss_ref->id;
            ss_cpy->tasks_id = ss_ref->tasks_id;
            for (auto t_ref : ss_ref->tasks)
            {
                auto& m_ref = t_ref->get_module();

                auto m_id = get_module_id(modules_vec, m_ref);
                auto t_id = get_task_id(m_ref.tasks, *t_ref);

                assert(m_id != -1);
                assert(t_id != -1);

                // add the task to the sub-sequence
                ss_cpy->tasks.push_back(this->all_modules[thread_id][m_id]->tasks[t_id].get());

                // replicate the sockets binding
                for (size_t s_id = 0; s_id < t_ref->sockets.size(); s_id++)
                {
                    if (t_ref->sockets[s_id]->get_type() == socket_t::SIN ||
                        t_ref->sockets[s_id]->get_type() == socket_t::SFWD)
                    {
                        const runtime::Socket* s_ref_out = nullptr;
                        try
                        {
                            s_ref_out = &t_ref->sockets[s_id]->get_bound_socket();
                        }
                        catch (...)
                        {
                        }
                        if (s_ref_out)
                        {
                            auto& t_ref_out = s_ref_out->get_task();
                            auto& m_ref_out = t_ref_out.get_module();

                            // check if `t_ref_out` is included in the tasks graph
                            auto t_in_seq = std::find(tsks_vec.begin(), tsks_vec.end(), &t_ref_out) != tsks_vec.end();
                            auto m_id_out = get_module_id(modules_vec, m_ref_out);

                            if (t_in_seq && m_id_out != -1)
                            {
                                auto t_id_out = get_task_id(m_ref_out.tasks, t_ref_out);
                                auto s_id_out = get_socket_id(t_ref_out.sockets, *s_ref_out);

                                assert(t_id_out != -1);
                                assert(s_id_out != -1);

                                auto& s_in = *this->all_modules[thread_id][m_id]->tasks[t_id]->sockets[s_id];
                                auto& s_out =
                                  *this->all_modules[thread_id][m_id_out]->tasks[t_id_out]->sockets[s_id_out];
                                s_in = s_out;
                            }
                        }
                    }
                }

                // replicate the tasks binding
                if (t_ref->fake_input_sockets.size())
                {
                    for (auto& fsi : t_ref->fake_input_sockets)
                    {
                        const runtime::Socket* s_ref_out = nullptr;
                        try
                        {
                            s_ref_out = &fsi->get_bound_socket();
                        }
                        catch (...)
                        {
                        }
                        if (s_ref_out)
                        {
                            auto& t_ref_out = s_ref_out->get_task();
                            auto& m_ref_out = t_ref_out.get_module();

                            // check if `t_ref_out` is included in the tasks graph
                            auto t_in_seq = std::find(tsks_vec.begin(), tsks_vec.end(), &t_ref_out) != tsks_vec.end();
                            auto m_id_out = get_module_id(modules_vec, m_ref_out);

                            if (t_in_seq && m_id_out != -1)
                            {
                                auto t_id_out = get_task_id(m_ref_out.tasks, t_ref_out);
                                auto s_id_out = get_socket_id(t_ref_out.sockets, *s_ref_out);

                                assert(t_id_out != -1);
                                assert(s_id_out != -1);

                                auto& t_in = *this->all_modules[thread_id][m_id]->tasks[t_id];
                                auto& s_out =
                                  *this->all_modules[thread_id][m_id_out]->tasks[t_id_out]->sockets[s_id_out];
                                t_in = s_out;
                            }
                        }
                    }
                }
            }

            // add the sub-sequence to the current tree node
            sequence_cpy->set_contents(ss_cpy);
            allocated_nodes[sequence_cpy->get_c()->id] = sequence_cpy;

            for (size_t c = 0; c < sequence_ref->get_children().size(); c++)
            {
                if (std::find(already_parsed_nodes.begin(),
                              already_parsed_nodes.end(),
                              sequence_ref->get_children()[c]) != already_parsed_nodes.end())
                    sequence_cpy->add_child(allocated_nodes[sequence_ref->get_children()[c]->get_c()->id]);
                else
                    sequence_cpy->add_child(new tools::Digraph_node<Sub_sequence>(
                      { sequence_cpy }, {}, nullptr, sequence_cpy->get_depth() + 1));
            }

            for (size_t c = 0; c < sequence_ref->get_children().size(); c++)
                replicate_sequence(sequence_ref->get_children()[c],
                                   sequence_cpy->get_children()[c],
                                   thread_id,
                                   already_parsed_nodes,
                                   allocated_nodes);
        }
    };

    for (size_t thread_id = (this->tasks_inplace ? 1 : 0); thread_id < this->sequences.size(); thread_id++)
    {
        if (this->is_thread_pinning())
        {
            if (!this->puids.empty())
                tools::Thread_pinning::pin(this->puids[thread_id]);
            else
                tools::Thread_pinning::pin(this->pin_objects_per_thread[thread_id]);
        }

        this->sequences[thread_id] = new tools::Digraph_node<Sub_sequence>({}, {}, nullptr, 0);
        already_parsed_nodes.clear();
        std::map<size_t, tools::Digraph_node<Sub_sequence>*> allocated_nodes;
        replicate_sequence(sequence, this->sequences[thread_id], thread_id, already_parsed_nodes, allocated_nodes);

        if (this->is_thread_pinning()) tools::Thread_pinning::unpin();
    }
}

template void
runtime::Sequence::replicate<runtime::Sub_sequence_const, const module::Module>(
  const tools::Digraph_node<runtime::Sub_sequence_const>*);
template void
runtime::Sequence::replicate<runtime::Sub_sequence, module::Module>(const tools::Digraph_node<runtime::Sub_sequence>*);

template<class SS>
void
Sequence::delete_tree(tools::Digraph_node<SS>* node, std::vector<tools::Digraph_node<SS>*>& already_deleted_nodes)
{
    if (node != nullptr &&
        std::find(already_deleted_nodes.begin(), already_deleted_nodes.end(), node) == already_deleted_nodes.end())
    {
        already_deleted_nodes.push_back(node);
        for (auto c : node->get_children())
            this->delete_tree(c, already_deleted_nodes);
        auto c = node->get_c();
        if (c != nullptr) delete c;
        delete node;
    }
}

template void
runtime::Sequence::delete_tree<runtime::Sub_sequence_const>(
  tools::Digraph_node<runtime::Sub_sequence_const>*,
  std::vector<tools::Digraph_node<Sub_sequence_const>*>& already_deleted_nodes);
template void
runtime::Sequence::delete_tree<runtime::Sub_sequence>(
  tools::Digraph_node<runtime::Sub_sequence>*,
  std::vector<tools::Digraph_node<Sub_sequence>*>& already_deleted_nodes);

template<class VTA>
void
Sequence::export_dot_subsequence(const VTA& subseq,
                                 const std::vector<size_t>& tasks_id,
                                 const subseq_t& subseq_type,
                                 const std::string& subseq_name,
                                 const std::string& tab,
                                 std::ostream& stream) const
{
    if (!subseq_name.empty())
    {
        stream << tab << "subgraph \"cluster_" << subseq_name << "_" << +this << "\" {" << std::endl;
        stream << tab << tab << "node [style=filled];" << std::endl;
    }
    size_t exec_order = 0;
    for (auto& t : subseq)
    {
        std::string color = dynamic_cast<module::Adaptor_m_to_n*>(&t->get_module()) ? "green" : "blue";
        color = dynamic_cast<module::AProbe*>(&t->get_module()) ? "pink" : color;
        stream << tab << tab << "subgraph \"cluster_" << +&t->get_module() << "_" << +t << "\" {" << std::endl;
        stream << tab << tab << tab << "node [style=filled];" << std::endl;
        stream << tab << tab << tab << "subgraph \"cluster_" << +&t << "\" {" << std::endl;
        stream << tab << tab << tab << tab << "node [style=filled];" << std::endl;

        if (t->fake_input_sockets.size())
        {
            std::string stype = "in";
            for (auto& fsi : t->fake_input_sockets)
                stream << tab << tab << tab << tab << "\"" << +fsi.get() << "\""
                       << "[label=\"" << stype << ":" << fsi->get_name() << "\", style=filled, "
                       << "fillcolor=red, penwidth=\"2.0\"];" << std::endl;
        }

        size_t sid = 0;
        for (auto& s : t->sockets)
        {
            std::string stype;
            bool static_input = false;
            switch (s->get_type())
            {
                case socket_t::SIN:
                    stype = "in[" + std::to_string(sid) + "]";
                    static_input = s->_get_dataptr() != nullptr && s->bound_socket == nullptr;
                    break;
                case socket_t::SOUT:
                    stype = "out[" + std::to_string(sid) + "]";
                    break;
                case socket_t::SFWD:
                    stype = "fwd[" + std::to_string(sid) + "]";
                    break;
                default:
                    stype = "unkn";
                    break;
            }

            std::string bold_or_not;
            if (t->is_last_input_socket(*s)) bold_or_not = ", penwidth=\"2.0\"";

            stream << tab << tab << tab << tab << "\"" << +s.get() << "\""
                   << "[label=\"" << stype << ":" << s->get_name() << "\"" << bold_or_not
                   << (static_input ? ", style=filled, fillcolor=green" : "") << "];" << std::endl;
            sid++;
        }

        stream << tab << tab << tab << tab << "label=\"" << t->get_name() << " (id = " << tasks_id[exec_order] << ")"
               << "\";" << std::endl;
        stream << tab << tab << tab << tab << "color=" << (t->is_replicable() ? color : "red") << ";" << std::endl;
        stream << tab << tab << tab << "}" << std::endl;
        stream << tab << tab << tab << "label=\"" << t->get_module().get_name() << "\n"
               << (t->get_module().get_custom_name().empty() ? "" : t->get_module().get_custom_name() + "\n")
               << "exec order: [" << exec_order++ << "]\n"
               << "addr: " << +&t->get_module() << "\";" << std::endl;
        stream << tab << tab << tab << "color=" << color << ";" << std::endl;
        stream << tab << tab << "}" << std::endl;
    }
    if (!subseq_name.empty())
    {
        stream << tab << tab << "label=\"" << subseq_name << "\";" << std::endl;
        std::string color = subseq_type == subseq_t::COMMUTE || subseq_type == subseq_t::SELECT ? "red" : "blue";
        stream << tab << tab << "color=" << color << ";" << std::endl;
        stream << tab << "}" << std::endl;
    }
}

template void
runtime::Sequence::export_dot_subsequence<std::vector<runtime::Task*>>(const std::vector<runtime::Task*>&,
                                                                       const std::vector<size_t>&,
                                                                       const subseq_t&,
                                                                       const std::string&,
                                                                       const std::string&,
                                                                       std::ostream&) const;
template void
runtime::Sequence::export_dot_subsequence<std::vector<const runtime::Task*>>(const std::vector<const runtime::Task*>&,
                                                                             const std::vector<size_t>&,
                                                                             const subseq_t&,
                                                                             const std::string&,
                                                                             const std::string&,
                                                                             std::ostream&) const;

template<class VTA>
void
Sequence::export_dot_connections(const VTA& subseq, const std::string& tab, std::ostream& stream) const
{
    for (auto& t : subseq)
    {
        for (auto& s : t->sockets)
        {
            if (s->get_type() == socket_t::SOUT || s->get_type() == socket_t::SFWD)
            {
                auto& bss = s->get_bound_sockets();
                size_t id = 0;
                for (auto& bs : bss)
                {
                    stream << tab << "\"" << +s.get() << "\" -> \"" << +bs << "\""
                           << (bss.size() > 1 ? "[label=\"" + std::to_string(id++) + "\"]" : "") << std::endl;
                }
            }
        }
    }
}

template void
runtime::Sequence::export_dot_connections<std::vector<runtime::Task*>>(const std::vector<runtime::Task*>&,
                                                                       const std::string&,
                                                                       std::ostream&) const;
template void
runtime::Sequence::export_dot_connections<std::vector<const runtime::Task*>>(const std::vector<const runtime::Task*>&,
                                                                             const std::string&,
                                                                             std::ostream&) const;

void
Sequence::export_dot(std::ostream& stream) const
{
    auto root = this->sequences[0];
    this->export_dot(root, stream);
}

template<class SS>
void
Sequence::export_dot(tools::Digraph_node<SS>* root, std::ostream& stream) const
{
    std::function<void(
      tools::Digraph_node<SS>*, const std::string&, std::ostream&, std::vector<tools::Digraph_node<SS>*>&)>
      export_dot_subsequences_recursive =
        [&export_dot_subsequences_recursive, this](tools::Digraph_node<SS>* cur_node,
                                                   const std::string& tab,
                                                   std::ostream& stream,
                                                   std::vector<tools::Digraph_node<SS>*>& already_parsed_nodes)
    {
        if (cur_node != nullptr &&
            std::find(already_parsed_nodes.begin(), already_parsed_nodes.end(), cur_node) == already_parsed_nodes.end())
        {
            already_parsed_nodes.push_back(cur_node);
            this->export_dot_subsequence(cur_node->get_c()->tasks,
                                         cur_node->get_c()->tasks_id,
                                         cur_node->get_c()->type,
                                         "Sub-sequence " + std::to_string(cur_node->get_c()->id) +
                                           " (depth = " + std::to_string(cur_node->get_depth()) + ")",
                                         tab,
                                         stream);

            for (auto c : cur_node->get_children())
                export_dot_subsequences_recursive(c, tab, stream, already_parsed_nodes);
        }
    };

    std::function<void(
      tools::Digraph_node<SS>*, const std::string&, std::ostream&, std::vector<tools::Digraph_node<SS>*>&)>
      export_dot_connections_recursive =
        [&export_dot_connections_recursive, this](tools::Digraph_node<SS>* cur_node,
                                                  const std::string& tab,
                                                  std::ostream& stream,
                                                  std::vector<tools::Digraph_node<SS>*>& already_parsed_nodes)
    {
        if (cur_node != nullptr &&
            std::find(already_parsed_nodes.begin(), already_parsed_nodes.end(), cur_node) == already_parsed_nodes.end())
        {
            already_parsed_nodes.push_back(cur_node);
            this->export_dot_connections(cur_node->get_c()->tasks, tab, stream);

            for (auto c : cur_node->get_children())
                export_dot_connections_recursive(c, tab, stream, already_parsed_nodes);
        }
    };

    std::string tab = "\t";
    stream << "digraph Sequence {" << std::endl;
    std::vector<tools::Digraph_node<SS>*> already_parsed_nodes;
    export_dot_subsequences_recursive(root, tab, stream, already_parsed_nodes);
    already_parsed_nodes.clear();
    export_dot_connections_recursive(root, tab, stream, already_parsed_nodes);
    stream << "}" << std::endl;
}

void
Sequence::gen_processes(const bool no_copy_mode)
{
    std::function<void(Socket * socket, std::vector<runtime::Socket*> & list_fwd)> explore_thread_rec =
      [&explore_thread_rec](Socket* socket, std::vector<runtime::Socket*>& list_fwd)
    {
        auto bound = socket->get_bound_sockets();
        for (auto explore_bound : bound)
        {
            if (find(list_fwd.begin(), list_fwd.end(), explore_bound) == list_fwd.end() &&
                explore_bound->get_type() != socket_t::SOUT)
            {
                list_fwd.push_back(explore_bound);
            }
            if (explore_bound->get_type() == socket_t::SFWD) explore_thread_rec(explore_bound, list_fwd);
        }
    };

    std::function<void(Socket * socket, std::vector<runtime::Socket*> & list_fwd)> explore_thread_rec_reverse =
      [&explore_thread_rec, &explore_thread_rec_reverse](Socket* socket, std::vector<runtime::Socket*>& list_fwd)
    {
        auto bound = &socket->get_bound_socket();
        if (find(list_fwd.begin(), list_fwd.end(), bound) == list_fwd.end())
        {
            list_fwd.push_back(bound);
        }
        if (bound->get_type() == socket_t::SFWD)
        {
            explore_thread_rec(bound, list_fwd);
            explore_thread_rec_reverse(bound, list_fwd);
        }
    };

    std::function<void(tools::Digraph_node<Sub_sequence>*, std::vector<tools::Digraph_node<Sub_sequence>*>&)>
      gen_processes_recursive =
        [&gen_processes_recursive, no_copy_mode, &explore_thread_rec, &explore_thread_rec_reverse](
          tools::Digraph_node<Sub_sequence>* cur_node,
          std::vector<tools::Digraph_node<Sub_sequence>*>& already_parsed_nodes)
    {
        if (cur_node != nullptr &&
            std::find(already_parsed_nodes.begin(), already_parsed_nodes.end(), cur_node) == already_parsed_nodes.end())
        {
            already_parsed_nodes.push_back(cur_node);

            std::map<runtime::Task*, std::function<const int*()>> modified_tasks;
            auto contents = cur_node->get_c();
            contents->processes.clear();
            contents->rebind_sockets.clear();
            contents->rebind_dataptrs.clear();
            for (auto task : contents->tasks)
            {
                if (dynamic_cast<module::Switcher*>(&task->get_module()) &&
                    task->get_name().find("select") != std::string::npos && no_copy_mode)
                {
                    auto select_task = task;
                    auto switcher = dynamic_cast<module::Switcher*>(&select_task->get_module());
                    switcher->set_no_copy_select(true);

                    const auto rebind_id = contents->rebind_sockets.size();
                    contents->rebind_sockets.resize(rebind_id + 1);
                    contents->rebind_dataptrs.resize(rebind_id + 1);

                    for (size_t s = 0; s < select_task->sockets.size() - 1; s++)
                    {
                        // there should be only one output socket at this time
                        if (select_task->sockets[s]->get_type() == socket_t::SOUT)
                        {
                            std::vector<runtime::Socket*> bound_sockets;
                            std::vector<void*> dataptrs;

                            for (auto socket : select_task->sockets[s]->get_bound_sockets())
                            {
                                bound_sockets.push_back(socket);
                                if (socket->get_type() == socket_t::SFWD) explore_thread_rec(socket, bound_sockets);
                            }
                            for (auto sck : bound_sockets)
                                dataptrs.push_back(sck->_get_dataptr());

                            contents->rebind_sockets[rebind_id].push_back(bound_sockets);
                            contents->rebind_dataptrs[rebind_id].push_back(dataptrs);
                        }
                    }

                    modified_tasks[select_task] = [contents, select_task, switcher, rebind_id]() -> const int*
                    {
                        select_task->exec();
                        const int* status = select_task->sockets.back()->get_dataptr<int>();

                        const auto path = switcher->get_path();
                        const auto in_dataptr = select_task->sockets[path]->_get_dataptr();

                        // rebind input sockets on the fly
                        // there should be only one output socket at this time (sout_id == 0)
                        for (size_t sout_id = 0; sout_id < contents->rebind_sockets[rebind_id].size(); sout_id++)
                            for (size_t sin_id = 0; sin_id < contents->rebind_sockets[rebind_id][sout_id].size();
                                 sin_id++)
                                contents->rebind_sockets[rebind_id][sout_id][sin_id]->dataptr = in_dataptr;

                        return status;
                    };
                }

                if (dynamic_cast<module::Switcher*>(&task->get_module()) &&
                    task->get_name().find("commute") != std::string::npos && no_copy_mode)
                {
                    auto commute_task = task;
                    auto switcher = dynamic_cast<module::Switcher*>(&commute_task->get_module());
                    switcher->set_no_copy_commute(true);

                    const auto rebind_id = contents->rebind_sockets.size();
                    contents->rebind_sockets.resize(rebind_id + 1);
                    contents->rebind_dataptrs.resize(rebind_id + 1);

                    for (size_t s = 0; s < commute_task->sockets.size() - 1; s++)
                    {
                        if (commute_task->sockets[s]->get_type() == socket_t::SOUT)
                        {
                            std::vector<runtime::Socket*> bound_sockets;
                            std::vector<void*> dataptrs;

                            for (auto socket : commute_task->sockets[s]->get_bound_sockets())
                            {
                                bound_sockets.push_back(socket);
                                if (socket->get_type() == socket_t::SFWD) explore_thread_rec(socket, bound_sockets);
                            }
                            for (auto sck : bound_sockets)
                                dataptrs.push_back(sck->_get_dataptr());

                            contents->rebind_sockets[rebind_id].push_back(bound_sockets);
                            contents->rebind_dataptrs[rebind_id].push_back(dataptrs);
                        }
                    }

                    modified_tasks[commute_task] = [contents, commute_task, switcher, rebind_id]() -> const int*
                    {
                        commute_task->exec();
                        const int* status = commute_task->sockets.back()->get_dataptr<int>();
                        const auto in_dataptr = commute_task->sockets[0]->_get_dataptr();
                        const auto path = switcher->get_path();

                        // rebind input sockets on the fly
                        for (size_t sin_id = 0; sin_id < contents->rebind_sockets[rebind_id][path].size(); sin_id++)
                            contents->rebind_sockets[rebind_id][path][sin_id]->dataptr = in_dataptr;

                        return status;
                    };
                }

                if (dynamic_cast<module::Adaptor_m_to_n*>(&task->get_module()) &&
                    task->get_name().find("pull") != std::string::npos && no_copy_mode)
                {
                    auto pull_task = task;
                    auto adp_pull = dynamic_cast<module::Adaptor_m_to_n*>(&pull_task->get_module());
                    adp_pull->set_no_copy_pull(true);
                    const auto rebind_id = contents->rebind_sockets.size();
                    contents->rebind_sockets.resize(rebind_id + 1);
                    contents->rebind_dataptrs.resize(rebind_id + 1);

                    for (size_t s = 0; s < pull_task->sockets.size() - 1; s++)
                    {
                        if (pull_task->sockets[s]->get_type() == socket_t::SOUT)
                        {
                            std::vector<runtime::Socket*> bound_sockets;
                            std::vector<void*> dataptrs;

                            bound_sockets.push_back(pull_task->sockets[s].get());
                            for (auto socket : pull_task->sockets[s]->get_bound_sockets())
                            {
                                bound_sockets.push_back(socket);
                                if (socket->get_type() == socket_t::SFWD) explore_thread_rec(socket, bound_sockets);
                            }
                            for (auto sck : bound_sockets)
                                dataptrs.push_back(sck->_get_dataptr());

                            contents->rebind_sockets[rebind_id].push_back(bound_sockets);
                            contents->rebind_dataptrs[rebind_id].push_back(dataptrs);
                        }
                    }

                    modified_tasks[pull_task] = [contents, pull_task, adp_pull, rebind_id]() -> const int*
                    {
                        // active or passive waiting here
                        pull_task->exec();
                        const int* status = pull_task->sockets.back()->get_dataptr<int>();

                        // rebind input sockets on the fly
                        for (size_t sin_id = 0; sin_id < contents->rebind_sockets[rebind_id].size(); sin_id++)
                        {
                            if (contents->rebind_sockets[rebind_id][sin_id].size() > 1)
                            {
                                // we start to 1 because the rebinding of the 'pull_task' is made in the
                                // 'pull_task->exec()' call (this way the debug mode is still working)
                                auto swap_buff = contents->rebind_sockets[rebind_id][sin_id][1]->_get_dataptr();
                                auto buff = adp_pull->get_filled_buffer(sin_id, swap_buff);
                                contents->rebind_sockets[rebind_id][sin_id][1]->dataptr = buff;
                                // for the next tasks the same buffer 'buff' is required, an easy mistake is to re-swap
                                // and the data will be false, this is why we just bind 'buff'
                                for (size_t ta = 2; ta < contents->rebind_sockets[rebind_id][sin_id].size(); ta++)
                                    contents->rebind_sockets[rebind_id][sin_id][ta]->dataptr = buff;
                            }
                        }
                        adp_pull->wake_up_pusher();
                        return status;
                    };
                }

                if (dynamic_cast<module::Adaptor_m_to_n*>(&task->get_module()) &&
                    task->get_name().find("push") != std::string::npos && no_copy_mode)
                {
                    auto push_task = task;
                    auto adp_push = dynamic_cast<module::Adaptor_m_to_n*>(&push_task->get_module());
                    adp_push->set_no_copy_push(true);
                    const auto rebind_id = contents->rebind_sockets.size();
                    contents->rebind_sockets.resize(rebind_id + 1);
                    contents->rebind_dataptrs.resize(rebind_id + 1);

                    for (size_t s = 0; s < push_task->sockets.size() - 1; s++)
                        if (push_task->sockets[s]->get_type() == socket_t::SIN)
                        {
                            std::vector<runtime::Socket*> bound_sockets;
                            std::vector<void*> dataptrs;

                            bound_sockets.push_back(push_task->sockets[s].get());

                            auto bound_socket = &push_task->sockets[s]->get_bound_socket();
                            bound_sockets.push_back(bound_socket);
                            explore_thread_rec(bound_socket, bound_sockets);

                            // If the socket is FWD, we have to update all the other sockets with a backward
                            // exploration
                            if (bound_socket->get_type() == socket_t::SFWD)
                                explore_thread_rec_reverse(bound_socket, bound_sockets);

                            for (auto sck : bound_sockets)
                                dataptrs.push_back(sck->_get_dataptr());

                            contents->rebind_sockets[rebind_id].push_back(bound_sockets);
                            contents->rebind_dataptrs[rebind_id].push_back(dataptrs);
                        }

                    modified_tasks[push_task] = [contents, push_task, adp_push, rebind_id]() -> const int*
                    {
                        // active or passive waiting here
                        push_task->exec();
                        const int* status = push_task->sockets.back()->get_dataptr<int>();
                        // rebind output sockets on the fly
                        for (size_t sout_id = 0; sout_id < contents->rebind_sockets[rebind_id].size(); sout_id++)
                        {
                            // we start to 1 because the rebinding of the 'push_task' is made in the
                            // 'push_task->exec()' call (this way the debug mode is still working)
                            auto swap_buff = contents->rebind_sockets[rebind_id][sout_id][1]->_get_dataptr();
                            auto buff = adp_push->get_empty_buffer(sout_id, swap_buff);
                            contents->rebind_sockets[rebind_id][sout_id][1]->dataptr = buff;
                            // the output socket linked to the push adp can have more than one socket bound and so
                            // we have to rebind all the input sokects bound to the current output socket
                            for (size_t ta = 2; ta < contents->rebind_sockets[rebind_id][sout_id].size(); ta++)
                                contents->rebind_sockets[rebind_id][sout_id][ta]->dataptr = buff;
                        }
                        adp_push->wake_up_puller();
                        return status;
                    };
                }
            }

            for (auto task : contents->tasks)
                if (modified_tasks.count(task))
                    contents->processes.push_back(modified_tasks[task]);
                else
                    contents->processes.push_back(
                      [task]() -> const int*
                      {
                          task->exec();
                          const int* status = task->sockets.back()->get_dataptr<int>();
                          return status;
                      });

            for (auto c : cur_node->get_children())
                gen_processes_recursive(c, already_parsed_nodes);
        }
    };

    size_t thread_id = 0;
    for (auto& sequence : this->sequences)
    {
        if (this->is_thread_pinning())
        {
            if (!this->puids.empty())
                tools::Thread_pinning::pin(this->puids[thread_id++]);
            else
                tools::Thread_pinning::pin(this->pin_objects_per_thread[thread_id++]);
        }
        std::vector<tools::Digraph_node<Sub_sequence>*> already_parsed_nodes;
        gen_processes_recursive(sequence, already_parsed_nodes);

        if (this->is_thread_pinning()) tools::Thread_pinning::unpin();
    }
}

void
Sequence::reset_no_copy_mode()
{
    std::function<void(tools::Digraph_node<Sub_sequence>*, std::vector<tools::Digraph_node<Sub_sequence>*>&)>
      reset_no_copy_mode_recursive =
        [&reset_no_copy_mode_recursive](tools::Digraph_node<Sub_sequence>* cur_node,
                                        std::vector<tools::Digraph_node<Sub_sequence>*>& already_parsed_nodes)
    {
        if (cur_node != nullptr &&
            std::find(already_parsed_nodes.begin(), already_parsed_nodes.end(), cur_node) == already_parsed_nodes.end())
        {
            already_parsed_nodes.push_back(cur_node);
            auto contents = cur_node->get_c();
            for (auto task : contents->tasks)
            {
                if (dynamic_cast<module::Switcher*>(&task->get_module()) &&
                    task->get_name().find("select") != std::string::npos)
                {
                    auto select_task = task;
                    auto switcher = dynamic_cast<module::Switcher*>(&select_task->get_module());
                    switcher->set_no_copy_select(false);
                }

                if (dynamic_cast<module::Switcher*>(&task->get_module()) &&
                    task->get_name().find("commute") != std::string::npos)
                {
                    auto commute_task = task;
                    auto switcher = dynamic_cast<module::Switcher*>(&commute_task->get_module());
                    switcher->set_no_copy_commute(false);
                }

                if (dynamic_cast<module::Adaptor_m_to_n*>(&task->get_module()) &&
                    task->get_name().find("pull") != std::string::npos)
                {
                    auto pull_task = task;
                    auto adp_pull = dynamic_cast<module::Adaptor_m_to_n*>(&pull_task->get_module());
                    adp_pull->set_no_copy_pull(false);
                    adp_pull->reset_buffer();
                }

                if (dynamic_cast<module::Adaptor_m_to_n*>(&task->get_module()) &&
                    task->get_name().find("push") != std::string::npos)
                {
                    auto push_task = task;
                    auto adp_push = dynamic_cast<module::Adaptor_m_to_n*>(&push_task->get_module());
                    adp_push->set_no_copy_push(false);
                    adp_push->reset_buffer();
                }
            }

            for (size_t rebind_id = 0; rebind_id < contents->rebind_sockets.size(); rebind_id++)
                for (size_t s = 0; s < contents->rebind_sockets[rebind_id].size(); s++)
                    for (size_t ta = 0; ta < contents->rebind_sockets[rebind_id][s].size(); ta++)
                        contents->rebind_sockets[rebind_id][s][ta]->dataptr =
                          contents->rebind_dataptrs[rebind_id][s][ta];

            for (auto c : cur_node->get_children())
                reset_no_copy_mode_recursive(c, already_parsed_nodes);
        }
    };

    for (auto& sequence : this->sequences)
    {
        std::vector<tools::Digraph_node<Sub_sequence>*> already_parsed_nodes;
        reset_no_copy_mode_recursive(sequence, already_parsed_nodes);
    }
}

void
Sequence::set_no_copy_mode(const bool no_copy_mode)
{
    this->no_copy_mode = no_copy_mode;
}

bool
Sequence::is_no_copy_mode() const
{
    return this->no_copy_mode;
}

void
Sequence::set_auto_stop(const bool auto_stop)
{
    this->auto_stop = auto_stop;
}

bool
Sequence::is_auto_stop() const
{
    return this->auto_stop;
}

Sub_sequence*
Sequence::get_last_subsequence(const size_t tid)
{
    std::function<Sub_sequence*(tools::Digraph_node<Sub_sequence>*, std::vector<tools::Digraph_node<Sub_sequence>*>&)>
      get_last_subsequence_recursive =
        [&get_last_subsequence_recursive,
         &tid](tools::Digraph_node<Sub_sequence>* cur_node,
               std::vector<tools::Digraph_node<Sub_sequence>*>& already_parsed_nodes) -> Sub_sequence*
    {
        if (cur_node != nullptr &&
            std::find(already_parsed_nodes.begin(), already_parsed_nodes.end(), cur_node) == already_parsed_nodes.end())
        {
            already_parsed_nodes.push_back(cur_node);
            if (!cur_node->get_children().size()) return cur_node->get_contents();
            Sub_sequence* last_ss = nullptr;
            for (auto c : cur_node->get_children())
            {
                Sub_sequence* last_branch_ss = nullptr;
                last_branch_ss = get_last_subsequence_recursive(c, already_parsed_nodes);
                if (last_ss && last_branch_ss && last_ss != last_branch_ss)
                {
                    std::stringstream message;
                    message << "Multiple candidates have been found for the last subsequence, this shouldn't be "
                            << "possible. (tid = " << tid << ", "
                            << "last_ss.id = " << last_ss->id << ", "
                            << "last_branch_ss.id = " << last_branch_ss->id << ")";
                    throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
                }
                last_ss = last_branch_ss ? last_branch_ss : last_ss;
            }
            return last_ss;
        }
        return nullptr;
    };

    std::vector<tools::Digraph_node<Sub_sequence>*> already_parsed_nodes;
    return get_last_subsequence_recursive(this->sequences[tid], already_parsed_nodes);
}

void
Sequence::update_tasks_id(const size_t tid)
{
    std::function<void(tools::Digraph_node<Sub_sequence>*, std::vector<tools::Digraph_node<Sub_sequence>*>&)>
      update_tasks_id_recursive =
        [&update_tasks_id_recursive](tools::Digraph_node<Sub_sequence>* cur_node,
                                     std::vector<tools::Digraph_node<Sub_sequence>*>& already_parsed_nodes)
    {
        if (cur_node != nullptr &&
            std::find(already_parsed_nodes.begin(), already_parsed_nodes.end(), cur_node) == already_parsed_nodes.end())
        {
            already_parsed_nodes.push_back(cur_node);
            Sub_sequence* ss = cur_node->get_contents();
            ss->tasks_id.resize(ss->tasks.size());
            std::iota(ss->tasks_id.begin(), ss->tasks_id.end(), ss->tasks_id.front());

            for (auto c : cur_node->get_children())
                update_tasks_id_recursive(c, already_parsed_nodes);
        }
    };

    std::vector<tools::Digraph_node<Sub_sequence>*> already_parsed_nodes;
    return update_tasks_id_recursive(this->sequences[tid], already_parsed_nodes);
}

std::vector<runtime::Task*>
Sequence::get_tasks_from_id(const size_t taid)
{
    std::function<void(tools::Digraph_node<Sub_sequence>*,
                       const size_t,
                       std::vector<runtime::Task*>&,
                       std::vector<tools::Digraph_node<Sub_sequence>*>&)>
      get_tasks_from_id_recursive =
        [&get_tasks_from_id_recursive](tools::Digraph_node<Sub_sequence>* cur_node,
                                       const size_t taid,
                                       std::vector<runtime::Task*>& tasks,
                                       std::vector<tools::Digraph_node<Sub_sequence>*>& already_parsed_nodes)
    {
        if (cur_node != nullptr &&
            std::find(already_parsed_nodes.begin(), already_parsed_nodes.end(), cur_node) == already_parsed_nodes.end())
        {
            already_parsed_nodes.push_back(cur_node);
            Sub_sequence* ss = cur_node->get_contents();
            bool found = false;
            for (size_t t = 0; t < ss->tasks_id.size(); t++)
                if (ss->tasks_id[t] == taid)
                {
                    tasks.push_back(ss->tasks[t]);
                    found = true;
                    break;
                }

            if (!found)
                for (auto c : cur_node->get_children())
                    get_tasks_from_id_recursive(c, taid, tasks, already_parsed_nodes);
        }
    };

    std::vector<runtime::Task*> tasks;
    for (auto& s : this->sequences)
    {
        std::vector<tools::Digraph_node<Sub_sequence>*> already_parsed_nodes;
        get_tasks_from_id_recursive(s, taid, tasks, already_parsed_nodes);
    }
    return tasks;
}

void
Sequence::update_firsts_and_lasts_tasks()
{
    this->firsts_tasks.clear();
    this->firsts_tasks.resize(this->n_threads);
    for (auto taid : firsts_tasks_id)
    {
        auto tasks = this->get_tasks_from_id(taid);
        for (size_t tid = 0; tid < tasks.size(); tid++)
            firsts_tasks[tid].push_back(tasks[tid]);
    }

    this->lasts_tasks.clear();
    this->lasts_tasks.resize(this->n_threads);
    for (auto taid : lasts_tasks_id)
    {
        auto tasks = this->get_tasks_from_id(taid);
        for (size_t tid = 0; tid < tasks.size(); tid++)
            lasts_tasks[tid].push_back(tasks[tid]);
    }
}

void
Sequence::_set_n_frames_unbind(std::vector<std::pair<runtime::Socket*, runtime::Socket*>>& unbind_sockets,
                               std::vector<std::pair<runtime::Task*, runtime::Socket*>>& unbind_tasks)
{
    std::function<void(const std::vector<runtime::Task*>&,
                       tools::Digraph_node<Sub_sequence>*,
                       std::vector<tools::Digraph_node<Sub_sequence>*>&)>
      graph_traversal = [&graph_traversal, &unbind_sockets, &unbind_tasks](
                          const std::vector<runtime::Task*>& possessed_tsks,
                          tools::Digraph_node<Sub_sequence>* cur_node,
                          std::vector<tools::Digraph_node<Sub_sequence>*>& already_parsed_nodes)
    {
        if (cur_node != nullptr &&
            std::find(already_parsed_nodes.begin(), already_parsed_nodes.end(), cur_node) == already_parsed_nodes.end())
        {
            already_parsed_nodes.push_back(cur_node);
            Sub_sequence* ss = cur_node->get_contents();
            for (auto tsk_out : ss->tasks)
            {
                for (auto sck_out : tsk_out->sockets)
                {
                    if (sck_out->get_type() == socket_t::SOUT || sck_out->get_type() == socket_t::SFWD)
                    {
                        auto sck_out_bound_sockets_cpy = sck_out->get_bound_sockets();
                        for (auto sck_in : sck_out_bound_sockets_cpy)
                        {
                            auto tsk_in = &sck_in->get_task();
                            // if the task of the current input socket is in the tasks of the sequence
                            if (std::find(possessed_tsks.begin(), possessed_tsks.end(), tsk_in) != possessed_tsks.end())
                            {
                                try
                                {
                                    tsk_in->unbind(*sck_out);
                                    unbind_tasks.push_back(std::make_pair(tsk_in, sck_out.get()));
                                }
                                catch (...)
                                {
                                    sck_in->unbind(*sck_out);
                                    // memorize the unbinds to rebind after!
                                    unbind_sockets.push_back(std::make_pair(sck_in, sck_out.get()));
                                }
                            }
                        }
                    }
                }
            }
            for (auto c : cur_node->get_children())
                graph_traversal(possessed_tsks, c, already_parsed_nodes);
        }
    };

    auto tsks_per_threads = this->get_tasks_per_threads();
    std::vector<tools::Digraph_node<Sub_sequence>*> already_parsed_nodes;
    for (size_t t = 0; t < this->get_n_threads(); t++)
    {
        already_parsed_nodes.clear();
        graph_traversal(tsks_per_threads[t], this->sequences[t], already_parsed_nodes);
    }
}

void
Sequence::_set_n_frames(const size_t n_frames)
{
    if (n_frames <= 0)
    {
        std::stringstream message;
        message << "'n_frames' has to be greater than 0 ('n_frames' = " << n_frames << ").";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    // set_n_frames for all the modules (be aware that this operation can fail)
    for (auto& mm : this->all_modules)
        for (auto& m : mm)
            m->set_n_frames(n_frames);
}

void
Sequence::_set_n_frames_rebind(const std::vector<std::pair<runtime::Socket*, runtime::Socket*>>& unbind_sockets,
                               const std::vector<std::pair<runtime::Task*, runtime::Socket*>>& unbind_tasks)
{
    // rebind the sockets
    for (auto& u : unbind_sockets)
        (*u.first) = *u.second;

    // rebind the tasks
    for (auto& u : unbind_tasks)
        (*u.first) = *u.second;
}

void
Sequence::set_n_frames(const size_t n_frames)
{
    const auto old_n_frames = this->get_n_frames();
    if (old_n_frames != n_frames)
    {
        std::vector<std::pair<runtime::Socket*, runtime::Socket*>> unbind_sockets;
        std::vector<std::pair<runtime::Task*, runtime::Socket*>> unbind_tasks;
        this->_set_n_frames_unbind(unbind_sockets, unbind_tasks);
        this->_set_n_frames(n_frames);
        this->_set_n_frames_rebind(unbind_sockets, unbind_tasks);
    }
}

bool
Sequence::is_control_flow() const
{
    return this->sequences[0]->get_children().size();
}

// /!\ this check has been commented because it is known to do not work in the general case
/*
template<class SS>
void Sequence
::check_ctrl_flow(tools::Digraph_node<SS>* root)
{
        std::function<void(tools::Digraph_node<SS>*,
                      std::vector<tools::Digraph_node<SS>*>&)> check_control_flow_parity =
                [&check_control_flow_parity](tools::Digraph_node<SS>* cur_node,
                                             std::vector<tools::Digraph_node<SS>*> already_parsed_nodes) -> void
                {
                        if (cur_node != nullptr &&
                            std::find(already_parsed_nodes.begin(),
                                      already_parsed_nodes.end(),
                                      cur_node) == already_parsed_nodes.end() &&
                                cur_node->get_children().size())
                        {
                                already_parsed_nodes.push_back(cur_node);
                                for (auto c : cur_node->get_children())
                                        check_control_flow_parity(c, already_parsed_nodes);
                        }
                        else
                        {
                                already_parsed_nodes.push_back(cur_node);
                                std::vector<module::Module*> parsed_switchers;
                                for (size_t i = 0; i < already_parsed_nodes.size(); i++)
                                {
                                        // This check occurs before dud-nodes are removed by _init, some nodes have no
contents and must be
                                        // accounted for
                                        if (already_parsed_nodes[i]->get_c() == nullptr ||
                                            !(already_parsed_nodes[i]->get_c()->type == subseq_t::COMMUTE ||
                                              already_parsed_nodes[i]->get_c()->type == subseq_t::SELECT))
                                                continue;

                                        // We search for the first switcher task in the path taken: already_parsed_nodes
                                        const runtime::Task *ctrl_task_first = nullptr;
                                        const runtime::Task *ctrl_task_second = nullptr;
                                        for (auto t : already_parsed_nodes[i]->get_c()->tasks)
                                        {
                                                if (dynamic_cast<const module::Switcher*>(&t->get_module()) &&
                                                    (t->get_name() == "select" || t->get_name() == "commute"))
                                                {
                                                        ctrl_task_first = t;
                                                        break;
                                                }
                                        }

                                        if (std::find(parsed_switchers.begin(), parsed_switchers.end(),
&(ctrl_task_first->get_module())) != parsed_switchers.end()) continue;

                                        // We now search for the second switcher task in the path taken
                                        auto expected_type = ctrl_task_first->get_name() == "select" ? subseq_t::COMMUTE
: subseq_t::SELECT; for (size_t j = i; j < already_parsed_nodes.size() && ctrl_task_second == nullptr; j++)
                                        {
                                                if (already_parsed_nodes[j]->get_c() == nullptr ||
                                                    already_parsed_nodes[j]->get_c()->type != expected_type)
                                                        continue;
                                                for (auto t : already_parsed_nodes[j]->get_c()->tasks)
                                                {
                                                        if ((t->get_name() == "select" || t->get_name() == "commute") &&
                                                            &(ctrl_task_first->get_module()) == &(t->get_module()))
                                                        {
                                                                parsed_switchers.push_back(&(t->get_module()));
                                                                ctrl_task_second = t;
                                                                break;
                                                        }
                                                }
                                        }

                                        if (ctrl_task_second == nullptr)
                                        {
                                                for (auto t : ctrl_task_first->get_module().tasks)
                                                {
                                                        if ((ctrl_task_first->get_name() == "select" && t->get_name() ==
"commute") || (ctrl_task_first->get_name() == "commute" && t->get_name() == "select"))
                                                        {
                                                                ctrl_task_second = t.get();
                                                                break;
                                                        }
                                                }
                                                std::stringstream message;
                                                message << ctrl_task_first->get_name() << " is missing a path to "
                                                        << ctrl_task_second->get_name() << ".";
                                                throw tools::control_flow_error(__FILE__, __LINE__, __func__,
message.str());
                                        }
                                }
                        }
                };

        std::vector<tools::Digraph_node<SS>*> already_parsed_nodes;
        return check_control_flow_parity(root, already_parsed_nodes);
}
*/
