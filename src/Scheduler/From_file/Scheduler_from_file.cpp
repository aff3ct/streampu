#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <regex>
#include <sstream>

#include "Scheduler/From_file/Scheduler_from_file.hpp"
#include "Tools/Exception/exception.hpp"
#include "Tools/Thread/Thread_pinning/Thread_pinning_utils.hpp"

using json = nlohmann::json;

using namespace spu;
using namespace spu::sched;

Scheduler_from_file::Scheduler_from_file(runtime::Sequence& sequence, const std::string filename, uint8_t file_version)
  : Scheduler(sequence)
  , file_version(file_version)
{
    std::ifstream f(filename);
    if (!f.good())
    {
        std::stringstream message;
        message << "The current file cannot be opened ('filename' = " << filename << ").";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    json data = json::parse(f);

    if (file_version == 1)
    {
        this->contsruct_policy_v1(data, sequence);
    }
    else
    {
        std::stringstream message;
        message << "Unsupported file version: " << static_cast<int>(file_version) << ". Supported version is 1.";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
}

Scheduler_from_file::Scheduler_from_file(runtime::Sequence* sequence, const std::string filename, uint8_t file_version)
  : Scheduler_from_file(*sequence, filename, file_version)
{
}

// Construct policy function for V1 file
void
Scheduler_from_file::contsruct_policy_v1(json& data, runtime::Sequence& sequence)
{

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

/*######################################### JSON SECOND VERSION ########################################################
#######################################################################################################################*/
#ifdef SPU_HWLOC
hwloc_topology_t topology;
std::vector<std::string>
get_pu_from_core(int id)
{
    std::vector<std::string> core_pus;
    hwloc_obj_t core_obj = hwloc_get_obj_by_type(topology, HWLOC_OBJ_CORE, id);

    if (core_obj == nullptr)
    {
        std::stringstream message;
        message << "No core with id " << id << " found in the topology.";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
    std::vector<std::string> pu_vector;
    for (hwloc_obj_t child = core_obj->first_child; child != nullptr; child = child->next_sibling)
    {
        if (child->type == HWLOC_OBJ_PU)
        {
            core_pus.push_back("PU_" + std::to_string(child->gp_index));
        }
    }
    return core_pus;
}
#endif
std::regex pu_single_regex(R"(^PU(\d+)$)");
std::regex pu_range_regex(R"(^PU(\d+)-(\d+)$)");
std::regex core_single_regex(R"(^CORE(\d+)$)");
std::regex core_range_regex(R"(^CORE(\d+)-(\d+)$)");

std::vector<std::vector<std::string>>
get_node_pus_from_node(const std::string& node_str)
{
    std::smatch match;
    std::vector<std::vector<std::string>> pu_vector;

    if (std::regex_match(node_str, match, pu_single_regex))
    {
        int pu_id = std::stoi(match[1].str());
        pu_vector.push_back({ "PU_" + std::to_string(pu_id) });
    }
    else if (std::regex_match(node_str, match, pu_range_regex))
    {
        int pu_start = std::stoi(match[1].str());
        int pu_end = std::stoi(match[2].str());
        for (int pu_id = pu_start; pu_id <= pu_end; ++pu_id)
        {
            pu_vector.push_back({ "PU_" + std::to_string(pu_id) });
        }
    }
    else if (std::regex_match(node_str, match, core_single_regex))
    {
#ifdef SPU_HWLOC
        int core_id = std::stoi(match[1].str());
        pu_vector.push_back(get_pu_from_core(core_id));
#else
        std::stringstream message;
        message << "Using the V2 json format pinning to cores feature"
                << "requires linking with the 'hwloc' library.";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
#endif
    }
    else if (std::regex_match(node_str, match, core_range_regex))
    {
#ifdef SPU_HWLOC
        int core_start = std::stoi(match[1].str());
        int core_end = std::stoi(match[2].str());
        for (int core_id = core_start; core_id <= core_end; ++core_id)
            pu_vector.push_back(get_pu_from_core(core_id));
#else
        std::stringstream message;
        message << "Using the V2 json format pinning to cores feature"
                << "requires linking with the 'hwloc' library.";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
#endif
    }
    else
    {
        std::stringstream message;
        message << "Invalid node string format: " << node_str;
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
    return pu_vector;
}

//! TO DO : define the function
std::string
build_stage_policy(std::vector<std::vector<std::string>>& pu_list, size_t n_replicates)
{
}

void
Scheduler_from_file::contsruct_policy_v2(json& data, runtime::Sequence& sequence)
{
    if (!data.contains("resources"))
    {
        std::stringstream message;
        message << "No ressources information in the json file.";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
    auto& resources = data["resources"];

    if (!resources.contains("p-core"))
    {
        std::stringstream message;
        message << "p-cores informations are not specified in the json file.";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    if (!resources.contains("e-core"))
    {
        std::stringstream message;
        message << "e-cores informations are not specified in the json file.";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    auto& p_core_ressources = resources["p-core"];
    auto& e_core_ressources = resources["e-core"];

    if (!p_core_ressources.contains("nodes-list"))
    {
        std::stringstream message;
        message << "p-cores list is not given in the json file.";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
    if (!e_core_ressources.contains("nodes-list"))
    {
        std::stringstream message;
        message << "e-cores list is not given in the json file.";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
    auto& p_core_list = p_core_ressources["nodes-list"];
    auto& e_core_list = e_core_ressources["nodes-list"];

    // Getting the list of PU of the given lists
#ifdef SPU_HWLOC
    hwloc_topology_init(&topology);
    hwloc_topology_load(topology);
#endif

    // Creating the p_core_pu_list
    for (auto& node : p_core_list)
    {
        std::vector<std::vector<std::string>> pu_vector;
        if (node.is_string())
        {
            pu_vector = get_node_pus_from_node(node);
        }
        else if (node.is_number_integer())
        {
            pu_vector = get_node_pus_from_node("PU" + std::to_string((int)node));
        }
        else
        {
            std::stringstream message;
            message << "Invalid type for p-core node: " << node.dump();
            throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
        }
        this->p_core_pu_list.insert(this->p_core_pu_list.end(), pu_vector.begin(), pu_vector.end());
    }
    // Creating the e_core_pu_list
    for (auto& node : e_core_list)
    {
        std::vector<std::vector<std::string>> pu_vector;
        if (node.is_string())
        {
            pu_vector = get_node_pus_from_node(node);
        }
        else if (node.is_number_integer())
        {
            pu_vector = get_node_pus_from_node("PU" + std::to_string((int)node));
        }
        else
        {
            std::stringstream message;
            message << "Invalid type for p-core node: " << node.dump();
            throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
        }
        this->e_core_pu_list.insert(this->e_core_pu_list.end(), pu_vector.begin(), pu_vector.end());
    }

    // Generating the solution_from_file
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

    // Starting the core allocation algorithme
    size_t n_tasks_json = 0;
    std::string pinning_policy = "";
    for (auto& stage : sched_data)
    {
        // Adding the | for stage separation
        if (!pinning_policy.empty()) pinning_policy += " | ";

        // Checking task entry
        if (!stage.contains("tasks"))
        {
            std::stringstream message;
            message << "The current entry does not contain the 'tasks' field.";
            throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
        }
        if (!stage["tasks"].is_number_integer())
        {
            std::stringstream message;
            message << "Unexpected type for 'tasks' field (should be an integer).";
            throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
        }
        size_t n_tasks = stage["tasks"];
        // Checking if the threads entry
        if (!stage.contains("threads"))
        {
            std::stringstream message;
            message << "The current entry does not contain the 'threads' field.";
            throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
        }
        if (!stage["threads"].is_number_integer())
        {
            std::stringstream message;
            message << "Unexpected type for 'threads' field (should be an integer).";
            throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
        }
        size_t n_replicates = stage["threads"];
        // Checking the type of cores to which the threads will be pinned
        if (!stage.contains("core-type"))
        {
            std::stringstream message;
            message << "The current entry does not contain the 'core-type' field.";
            throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
        }
        if (!stage["core-type"].is_string())
        {
            std::stringstream message;
            message << "Unexpected type for 'core-type' field (should be a string).";
            throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
        }
        std::string core_type = stage["core-type"];
        if (core_type == "p-core")
        {
            pinning_policy += build_stage_policy(this->p_core_pu_list, n_replicates);
        }
        else if (core_type == "e-core")
        {
            pinning_policy += build_stage_policy(this->e_core_pu_list, n_replicates);
        }
        else
        {
            std::stringstream message;
            message << "Unexpected value for 'core-type' field (should be 'p-core' or 'e-core', given = " << core_type
                    << ").";
            throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
        }
    }
}
/*######################################################################################################################
#######################################################################################################################*/

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
