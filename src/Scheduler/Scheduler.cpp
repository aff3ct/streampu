#include "Scheduler/Scheduler.hpp"
#include "Tools/Exception/exception.hpp"

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
        stream << " - Name: " << t.tptr->get_name();
        stream << " - Execution duration: " << t.exec_duration.count();
        stream << " - Stateful: " << t.tptr->is_stateful() << std::endl;
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

runtime::Pipeline*
Scheduler::instantiate_pipeline()
{
    if (this->solution.size() == 0)
    {
        std::stringstream message;
        message << "The solution has to contain at least one element.";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    std::vector<runtime::Task*> firsts;
    std::vector<runtime::Task*> lasts;
    std::vector<std::pair<std::vector<runtime::Task*>, std::vector<runtime::Task*>>> sep_stages;
    std::vector<size_t> n_threads;
    std::vector<size_t> synchro_buffer_sizes;
    std::vector<bool> synchro_active_waiting;
    std::vector<bool> thread_pining;
    std::vector<std::vector<size_t>> puids;

    firsts.push_back((this->tasks_desc[0]).tptr);

    int N = this->tasks_desc.size();
    lasts.push_back((this->tasks_desc[N - 1]).tptr);

    size_t buffer_size = 1000;
    bool active_wait = false;
    bool thread_pin = true;
    int begin = 0; // beginning of the current stage
    int end = 0;   // end of the current stage
    int puid_counter = 0;
    for (auto& stage : this->solution)
    {
        end = begin + stage.first - 1;
        std::pair<std::vector<runtime::Task*>, std::vector<runtime::Task*>> cur_stage_desc;
        cur_stage_desc.first.push_back(this->tasks_desc[begin].tptr);
        cur_stage_desc.second.push_back(this->tasks_desc[end].tptr);
        sep_stages.push_back(cur_stage_desc);
        begin = end + 1;

        n_threads.push_back(stage.second);
        thread_pining.push_back(thread_pin);

        if (this->solution.size() != 1)
        {
            synchro_buffer_sizes.push_back(buffer_size);
            synchro_active_waiting.push_back(active_wait);
        }

        std::vector<size_t> cur_puids;
        for (size_t c = puid_counter; c < (size_t)(puid_counter + stage.second); c++)
        {
            cur_puids.push_back(c + stage.second);
        }
        puids.push_back(cur_puids);
    }

    // remove the last buffer size and active waiting (no need for the last stage)
    if (this->solution.size() != 1)
    {
        synchro_buffer_sizes.pop_back();
        synchro_active_waiting.pop_back();
    }

    return new runtime::Pipeline(
      firsts, lasts, sep_stages, n_threads, synchro_buffer_sizes, synchro_active_waiting, thread_pining, puids);
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

    return this->instantiate_pipeline();
}
