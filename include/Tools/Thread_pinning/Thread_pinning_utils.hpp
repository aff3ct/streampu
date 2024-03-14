#ifndef THREAD_PINNING_UTILS_HPP
#define THREAD_PINNING_UTILS_HPP

#ifdef AFF3CT_CORE_HWLOC
#include <hwloc.h>
#include <utility>
#endif
#include <map>
#include <regex>
#include <string>
#include <vector>

namespace aff3ct
{
namespace tools
{
class Thread_pinning_utils
{
public:
	static std::vector<std::string> pipeline_parser_unpacker(std::string const &hwloc_objects_pipeline,
	                                                         const size_t number_of_stages);

	static std::vector<std::string> stage_parser_unpacker(std::string const &hwloc_objects_stage,
	                                                      const size_t number_of_threads);

	static std::vector<std::string> thread_parser(std::string const &hwloc_objects_thread);

#ifdef AFF3CT_CORE_HWLOC
	static std::pair<hwloc_obj_type_t, int> str_to_hwloc_object(std::string &str_object);
#endif
};
}
}

#endif