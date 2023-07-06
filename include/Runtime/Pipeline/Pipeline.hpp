/*!
 * \file
 * \brief Class runtime::Pipeline.
 */
#ifndef PIPELINE_HPP_
#define PIPELINE_HPP_

#include <functional>
#include <cstdint>
#include <utility>
#include <memory>
#include <vector>

#include "Tools/Interface/Interface_get_set_n_frames.hpp"
#include "Runtime/Sequence/Sequence.hpp"

namespace aff3ct
{
namespace runtime
{
class Pipeline : public tools::Interface_get_set_n_frames
{


protected:
	Sequence original_sequence;
	std::vector<std::shared_ptr<Sequence>> stages;
	std::vector<std::pair<std::vector<std::shared_ptr<module::Adaptor>>,
	                      std::vector<std::shared_ptr<module::Adaptor>>>> adaptors;

	

	//                               sck out addr      stage   tsk id  sck id  unbind_pos
	std::vector<std::pair<std::tuple<runtime::Socket*, size_t, size_t, size_t, size_t>,
	//                               sck in addr       stage   tsk id  sck id
	                      std::tuple<runtime::Socket*, size_t, size_t, size_t>>> sck_orphan_binds;

	//                     sck out addr      sck in addr      priority
	std::vector<std::tuple<runtime::Socket*, runtime::Socket*, size_t>> adaptors_binds;

	std::vector<std::vector<size_t>> saved_firsts_tasks_id;
	std::vector<std::vector<size_t>> saved_lasts_tasks_id;

	bool bound_adaptors;
	bool auto_stop;
	

public:
	// Pipeline(const runtime::Task &first,
	//          const std::vector<std::pair<std::vector<const runtime::Task*>, std::vector<const runtime::Task*>>> &sep_stages = {},
	//          const std::vector<size_t> &n_threads = {},
	//          const std::vector<size_t> &synchro_buffer_sizes = {},
	//          const std::vector<bool> &synchro_active_waiting = {},
	//          const std::vector<bool> &thread_pinning = {},
	//          const std::vector<std::vector<size_t>> &puids = {});
	// Pipeline(const runtime::Task &first,
	//          const runtime::Task &last,
	//          const std::vector<std::pair<std::vector<const runtime::Task*>, std::vector<const runtime::Task*>>> &sep_stages = {},
	//          const std::vector<size_t> &n_threads = {},
	//          const std::vector<size_t> &synchro_buffer_sizes = {},
	//          const std::vector<bool> &synchro_active_waiting = {},
	//          const std::vector<bool> &thread_pinning = {},
	//          const std::vector<std::vector<size_t>> &puids = {});
	Pipeline(const std::vector<runtime::Task*> &firsts,
	         const std::vector<runtime::Task*> &lasts,
	         const std::vector<std::tuple<std::vector<runtime::Task*>, std::vector<runtime::Task*>, std::vector<runtime::Task*>>> &sep_stages = {},
	         const std::vector<size_t> &n_threads = {},
	         const std::vector<size_t> &synchro_buffer_sizes = {},
	         const std::vector<bool> &synchro_active_waiting = {},
	         const std::vector<bool> &thread_pinning = {},
	         const std::vector<std::vector<size_t>> &puids = {}/*,
	         const std::vector<bool> &tasks_inplace = {}*/);
	Pipeline(const std::vector<runtime::Task*> &firsts,
	         const std::vector<runtime::Task*> &lasts,
	         const std::vector<std::pair<std::vector<runtime::Task*>, std::vector<runtime::Task*>>> &sep_stages = {},
	         const std::vector<size_t> &n_threads = {},
	         const std::vector<size_t> &synchro_buffer_sizes = {},
	         const std::vector<bool> &synchro_active_waiting = {},
	         const std::vector<bool> &thread_pinning = {},
	         const std::vector<std::vector<size_t>> &puids = {}/*,
	         const std::vector<bool> &tasks_inplace = {}*/);
	Pipeline(const std::vector<runtime::Task*> &firsts,
	         const std::vector<std::tuple<std::vector<runtime::Task*>, std::vector<runtime::Task*>, std::vector<runtime::Task*>>> &sep_stages = {},
	         const std::vector<size_t> &n_threads = {},
	         const std::vector<size_t> &synchro_buffer_sizes = {},
	         const std::vector<bool> &synchro_active_waiting = {},
	         const std::vector<bool> &thread_pinning = {},
	         const std::vector<std::vector<size_t>> &puids = {}/*,
	         const std::vector<bool> &tasks_inplace = {}*/);
	Pipeline(const std::vector<runtime::Task*> &firsts,
	         const std::vector<std::pair<std::vector<runtime::Task*>, std::vector<runtime::Task*>>> &sep_stages = {},
	         const std::vector<size_t> &n_threads = {},
	         const std::vector<size_t> &synchro_buffer_sizes = {},
	         const std::vector<bool> &synchro_active_waiting = {},
	         const std::vector<bool> &thread_pinning = {},
	         const std::vector<std::vector<size_t>> &puids = {}/*,
	         const std::vector<bool> &tasks_inplace = {}*/);
	Pipeline(runtime::Task &first,
	         const std::vector<std::pair<std::vector<runtime::Task*>, std::vector<runtime::Task*>>> &sep_stages = {},
	         const std::vector<size_t> &n_threads = {},
	         const std::vector<size_t> &synchro_buffer_sizes = {},
	         const std::vector<bool> &synchro_active_waiting = {},
	         const std::vector<bool> &thread_pinning = {},
	         const std::vector<std::vector<size_t>> &puids = {}/*,
	         const std::vector<bool> &tasks_inplace = {}*/);
	Pipeline(runtime::Task &first,
	         runtime::Task &last,
	         const std::vector<std::pair<std::vector<runtime::Task*>, std::vector<runtime::Task*>>> &sep_stages = {},
	         const std::vector<size_t> &n_threads = {},
	         const std::vector<size_t> &synchro_buffer_sizes = {},
	         const std::vector<bool> &synchro_active_waiting = {},
	         const std::vector<bool> &thread_pinning = {},
	         const std::vector<std::vector<size_t>> &puids = {}/*,
	         const std::vector<bool> &tasks_inplace = {}*/);

