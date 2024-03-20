#include <functional>
#include <iostream>
#include <sstream>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <vector>
#include <mutex>
#ifdef AFF3CT_CORE_HWLOC
#include <hwloc.h>
#endif

#include "Tools/Exception/exception.hpp"
#include "Tools/Thread_pinning/Thread_pinning.hpp"
#include "Tools/Thread_pinning/Thread_pinning_utils.hpp"

using namespace aff3ct;
using namespace aff3ct::tools;

#ifdef AFF3CT_CORE_HWLOC
static hwloc_topology_t g_topology;
static int g_topodepth = 0;
#endif
static bool g_is_init = false;
static std::mutex g_mtx;
static bool g_enable_logs = false;

void Thread_pinning
::init()
{
	if (!g_is_init)
	{
		g_mtx.lock();
		if (!g_is_init)
		{
			g_is_init = true;

#ifdef AFF3CT_CORE_HWLOC
			/* Allocate and initialize topology object. */
			hwloc_topology_init(&g_topology);

			/* ... Optionally, put detection configuration here to ignore
			 some objects types, define a synthetic topology, etc....
			 The default is to detect all the objects of the machine that
			 the caller is allowed to access.  See Configure Topology
			 Detection. */

			/* Perform the topology detection. */
			hwloc_topology_load(g_topology);

			/* Optionally, get some additional topology information
			 in case we need the topology depth later. */
			g_topodepth = hwloc_topology_get_depth(g_topology);
#endif
		}
		g_mtx.unlock();
	}
}

void Thread_pinning
::destroy()
{
	if (g_is_init)
	{
		g_mtx.lock();
		if (g_is_init)
		{
#ifdef AFF3CT_CORE_HWLOC
			/* Destroy topology object. */
			hwloc_topology_destroy(g_topology);
			g_topodepth = 0;
#endif
			g_is_init = false;
		}
		g_mtx.unlock();
	}
}

void Thread_pinning
::set_logs(const bool enable_logs)
{
	g_mtx.lock();
	g_enable_logs = enable_logs;
	g_mtx.unlock();
}

bool Thread_pinning
::is_logs()
{
	return g_enable_logs;
}

void Thread_pinning
::pin(const size_t puid)
{
	g_mtx.lock();
#ifdef AFF3CT_CORE_HWLOC
	if (g_is_init)
	{
		int pu_depth = hwloc_get_type_or_below_depth(g_topology, HWLOC_OBJ_PU);
		hwloc_obj_t pu_obj = hwloc_get_obj_by_depth(g_topology, pu_depth, puid);

		if (pu_obj == nullptr)
		{
			std::stringstream message;
			message << "'pu_obj' is nullptr ('puid' = " << puid << ").";
			throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
		}

		/* Get a copy of its cpuset that we may modify. */
		hwloc_cpuset_t cpuset = hwloc_bitmap_dup(pu_obj->cpuset);

		/* Get only one logical processor (in case the core is
		SMT/hyper-threaded). */
		hwloc_bitmap_singlify(cpuset);

		if (g_enable_logs)
		{
			char c[128];
			hwloc_bitmap_snprintf(c, 128, cpuset);

			std::cerr << "Thread pinning info -- "
			          << "PU logical index (hwloc): " << pu_obj->logical_index << " -- "
			          << "P OS index: " << pu_obj->os_index << " -- "
			          << "bitmap: " << c << std::endl;
		}

		/* And try to bind ourself there. */
		if (hwloc_set_cpubind(g_topology, cpuset, HWLOC_CPUBIND_THREAD))
		{
			char *str;
			int error = errno;
			hwloc_bitmap_asprintf(&str, pu_obj->cpuset);
			std::clog << "Couldn't bind to cpuset " << str << ": " << strerror(error) << std::endl;
			free(str);
		}

		/* Free our cpuset copy */
		hwloc_bitmap_free(cpuset);
	}
	else
	{
		if (g_enable_logs)
		{
			std::clog << "You can't call the 'pin' method if you have not call the 'init' method before, nothing will "
			          << "be done." << std::endl;
		}
	}
#else
	if (g_enable_logs)
	{
		std::clog << "'pin' method do nothing as AFF3CT has not been linked with the 'hwloc' library." << std::endl;
	}
#endif
	g_mtx.unlock();
}

