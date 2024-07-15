#include "Tools/Scheduler/Scheduler.hpp"
#include "Tools/Display/Statistics/Statistics.hpp"
#include <cassert>

using namespace spu;
using namespace spu::tools;

Scheduler::Scheduler(runtime::Sequence& sequence)
  : sequence(&sequence)
{
    // assert(&sequence != nullptr);
    this->sequence = &sequence;
}

Scheduler::Scheduler(runtime::Sequence* sequence)
  : sequence(sequence)
{
    assert(sequence != nullptr);
    this->sequence = sequence;
}

void
Scheduler::profile()
{
    assert(sequence->get_n_threads() == 1);
    assert(!this->sequence->is_control_flow());

    this->tasks_desc.clear();

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
    unsigned int n_exec = 10;
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
Scheduler::print_profiling()
{
    std::cout << "# Profiling:" << std::endl;
    for (auto& t : this->tasks_desc)
    {
        std::cout << " - Name: " << t.tptr->get_name();
        std::cout << " - Execution duration: " << t.exec_duration.count();
        std::cout << " - Stateful: " << t.tptr->is_stateful() << std::endl;
    }
}

void
Scheduler::reset()
{
}