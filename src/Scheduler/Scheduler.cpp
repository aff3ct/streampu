#include "Scheduler/Scheduler.hpp"
#include "Tools/Exception/exception.hpp"
#include "Tools/Thread/Thread_pinning/Thread_pinning.hpp"

#include <iostream>
#include <sstream>

using namespace spu;
using namespace spu::sched;

Scheduler::Scheduler(runtime::Sequence& sequence)
  : sequence(&sequence)
{
    this->sequence = &sequence;
}

Scheduler::Scheduler(runtime::Sequence* sequence)
  : sequence(sequence)
{
    if (sequence == nullptr)
    {
        std::stringstream message;
        message << "'sequence' can't be nullptr.";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    this->sequence = sequence;
}

void
Scheduler::profile(const size_t n_exec)
{
    if (n_exec == 0)
    {
        std::stringstream message;
        message << "'n_exec' has to be higher than zero.";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    if (sequence->get_n_threads() > 1)
    {
        std::stringstream message;
        message << "'sequence->get_n_threads()' has to be strictly equal to 1.";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    if (this->sequence->is_control_flow())
    {
        std::stringstream message;
        message << "Control flow in the sequence is not supported yet.";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    if (!this->tasks_desc.empty())
    {
        std::stringstream message;
        message << "'tasks_desc' should be empty, you should call 'Scheduler::reset' first if you want to re-run the "
                   "profiling'.";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    this->sequence->set_auto_stop(false);
    for (auto& mod : this->sequence->get_modules<module::Module>(false))
        for (auto& tsk : mod->tasks)
        {
            tsk->reset();
            tsk->set_stats(true); // enable the statistics
            tsk->set_fast(true);  // enable the fast mode (= disable the useless verifs
                                  // in the tasks)
        }
    unsigned int counter = 0;
    this->sequence->exec([&counter, &n_exec]() { return ++counter >= n_exec; });
    this->sequence->set_auto_stop(true);

    std::vector<runtime::Task*>& tasks = this->sequence->sequences[0]->get_contents()->tasks;
    for (auto& t : tasks)
    {
        task_desc_t new_t;
        new_t.tptr = t;
        new_t.exec_duration = t->get_duration_avg();
        this->tasks_desc.push_back(new_t);
    }
}

void
Scheduler::print_profiling(std::ostream& stream)
{
    stream << "# Profiling:" << std::endl;
    for (auto& t : this->tasks_desc)
    {
        stream << "# - Name: " << t.tptr->get_name();
        stream << " - Execution duration: " << t.exec_duration.count() << " ns";
        stream << " - Replicable: " << (t.tptr->is_replicable() ? "yes" : "no") << std::endl;
    }
}

const std::vector<task_desc_t>&
Scheduler::get_profiling()
{
    return this->tasks_desc;
}

void
Scheduler::reset()
{
    this->solution.clear();
    this->tasks_desc.clear();
}

std::string
Scheduler::perform_threads_mapping() const
{
    if (this->solution.size() == 0)
    {
        std::stringstream message;
        message
          << "The solution has to contain at least one element, please run the 'Scheduler::schedule' method first.";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    std::string pinning_policy;
    bool first_stage = true;
    size_t puid = 0;
    for (auto& stage : this->solution)
    {
        if (!first_stage) pinning_policy += " | ";

        for (size_t st = 0; st < stage.second; st++)
            pinning_policy += std::string((st == 0) ? "" : "; ") + "PU_" + std::to_string(puid++);

        first_stage = false;
    }

    return pinning_policy;
}

runtime::Pipeline*
Scheduler::instantiate_pipeline(const size_t buffer_size,
                                const bool active_waiting,
                                const bool thread_pining,
                                const std::string& pinning_policy)
{
    if (this->solution.size() == 0)
    {
        std::stringstream message;
        message
          << "The solution has to contain at least one element, please run the 'Scheduler::schedule' method first.";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    std::vector<runtime::Task*> firsts(this->tasks_desc.size());
    std::vector<runtime::Task*> lasts(this->tasks_desc.size());

    std::vector<std::pair<std::vector<runtime::Task*>, std::vector<runtime::Task*>>> sep_stages(this->solution.size());
    std::vector<size_t> n_threads(this->solution.size());
    size_t s = 0;
    size_t i = 0;
    for (auto& stage : this->solution)
    {
        std::pair<std::vector<runtime::Task*>, std::vector<runtime::Task*>> cur_stage_desc;
        sep_stages[s].first.resize(stage.first);
        sep_stages[s].second.resize(stage.first);
        for (size_t t = 0; t < stage.first; t++)
        {
            firsts[i] = this->tasks_desc[i].tptr;
            lasts[i] = this->tasks_desc[i].tptr;

            sep_stages[s].first[t] = this->tasks_desc[i].tptr;
            sep_stages[s].second[t] = this->tasks_desc[i].tptr;

            i++;
        }

        n_threads[s] = stage.second;
        s++;
    }

    std::vector<size_t> synchro_buffer_sizes(this->solution.size() - 1, buffer_size);
    std::vector<bool> synchro_active_waitings(this->solution.size() - 1, active_waiting);
    std::vector<bool> thread_pinings(this->solution.size(), thread_pining);

    return new runtime::Pipeline(firsts,
                                 lasts,
                                 sep_stages,
                                 n_threads,
                                 synchro_buffer_sizes,
                                 synchro_active_waitings,
                                 thread_pinings,
                                 pinning_policy);
}

std::vector<std::pair<size_t, size_t>>
Scheduler::get_solution()
{
    return this->solution;
}

runtime::Pipeline*
Scheduler::generate_pipeline()
{
    if (tasks_desc.empty()) this->profile();

    if (solution.empty()) this->schedule();

    return this->instantiate_pipeline(1, false, tools::Thread_pinning::is_init(), this->perform_threads_mapping());
}

size_t
Scheduler::get_n_alloc_ressources() const
{
    if (this->solution.size() == 0)
    {
        std::stringstream message;
        message
          << "The solution has to contain at least one element, please run the 'Scheduler::schedule' method first.";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }
    size_t R = 0;
    for (auto s : this->solution)
        R += s.second;
    return R;
}

double
Scheduler::get_throughput_est() const
{
    throw tools::unimplemented_error(__FILE__, __LINE__, __func__);
}