// Thread pinning second function using hwloc objects
void Thread_pinning::pin(const std::string hwloc_objects)
{
	g_mtx.lock();

#ifdef AFF3CT_CORE_HWLOC
	if (g_is_init)
	{
		// Objects parsing
		std::vector<std::string> hwloc_objects_vector = Thread_pinning_utils::thread_parser(hwloc_objects);

		// getting the pairs
		std::vector<std::pair<hwloc_obj_type_t, int>> object_numbers;
		for (auto obj : hwloc_objects_vector)
			object_numbers.push_back(Thread_pinning_utils::str_to_hwloc_object(obj));

		// getting topology depth of each object
		std::vector<int> object_depth = {};
		for (auto obj : object_numbers)
			object_depth.push_back(hwloc_get_type_or_below_depth(g_topology, obj.first));

		// Allocating cpu_set
		hwloc_bitmap_t all_pus = hwloc_bitmap_alloc();
		hwloc_bitmap_zero(all_pus);

		for (size_t i = 0; i < object_numbers.size(); ++i)
		{
			hwloc_obj_t obj = hwloc_get_obj_by_depth(g_topology, object_depth[i], object_numbers[i].second);
			if (obj == nullptr)
			{
				std::stringstream message;
				message << "obj is nullptr ('Type' = " << hwloc_objects_vector[i] <<").";
				throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
			}
			hwloc_bitmap_or(all_pus, all_pus, obj->cpuset);
		}

		if (g_enable_logs)
		{
			char c[128];
			hwloc_bitmap_snprintf(c, 128, all_pus);

			std::clog << "Thread pinning info -- ";
			for (size_t i = 0; i < hwloc_objects_vector.size(); ++i)
			{
				std::clog << "Object = " << hwloc_objects_vector[i] << " | number = " << object_numbers[i].second
				          << std::endl;
			}
			std::clog << "bitmap: " << all_pus << std::endl;
		}

		/* And try to bind ourself there. */
		if (hwloc_set_cpubind(g_topology, all_pus, HWLOC_CPUBIND_THREAD))
		{
			char *str;
			int   error = errno;
			hwloc_bitmap_asprintf(&str, all_pus);
			std::clog << "Couldn't bind to cpuset " << str << ": " << strerror(error) << std::endl;
			free(str);
		}

		/* Free our cpuset copy */
		hwloc_bitmap_free(all_pus);
	}
	else
	{
		if (g_enable_logs)
		{
			std::clog << "You can't call the 'pin' method if you have not call the 'init' method before, nothing will "
			          << "be done." << std::endl;
		}
	}
#else
	if (g_enable_logs)
	{
		std::clog << "'pin' method do nothing as AFF3CT has not been linked with the 'hwloc' library." << std::endl;
	}
#endif
	g_mtx.unlock();
}


void Thread_pinning
::unpin()
{
	g_mtx.lock();
#ifdef AFF3CT_CORE_HWLOC
	if (!g_is_init)
	{
		if (g_enable_logs)
		{
			std::clog << "You can't call the 'unpin' method if you have not call the 'init' method before, nothing "
			          << "will be done." << std::endl;
		}
	}
	else
	{
		// get cpuset of root object
		hwloc_cpuset_t unpin_set = hwloc_bitmap_dup(hwloc_get_obj_by_depth(g_topology, 0, 0)->cpuset);
		if (hwloc_set_cpubind(g_topology, unpin_set, HWLOC_CPUBIND_THREAD))
		{
			char *bitmap_str;
			int error = errno;
			hwloc_bitmap_asprintf(&bitmap_str, unpin_set);
			std::clog << "'unpin' method failed ('bitmap_str' = " << bitmap_str << ", 'error' = " << strerror(error)
			          << ")" << std::endl;
			free(bitmap_str);
		}
		hwloc_bitmap_free(unpin_set);
	}
#else
	if (g_enable_logs)
	{
		std::clog << "'unpin' method do nothing as AFF3CT has not been linked with the 'hwloc' library." << std::endl;
	}
#endif
	g_mtx.unlock();
}

std::string Thread_pinning
::get_cur_cpuset_str()
{
#ifdef AFF3CT_CORE_HWLOC
	hwloc_cpuset_t cur_cpuset = hwloc_bitmap_alloc();

	hwloc_get_cpubind(g_topology, cur_cpuset, HWLOC_CPUBIND_THREAD);

	char c[128];
	hwloc_bitmap_snprintf(c, 128, cur_cpuset);

	/* Free our cpuset copy */
	hwloc_bitmap_free(cur_cpuset);

	return std::string(c);
#else
	std::stringstream message;
	message << "'get_cur_cpuset_str' method can be called only if AFF3CT is linked with the 'hwloc' library.";
	throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
#endif
}
