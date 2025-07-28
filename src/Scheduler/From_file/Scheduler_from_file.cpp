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
  , final_pinning_policy_v2("")
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
    else if (file_version == 2)
    {
        this->contsruct_policy_v2(data, sequence);
    }
    else
    {
        std::stringstream message;
        message << "Unsupported file version: " << static_cast<int>(file_version) << ". Supported version are 1 or 2.";
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
std::vector<std::size_t>
get_pu_from_core(int id)
{
    std::vector<std::size_t> core_pus;
    hwloc_obj_t core_obj = hwloc_get_obj_by_type(topology, HWLOC_OBJ_CORE, id);

    if (core_obj == nullptr)
    {
        std::stringstream message;
        message << "No core with id " << id << " found in the topology.";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
    for (hwloc_obj_t child = core_obj->first_child; child != nullptr; child = child->next_sibling)
    {
        if (child->type == HWLOC_OBJ_PU)
        {
            core_pus.push_back(child->logical_index);
        }
    }
    return core_pus;
}
#endif
std::regex pu_single_regex(R"(^PU(\d+)$)");
std::regex pu_range_regex(R"(^PU(\d+)-(\d+)$)");
std::regex core_single_regex(R"(^core(\d+)$)");
std::regex core_range_regex(R"(^core(\d+)-(\d+)$)");

std::vector<std::vector<size_t>>
get_node_pus_from_node(const std::string& node_str)
{
    std::smatch match;
    std::vector<std::vector<size_t>> pu_vector;

    if (std::regex_match(node_str, match, pu_single_regex))
    {
        size_t pu_id = std::stoi(match[1].str());
        pu_vector.push_back({ pu_id });
    }
    else if (std::regex_match(node_str, match, pu_range_regex))
    {
        size_t pu_start = std::stoi(match[1].str());
        size_t pu_end = std::stoi(match[2].str());
        for (size_t pu_id = pu_start; pu_id <= pu_end; ++pu_id)
        {
            pu_vector.push_back({ pu_id });
        }
    }
    else if (std::regex_match(node_str, match, core_single_regex))
    {
#ifdef SPU_HWLOC
        size_t core_id = std::stoi(match[1].str());
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
        size_t core_start = std::stoi(match[1].str());
        size_t core_end = std::stoi(match[2].str());
        for (size_t core_id = core_start; core_id <= core_end; ++core_id)
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

void
Scheduler_from_file::build_stage_policy_packed(std::vector<std::vector<size_t>>& pu_list,
                                               size_t n_replicates,
                                               size_t st_index)
{
    size_t pu_index = 0;
    size_t pu_list_size = pu_list.size();
    this->puids_from_file.push_back({});
    for (size_t i = 0; i < n_replicates; i++)
    {
        // Check if the current PU list is empty
        if (pu_list.empty())
        {
            std::stringstream message;
            message << "Consumed the list of all avalable PUs during construction.";
            throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
        }
        this->puids_from_file[st_index].push_back(pu_list[pu_index][0]);

        pu_list[pu_index].erase(pu_list[pu_index].begin()); // Remove the first PU from the list
        if (pu_list[pu_index].empty())
        {
            pu_list.erase(pu_list.begin() + pu_index); // Remove the empty list
            pu_index--;
            pu_list_size--;
        }
        if (pu_list_size > 0) pu_index = (pu_index + 1) % pu_list_size; // Move to the next PU in the list
    }
}

void
Scheduler_from_file::build_stage_policy_loose(std::vector<std::vector<size_t>>& pu_list,
                                              size_t n_replicates,
                                              size_t st_index)
{
    size_t pu_list_size = pu_list.size();
    this->puids_from_file.push_back({});
    for (size_t j = 0; j < pu_list_size; j++)
        this->puids_from_file[st_index].push_back(pu_list[j][0]);
}

void
Scheduler_from_file::build_stage_policy_distant(std::vector<std::vector<size_t>>& pu_list,
                                                size_t n_replicates,
                                                size_t st_index,
                                                size_t curr_type_index,
                                                size_t smt_value)
{
    size_t pu_index = 0;
    size_t pu_list_size = pu_list.size();
    this->puids_from_file.push_back({});
    for (size_t i = 0; i < n_replicates; i++)
    {
        // Check if the current PU list is empty
        if (pu_list.empty())
        {
            std::stringstream message;
            message << "Consumed the list of all avalable PUs during construction.";
            throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
        }
        // Compute the index of the PU to use
        pu_index = ((curr_type_index + i) % 2) * (pu_list_size / (2 * smt_value));
        this->puids_from_file[st_index].push_back(pu_list[pu_index][0]);

        pu_list[pu_index].erase(pu_list[pu_index].begin()); // Remove the first PU from the list
        if (pu_list[pu_index].empty())
        {
            pu_list.erase(pu_list.begin() + pu_index); // Remove the empty list
            pu_index--;
            pu_list_size--;
        }
    }
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

    if (!p_core_ressources.contains("node-list"))
    {
        std::stringstream message;
        message << "p-cores list is not given in the json file.";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
    if (!e_core_ressources.contains("node-list"))
    {
        std::stringstream message;
        message << "e-cores list is not given in the json file.";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
    auto& p_core_list = p_core_ressources["node-list"];
    auto& e_core_list = e_core_ressources["node-list"];

    // Getting the smt value
    if (!p_core_ressources.contains("smt"))
    {
        std::stringstream message;
        message << "p-cores smt value is not given in the json file.";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
    size_t p_core_smt = p_core_ressources["smt"];

    if (!e_core_ressources.contains("smt"))
    {
        std::stringstream message;
        message << "e-cores smt value is not given in the json file.";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
    size_t e_core_smt = e_core_ressources["smt"];

    // Getting the list of PU of the given lists
#ifdef SPU_HWLOC
    hwloc_topology_init(&topology);
    hwloc_topology_load(topology);
#endif
    // Creating the p_core_pu_list
    for (auto& node : p_core_list)
    {
        std::vector<std::vector<size_t>> pu_vector;
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
    if (p_core_smt > 1)
    {
        for (size_t i = 0; i < p_core_smt - 1; i++)
        {
            for (size_t j = 0; j < this->p_core_pu_list.size(); j++)
            {
                if (this->p_core_pu_list[j].size() > 1)
                {
                    this->p_core_pu_list.push_back({ this->p_core_pu_list[j][1] });
                    this->p_core_pu_list[j].erase(this->p_core_pu_list[j].begin() + 1);
                }
            }
        }
    }
    // Creating the e_core_pu_list
    for (auto& node : e_core_list)
    {
        std::vector<std::vector<size_t>> pu_vector;
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
    if (e_core_smt > 1)
    {
        for (size_t i = 0; i < e_core_smt - 1; i++)
        {
            for (size_t j = 0; j < this->e_core_pu_list.size(); j++)
            {
                if (this->e_core_pu_list[j].size() > 1)
                {
                    this->e_core_pu_list.push_back({ this->e_core_pu_list[j][1] });
                    this->e_core_pu_list[j].erase(this->e_core_pu_list[j].begin() + 1);
                }
            }
        }
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
    // Getting information about wait type and buffer size
    this->sync_buff_sizes_from_file.resize(sched_data.size() - 1, 1);
    this->sync_active_waitings_from_file.resize(sched_data.size() - 1, false);

    // Starting the core allocation algorithme
    size_t n_tasks_json = 0;
    size_t curr_p_core_stage = 0;
    size_t curr_e_core_stage = 0;
    for (size_t d = 0; d < sched_data.size(); d++)
    {
        // Checking task entry
        if (!sched_data[d].contains("tasks"))
        {
            std::stringstream message;
            message << "The current entry does not contain the 'tasks' field.";
            throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
        }
        if (!sched_data[d]["tasks"].is_number_integer())
        {
            std::stringstream message;
            message << "Unexpected type for 'tasks' field (should be an integer).";
            throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
        }
        n_tasks_json += (size_t)sched_data[d]["tasks"];
        // Checking if the threads entry
        if (!sched_data[d].contains("threads"))
        {
            std::stringstream message;
            message << "The current entry does not contain the 'threads' field.";
            throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
        }
        if (!sched_data[d]["threads"].is_number_integer())
        {
            std::stringstream message;
            message << "Unexpected type for 'threads' field (should be an integer).";
            throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
        }
        size_t n_replicates = sched_data[d]["threads"];

        // Generating solution_from_file
        this->solution_from_file.push_back(std::make_pair((size_t)sched_data[d]["tasks"], n_replicates));

        // Getting the currect stage pinning strategy
        if (!sched_data[d].contains("pinning-strategy"))
        {
            std::stringstream message;
            message << "The current entry does not contain the 'pinning-strategy' field.";
            throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
        }
        if (!sched_data[d]["pinning-strategy"].is_string())
        {
            std::stringstream message;
            message << "Unexpected type for 'pinning-strategy' field (should be a string).";
            throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
        }
        this->pinning_strategy = sched_data[d]["pinning-strategy"];

        // Checking the type of cores to which the threads will be pinned
        if (!sched_data[d].contains("core-type"))
        {
            std::stringstream message;
            message << "The current entry does not contain the 'core-type' field.";
            throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
        }
        if (!sched_data[d]["core-type"].is_string())
        {
            std::stringstream message;
            message << "Unexpected type for 'core-type' field (should be a string).";
            throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
        }
        std::string core_type = sched_data[d]["core-type"];
        if (core_type == "p-core")
        {
            if (this->pinning_strategy == "packed")
            {
                build_stage_policy_packed(this->p_core_pu_list, n_replicates, d);
            }
            else if (this->pinning_strategy == "loose")
            {
                build_stage_policy_loose(this->p_core_pu_list, n_replicates, d);
            }
            else if (this->pinning_strategy == "distant")
            {
                build_stage_policy_distant(this->p_core_pu_list, n_replicates, d, curr_p_core_stage, p_core_smt);
            }
            else if (this->pinning_strategy != "no")
            {
                this->pinning_strategy = "no";
            }
            curr_p_core_stage++;
        }
        else if (core_type == "e-core")
        {
            if (this->pinning_strategy == "packed")
            {
                build_stage_policy_packed(this->e_core_pu_list, n_replicates, d);
            }
            else if (this->pinning_strategy == "loose")
            {
                build_stage_policy_loose(this->e_core_pu_list, n_replicates, d);
            }
            else if (this->pinning_strategy == "distant")
            {
                build_stage_policy_distant(this->e_core_pu_list, n_replicates, d, curr_e_core_stage, e_core_smt);
            }
            else if (this->pinning_strategy != "no")
            {
                this->pinning_strategy = "no";
            }
            curr_e_core_stage++;
        }
        else
        {
            std::stringstream message;
            message << "Unexpected value for 'core-type' field (should be 'p-core' or 'e-core', given = " << core_type
                    << ").";
            throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
        }
        // Checking if the current stage has inter-stage buffer size
        if (sched_data[d].contains("sync_buff_size"))
        {
            if (!sched_data[d]["sync_buff_size"].is_number_unsigned())
            {
                std::stringstream message;
                message << "Unexpected type for 'sync_buff_size' field (should be unsigned integer).";
                throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
            }
            else
            {
                this->sync_buff_sizes_from_file[d] = (size_t)sched_data[d]["sync_buff_size"];
            }
        }
        // Checking if the current stage has a synchronization type
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
    }
    // Checking if the number of tasks in the json file is equal to the number of tasks in the sequence
    if (n_tasks_json != sequence.get_tasks_per_threads()[0].size())
    {
        std::stringstream message;
        message << "The number of tasks in the json file differs from the number of tasks in the sequence "
                << "('n_tasks_json' = " << n_tasks_json
                << ", 'sequence.get_tasks_per_threads()[0].size()' = " << sequence.get_tasks_per_threads()[0].size()
                << ").";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
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
    if (this->pinning_strategy == "no") return "";

    std::string pinning_policy;
    if (this->puids_from_file.size())
    {
        if (this->file_version == 1 || (this->file_version == 2 && this->pinning_strategy == "packed") ||
            (this->file_version == 2 && this->pinning_strategy == "distant"))
        {
            for (size_t s = 0; s < this->puids_from_file.size(); s++)
            {
                if (s != 0) pinning_policy += " | ";
                for (size_t i = 0; i < this->puids_from_file[s].size(); i++)
                {
                    pinning_policy +=
                      std::string((i == 0) ? "" : "; ") + "PU_" + std::to_string(this->puids_from_file[s][i]);
                }

                if (!this->puids_from_file[s].size()) pinning_policy += "NO_PIN";
            }
        }
        else if (this->file_version == 2 && this->pinning_strategy == "loose")
        {
            for (size_t s = 0; s < this->puids_from_file.size(); s++)
            {
                if (s != 0) pinning_policy += " | ";
                for (size_t replicate = 0; this->solution_from_file[s].second > replicate; replicate++)
                {
                    if (replicate != 0) pinning_policy += " ; ";
                    for (size_t i = 0; i < this->puids_from_file[s].size(); i++)
                    {
                        pinning_policy +=
                          std::string((i == 0) ? "" : ", ") + "PU_" + std::to_string(this->puids_from_file[s][i]);
                    }
                }
                if (!this->puids_from_file[s].size()) pinning_policy += "NO_PIN";
            }
        }
        else
        {
            std::stringstream message;
            message << "Unknown pinning strategy: " << this->pinning_strategy;
            throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
        }
    }
    else
        pinning_policy = Scheduler::get_threads_mapping();

    return pinning_policy;
}
