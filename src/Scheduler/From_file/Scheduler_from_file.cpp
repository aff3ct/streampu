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
    if (!f.good())
    {
        std::stringstream message;
        message << "The current file cannot be opened ('filename' = " << filename << ").";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    json data = json::parse(f);

    if (!data.contains("schedule"))
    {
        std::stringstream message;
        message << "The current json file does not contain the required 'schedule' field.";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    auto sched_data = data["schedule"];

    if (!sched_data.is_array())
    {
        std::stringstream message;
        message << "Unexpected type for the 'schedule' field (should be an array).";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    this->sync_buff_sizes_from_file.resize(sched_data.size() - 1, 1);
    this->sync_active_waitings_from_file.resize(sched_data.size() - 1, false);

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

        if (sched_data[d].contains("sync_buff_size"))
        {
            if (d == sched_data.size() - 1)
            {
                std::stringstream message;
                message << "The 'sync_buff_size' field cannot be set on the last stage.";
                throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
            }
            if (!sched_data[d]["sync_buff_size"].is_number_unsigned())
            {
                std::stringstream message;
                message << "Unexpected type for 'sync_buff_size' field (should be unsigned integer).";
                throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
            }

            size_t cur_buff_size(sched_data[d]["sync_buff_size"]);
            this->sync_buff_sizes_from_file[d] = cur_buff_size;
        }

        if (sched_data[d].contains("sync_waiting_type"))
        {
            if (d == sched_data.size() - 1)
            {
                std::stringstream message;
                message << "The 'sync_waiting_type' field cannot be set on the last stage.";
                throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
            }
            if (!sched_data[d]["sync_waiting_type"].is_string())
            {
                std::stringstream message;
                message << "Unexpected type for 'sync_waiting_type' field (should be a string).";
                throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
            }

            std::string cur_sync_waiting_type(sched_data[d]["sync_waiting_type"]);
            if (cur_sync_waiting_type != "active" && cur_sync_waiting_type != "passive")
            {
                std::stringstream message;
                message << "Unexpected value for 'sync_waiting_type' field (should be a 'active' or 'passive', given = "
                        << cur_sync_waiting_type << ").";
                throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
            }

            if (cur_sync_waiting_type == "active") this->sync_active_waitings_from_file[d] = true;
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

std::vector<size_t>
Scheduler_from_file::get_sync_buff_sizes() const
{
    return this->sync_buff_sizes_from_file;
}

std::vector<bool>
Scheduler_from_file::get_sync_active_waitings() const
{
    return this->sync_active_waitings_from_file;
}

std::vector<bool>
Scheduler_from_file::get_thread_pinnings() const
{
    std::vector<bool> threads_pin(this->solution_from_file.size(), false);
    for (size_t s = 0; s < this->puids_from_file.size(); s++)
        if (this->puids_from_file[s].size()) threads_pin[s] = true;
    return threads_pin;
}

std::string
Scheduler_from_file::get_threads_mapping() const
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
        pinning_policy = Scheduler::get_threads_mapping();

    return pinning_policy;
}