	virtual ~Pipeline();

	void exec(const std::vector<std::function<bool(const std::vector<const int*>&)>> &stop_conditions);
	void exec(const std::vector<std::function<bool(                              )>> &stop_conditions);

	void exec(std::function<bool(const std::vector<const int*>&)> stop_condition);
	void exec(std::function<bool(                              )> stop_condition);
	void exec(                                                                  );

	std::vector<Sequence*> get_stages();
	Sequence& operator[](const size_t stage_id);

	template <class C = module::Module>
	std::vector<C*> get_modules(const bool subsequence_modules = true) const;
	std::vector<std::vector<module::Module*>> get_modules_per_threads() const;
	std::vector<std::vector<module::Module*>> get_modules_per_types  () const;

	std::vector<std::vector<runtime::Task*>> get_tasks_per_threads() const;
	std::vector<std::vector<runtime::Task*>> get_tasks_per_types  () const;

	inline const std::vector<std::vector<runtime::Task*>>& get_firsts_tasks() const;
	inline const std::vector<std::vector<runtime::Task*>>& get_lasts_tasks() const;

	void export_dot(std::ostream &stream = std::cout) const;

	void bind_adaptors();
	void unbind_adaptors();
	bool is_bound_adaptors() const;

	void set_auto_stop(const bool auto_stop);
	bool is_auto_stop() const;

	size_t get_n_frames() const;
	void set_n_frames(const size_t n_frames);

protected:
	void create_adaptors(const std::vector<size_t> &synchro_buffer_sizes = {},
	                     const std::vector<bool> &synchro_active_waiting = {});

	void _bind_adaptors(const bool bind_adaptors = true);
	void _unbind_adaptors(const bool bind_orphans = true);
private:
	template <class TA>
	void init(const std::vector<TA*> &firsts,
	          const std::vector<TA*> &lasts,
	          const std::vector<std::tuple<std::vector<TA*>, std::vector<TA*>, std::vector<TA*>>> &sep_stages = {},
	          const std::vector<size_t> &n_threads = {},
	          const std::vector<size_t> &synchro_buffer_sizes = {},
	          const std::vector<bool> &synchro_active_waiting = {},
	          const std::vector<bool> &thread_pinning = {},
	          const std::vector<std::vector<size_t>> &puids = {}/*,
	          const std::vector<bool> &tasks_inplace = {}*/);
};
}
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "Runtime/Pipeline/Pipeline.hxx"
#endif

#endif /* PIPELINE_HPP_ */
