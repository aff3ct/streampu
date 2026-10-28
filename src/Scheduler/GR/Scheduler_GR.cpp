#include <cmath>
#include <iostream>
#include <limits>
#include <sstream>

#include "Scheduler/GR/Scheduler_GR.hpp"
#include "Tools/Exception/exception.hpp"

using namespace spu;
using namespace spu::sched;

Scheduler_GR::Scheduler_GR(runtime::Sequence& sequence, const size_t R)
  : Scheduler(&sequence)
  , R(R)
{
}

Scheduler_GR::Scheduler_GR(runtime::Sequence* sequence, const size_t R)
  : Scheduler(sequence)
  , R(R)
{
}

void
Scheduler_GR::schedule()
{
    if (this->tasks_desc.empty())
    {
        std::stringstream message;
        message << "'tasks_desc' cannot be empty, you need to execute the 'Scheduler::profile()' method first!";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    if (!this->solution.empty()) this->solution.clear();
    for (size_t t = 0; t < this->tasks_desc.size(); t++)
    {
        size_t cur_R = 1;
        if (tasks_desc[t].tptr->is_replicable()) cur_R = this->R;
        this->solution.push_back({ 1, cur_R });
    }
}
