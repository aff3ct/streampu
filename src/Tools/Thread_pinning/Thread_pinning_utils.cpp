#include <iostream>
#include <sstream>

#include "Tools/Exception/exception.hpp"
#include <Tools/Thread_pinning/Thread_pinning_utils.hpp>

using namespace aff3ct;
using namespace aff3ct::tools;

#ifdef AFF3CT_CORE_HWLOC
static std::map<std::string, hwloc_obj_type_t> object_map = {
    { "NUMA", HWLOC_OBJ_NUMANODE }, { "PACKAGE", HWLOC_OBJ_PACKAGE }, { "CORE", HWLOC_OBJ_CORE },
    { "PU", HWLOC_OBJ_PU },         { "L1D", HWLOC_OBJ_L1CACHE },     { "L2D", HWLOC_OBJ_L2CACHE },
    { "L3D", HWLOC_OBJ_L3CACHE },   { "L4D", HWLOC_OBJ_L4CACHE },     { "L5D", HWLOC_OBJ_L5CACHE },
    { "L1I", HWLOC_OBJ_L1ICACHE },  { "L2I", HWLOC_OBJ_L2ICACHE },    { "L3I", HWLOC_OBJ_L3ICACHE },
    { "GROUP", HWLOC_OBJ_GROUP },
};
#endif

std::vector<std::string>
Thread_pinning_utils::pipeline_parser_unpacker(std::string const& hwloc_objects_pipeline, const size_t number_of_stages)
{
    std::vector<std::string> vector_stages = {};
    std::string tmp;
    size_t i = 0;

    // Parsing part
    while (i < hwloc_objects_pipeline.size())
    {
        if (hwloc_objects_pipeline[i] == '|')
        {
            vector_stages.push_back(tmp);
            tmp.clear();
        }
        else if (hwloc_objects_pipeline[i] != ' ')
        {
            tmp.push_back(hwloc_objects_pipeline[i]);
        }
        i++;
    }
    vector_stages.push_back(tmp);

    // Unpacking part : pipeline=>stages
    if (vector_stages.size() == 1 && number_of_stages > 1)
    {
        for (size_t j = 0; j < number_of_stages - 1; ++j)
            vector_stages.push_back(vector_stages[0]);
    }
    else if (vector_stages.size() != number_of_stages)
    {
        std::stringstream message;
        message << "The number of objects is not equal to the number of stages ('vector_stages.size()' = "
                << vector_stages.size() << " and 'number_of_stages' = " << number_of_stages << ").";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    return vector_stages;
}

std::vector<std::string>
Thread_pinning_utils::stage_parser_unpacker(std::string const& hwloc_objects_stage, const size_t number_of_threads)
{
    std::vector<std::string> vector_threads = {};
    std::string tmp;
    size_t i = 0;
    while (i < hwloc_objects_stage.size())
    {
        if (hwloc_objects_stage[i] == ';')
        {
            vector_threads.push_back(tmp);
            tmp.clear();
        }
        else if (hwloc_objects_stage[i] != ' ')
        {
            tmp.push_back(hwloc_objects_stage[i]);
        }
        i++;
    }
    vector_threads.push_back(tmp); // Last thread push

    // Unpacking part : stage=>threads
    if (vector_threads.size() == 1 && number_of_threads > 1)
    {
        for (size_t j = 0; j < number_of_threads - 1; ++j)
            vector_threads.push_back(vector_threads[0]);
    }
    else if (vector_threads.size() != number_of_threads)
    {
        std::stringstream message;
        message << "The number of objects is not equal to the number of threads ('vector_threads.size()' = "
                << vector_threads.size() << " and 'number_of_threads' = " << number_of_threads << ").";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
    return vector_threads;
}

std::vector<std::string>
Thread_pinning_utils::thread_parser(std::string const& hwloc_objects_thread)
{
    std::vector<std::string> vector_objets = {};
    std::string tmp;
    size_t i = 0;

    while (i < hwloc_objects_thread.size())
    {
        if (hwloc_objects_thread[i] == ',')
        {
            vector_objets.push_back(tmp);
            tmp.clear();
        }
        else if (hwloc_objects_thread[i] != ' ')
        {
            tmp.push_back(hwloc_objects_thread[i]);
        }
        i++;
    }
    vector_objets.push_back(tmp); // Last thread push
    return vector_objets;
}

#ifdef AFF3CT_CORE_HWLOC
std::pair<hwloc_obj_type_t, int>
Thread_pinning_utils::str_to_hwloc_object(std::string& str_object)
{
    std::pair<hwloc_obj_type_t, int> result;
    std::stringstream ss(str_object);
    char delim = '_';
    std::string item;
    std::getline(ss, item, delim);
    if (object_map.find(item) == object_map.end())
    {
        std::stringstream msg;
        msg << "'";
        msg << item;
        msg << "' is not a valid hwloc object.";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, msg.str());
    }
    result.first = object_map[item];
    std::getline(ss, item, delim);
    result.second = std::stoi(item);
    return result;
}
#endif
