#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <nlohmann/json.hpp>
#include <sstream>
using json = nlohmann::json;

#include "Scheduler/From_file/Scheduler_from_file.hpp"
#include "Tools/Exception/exception.hpp"

using namespace spu;
using namespace spu::sched;

Scheduler_from_file::Scheduler_from_file(runtime::Sequence& sequence, const std::string filename)
  : Scheduler(sequence)
{
    std::ifstream f(filename);
    json sched_data = json::parse(f);

    size_t n_tasks_json = 0;
    for (size_t d = 0; d < sched_data.size(); d++)
    {
        if (!sched_data[d].contains("cores"))
        {
            std::stringstream message;
            message << "The current entry does not contain the 'cores' field.";
            throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
        }
        if (!sched_data[d].contains("tasks"))
        {
            std::stringstream message;
            message << "The current entry does not contain the 'tasks' field.";
            throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
        }

        size_t r;
        if (sched_data[d]["cores"].is_array())
        {
            if (!this->puids_from_file.size()) this->puids_from_file.resize(sched_data.size());

            this->puids_from_file[d].resize(sched_data[d]["cores"].size());
            std::copy(sched_data[d]["cores"].begin(), sched_data[d]["cores"].end(), this->puids_from_file[d].begin());
            r = sched_data[d]["cores"].size();
        }
        else if (sched_data[d]["cores"].is_number_unsigned())
            r = sched_data[d]["cores"];
        else
        {
            std::stringstream message;
            message << "Unexpected type for 'cores' field (should be unsigned integer or array of unsigned integers).";
            throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
        }

        if (sched_data[d]["tasks"].is_number_unsigned())
            this->solution_from_file.push_back(std::make_pair(sched_data[d]["tasks"], r));
        else
        {
            std::stringstream message;
            message << "Unexpected type for 'tasks' field (should be unsigned integer).";
            throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
        }

        size_t inc(sched_data[d]["tasks"]);
        n_tasks_json += inc;
    }

    if (n_tasks_json != sequence.get_tasks_per_threads()[0].size())
    {
        std::stringstream message;
        message << "The number of tasks in the json file differs from the number of tasks in the sequence "
                << "('n_tasks_json' = " << n_tasks_json
                << ", 'sequence.get_tasks_per_threads()[0].size()' = " << sequence.get_tasks_per_threads()[0].size()
                << ".";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
}

Scheduler_from_file::Scheduler_from_file(runtime::Sequence* sequence, const std::string filename)
  : Scheduler_from_file(*sequence, filename)
{
}

void
Scheduler_from_file::schedule()
{
    this->solution = this->solution_from_file;
}

std::string
Scheduler_from_file::perform_threads_mapping() const
{
    std::string pinning_policy;
    if (this->puids_from_file.size())
    {
        for (size_t s = 0; s < this->puids_from_file.size(); s++)
        {
            if (s != 0) pinning_policy += " | ";
            for (size_t i = 0; i < this->puids_from_file[s].size(); i++)
                pinning_policy +=
                  std::string((i == 0) ? "" : "; ") + "PU_" + std::to_string(this->puids_from_file[s][i]);
            if (!this->puids_from_file[s].size()) pinning_policy += "NO_PIN";
        }
    }
    else
        pinning_policy = Scheduler::perform_threads_mapping();

    return pinning_policy;
}

std::vector<bool>
Scheduler_from_file::get_threads_pinning() const
{
    std::vector<bool> threads_pin(this->solution_from_file.size(), false);
    for (size_t s = 0; s < this->puids_from_file.size(); s++)
        if (this->puids_from_file[s].size()) threads_pin[s] = true;
    return threads_pin;
}
