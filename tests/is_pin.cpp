#include <iostream>
#include <cstdlib>
#include <memory>
#include <vector>
#include <string>
#include <atomic>
#include <fstream>
#include <algorithm>
#include <getopt.h>
#include <hwloc.h>
#include <pthread.h>

#include <aff3ct-core.hpp>
using namespace aff3ct;
using namespace aff3ct::runtime;

// Definning custom finalizer class
class Finalizer_histo : public module::Finalizer<uint32_t>
{
	private:
		std::vector<uint32_t> historique;
	public:
		Finalizer_histo(size_t data_length): module::Finalizer<uint32_t>(data_length)
		{
			const std::string name = "Finalizer_histo";
			this->set_name(name);
			this->set_short_name(name);

			if (data_length == 0)
			{
				std::stringstream message;
				message << "'data_length' has to be greater than 0.";
				throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
			}

			auto &p = this->create_task("finalize_store");
			auto ps_in = this->template create_socket_in<uint32_t>(p, "in", data_length);
			this->create_codelet(p, [this,ps_in](Module &m, runtime::Task &t, const size_t frame_id) -> int
			{
				auto data_in = t[ps_in].get_dataptr<const uint32_t>();
				this->historique.push_back(*data_in);
				return runtime::status_t::SUCCESS;
			});
		}

		std::vector<uint32_t> get_histo()
		{
			return historique;
		}
};

int main(int argc, char** argv)
{
	tools::setup_signal_handler();

	option longopts[] = {
		{"n-exec", required_argument, NULL, 'e'},
		{"dot-filepath", required_argument, NULL, 'o'},
		{"print-stats", no_argument, NULL, 'p'},
		{"debug", no_argument, NULL, 'g'},
		{"active-waiting", no_argument, NULL, 'w'},
		{"help", no_argument, NULL, 'h'},
		{0}};

	size_t n_inter_frames = 1;
	size_t data_length = 1;
	size_t n_exec = 100000;
	size_t buffer_size = 4 ;
	std::string dot_filepath;
	bool print_stats = false;
	bool debug = false;
	bool active_waiting = false;

	while (1)
	{
		const int opt = getopt_long(argc, argv, "t:f:s:d:e:u:o:pbgqwh", longopts, 0);
		if (opt == -1)
			break;
		switch (opt)
		{
			case 'e':
				n_exec = atoi(optarg);
				break;
			case 'o':
				dot_filepath = std::string(optarg);
				break;
			case 'p':
				print_stats = true;
				break;
			case 'g':
				debug = true;
				break;
			case 'w':
				active_waiting = true;
				break;
			case 'h':
				std::cout << "usage: " << argv[0] << " [options]" << std::endl;
				std::cout << std::endl;
				std::cout << "  -e, --n-exec          "
				          << "Number of sequence executions                                         "
				          << "[" << n_exec << "]" << std::endl;
				std::cout << "  -o, --dot-filepath    "
				          << "Path to dot output file                                               "
				          << "[" << (dot_filepath.empty() ? "empty" : "\"" + dot_filepath + "\"") << "]" << std::endl;
				std::cout << "  -p, --print-stats     "
				          << "Enable to print per task statistics (performance will be reduced)     "
				          << "[" << (print_stats ? "true" : "false") << "]" << std::endl;
				std::cout << "  -g, --debug           "
				          << "Enable task debug mode (print socket data)                            "
				          << "[" << (debug ? "true" : "false") << "]" << std::endl;
				std::cout << "  -w, --active-waiting  "
				          << "Enable active waiting in the pipeline synchronizations                "
				          << "[" << (active_waiting ? "true" : "false") << "]" << std::endl;
				std::cout << "  -h, --help            "
				          << "This help                                                             "
				          << "[false]" << std::endl;
				exit(0);
				break;
			default:
				break;
		}
	}

	std::cout << "############################################################" << std::endl;
	std::cout << "# Micro-benchmark: Complexe pipeline increment and compare #" << std::endl;
	std::cout << "############################################################" << std::endl;
	std::cout << "#" << std::endl;

	std::cout << "# Command line arguments:" << std::endl;
	std::cout << "#   - n_inter_frames = " << n_inter_frames << std::endl;
	std::cout << "#   - data_length    = " << data_length << std::endl;
	std::cout << "#   - n_exec         = " << n_exec << std::endl;
	std::cout << "#   - buffer_size    = " << buffer_size << std::endl;
	std::cout << "#   - dot_filepath   = " << (dot_filepath.empty() ? "[empty]" : dot_filepath.c_str()) << std::endl;
	std::cout << "#   - print_stats    = " << (print_stats ? "true" : "false") << std::endl;
	std::cout << "#   - debug          = " << (debug ? "true" : "false") << std::endl;
	std::cout << "#   - active_waiting = " << (active_waiting ? "true" : "false") << std::endl;
	std::cout << "#" << std::endl;

	tools::Thread_pinning::init();
	module::Finalizer  <uint32_t> finalizer  (data_length);
	std::vector<std::shared_ptr<Finalizer_histo>> finalizers_histo(3);
	for (size_t s = 0; s < finalizers_histo.size(); s++)
	{
		finalizers_histo[s].reset(new Finalizer_histo(data_length));
	}
	// Getting hwloc topology
	hwloc_topology_t g_topology;
	hwloc_topology_init(&g_topology);
	hwloc_topology_load(g_topology);

	/*
	Stateless module, with a task which forwards the current hardware core of the running thread
	*/
	module::Stateless pin_mod;
	pin_mod.set_name("Pinner");
	auto& pin_task = pin_mod.create_task("pin");
	auto sock_out = pin_mod.create_socket_out<uint32_t>(pin_task, "val", data_length);

	pin_mod.create_codelet(pin_task,[sock_out, data_length, g_topology]
		(module::Module &m, runtime::Task &t, const size_t frame_id) -> int
	{
		auto tab = t[sock_out].get_dataptr<uint32_t>();
		hwloc_bitmap_t set = hwloc_bitmap_alloc();
		hwloc_get_thread_cpubind(g_topology,pthread_self(),set, 0);
		auto core = sched_getcpu();
		auto pu_obj = hwloc_get_pu_obj_by_os_index(g_topology, core);
		 *tab = pu_obj->logical_index;
		return runtime::status_t::SUCCESS;
	});

	module::Stateless *pin_mod1 = &pin_mod;
	module::Stateless *pin_mod2 = pin_mod.clone();
	module::Stateless *pin_mod3 = pin_mod.clone();

	// sockets binding
	(*pin_mod2)("pin") = (*pin_mod1)["pin::val"];
	(*pin_mod3)("pin") = (*pin_mod2)["pin::val"];
	(*finalizers_histo[0])["finalize_store::in"] = (*pin_mod1)["pin::val"];
	(*finalizers_histo[1])["finalize_store::in"] = (*pin_mod2)["pin::val"];
	(*finalizers_histo[2])["finalize_store::in"] = (*pin_mod3)["pin::val"];

	std::unique_ptr<runtime::Pipeline> pipeline_chain;
	std::vector<std::vector<Finalizer_histo*>> finalizer_histo_list;
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////// PIPELINE EXEC //
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	pipeline_chain.reset(new runtime::Pipeline(
		(*pin_mod1)("pin"), // first task of the sequence
		{  // pipeline stage 0
			{ { &(*pin_mod1)("pin") },  		// first tasks of stage 0
			  { &(*pin_mod1)("pin") } },		// last tasks of stage 0
			// pipeline stage 1
			{ { &(*pin_mod2)("pin") },  		// first tasks of stage 1
			  { &(*pin_mod2)("pin") } },		// last tasks of stage 1
			// pipeline stage 2
			{ { &(*pin_mod3)("pin") },  		// first tasks of stage 2
			  { &(*pin_mod3)("pin") } },		// last tasks of stage 0
			// pipeline stage 3
			{ { &((*finalizers_histo[0])("finalize_store")),	// first tasks of stage 3
				&((*finalizers_histo[1])("finalize_store")),
				&((*finalizers_histo[2])("finalize_store")) },

				{ &((*finalizers_histo[0])("finalize_store")),	// last tasks of stage 3
				&((*finalizers_histo[1])("finalize_store")),
				&((*finalizers_histo[2])("finalize_store")) } },
		},
		{
			1, // number of threads in the stage 0
			3, // number of threads in the stage 1
			1, // number of threads in the stage 2
			1, // number of threads in the stage 3
		},
		{
		   	buffer_size, // synchronization buffer size between stages 0 and 1
		   	buffer_size, // synchronization buffer size between stages 1 and 2
			buffer_size, // synchronization buffer size between stages 2 and 3
		},
		{
		   	active_waiting, // type of waiting between stages 0 and 1 (true = active, false = passive)
		   	active_waiting, // type of waiting between stages 1 and 2 (true = active, false = passive)
			active_waiting, // type of waiting between stages 2 and 3 (true = active, false = passive)
		},
		{
			true,	// Pinning activation for stage 0
			true,	// Pinning activation for stage 1
			true,	// Pinning activation for stage 2
			false	// Pinning activation for stage 3
		},
		" PU_0 | PU_0; PU_1; PU_2 | PU_3 |"));
	pipeline_chain->set_n_frames(n_inter_frames);

	if (!dot_filepath.empty())
	{
		std::ofstream file(dot_filepath);
		pipeline_chain->export_dot(file);
	}

	// configuration of the sequence tasks
	for (auto& mod : pipeline_chain->get_modules<module::Module>(false)) for (auto& tsk : mod->tasks)
	{
		tsk->reset          (           );
		tsk->set_debug      (debug      ); // disable the debug mode
		tsk->set_debug_limit(16         ); // display only the 16 first bits if the debug mode is enabled
		tsk->set_stats      (print_stats); // enable the statistics
		tsk->set_fast       (true       ); // enable the fast mode (= disable the useless verifs in the tasks)
	}

	auto t_start = std::chrono::steady_clock::now();
	std::atomic<unsigned int> counter(0);
	pipeline_chain->exec([&counter, n_exec]() { return ++counter >= n_exec; } );
	std::chrono::nanoseconds duration = std::chrono::steady_clock::now() - t_start;
	auto elapsed_time = duration.count() / 1000.f / 1000.f;
	std::cout << "Sequence elapsed time: " << elapsed_time << " ms" << std::endl;

	for (size_t i = 0; i < finalizers_histo.size(); ++i)
	{
		finalizer_histo_list.push_back(pipeline_chain.get()->get_stages()[pipeline_chain.get()->get_stages().size()-1]
		->get_cloned_modules<Finalizer_histo>(*finalizers_histo[i].get()));
	}

	// verification of the pipeline thread binding
	bool tests_passed = true;
	for (size_t i = 0; i < n_exec; ++i)
	{
		if (finalizer_histo_list[0][0]->get_histo()[i] != 0)
		{
			tests_passed = false;
			std::cout << "# Thread is not pin to exepcted value for the execution number " << i << std::endl <<
					"Stage : " << 0 << ", Thread number : " << 0 << std::endl <<
					"Exepected  : 0, " << "Real : " << finalizer_histo_list[0][0]->get_histo()[i] << std::endl <<
					"Did you compile with  -DAFF3CT_CORE_LINK_HWLOC=ON ?" << std::endl;
			break;
		}

		if (finalizer_histo_list[2][0]->get_histo()[i] != 3)
		{
			tests_passed = false;
			std::cout << "# Thread is not pin to exepcted value for the execution number " << i << std::endl <<
					"Stage : " << 2 << ", Thread number : " << 0 << std::endl<<
					"Exepected  : 3, " << "Real : " << finalizer_histo_list[2][0]->get_histo()[i] << std::endl <<
					"Did you compile with  -DAFF3CT_CORE_LINK_HWLOC=ON ?" << std::endl;
			break;
		}

		uint32_t exepcted = i%3 == 0 ? (0) : (i%3 == 1 ? (1) : 2);
		if (finalizer_histo_list[1][0]->get_histo()[i] != exepcted)
		{
			tests_passed = false;
			std::cout << "# Thread is not pin to exepcted value for the execution number " << i << std::endl <<
					"Stage : " << 1 << ", Thread number : " << i%3 << std::endl <<
					"Exepected  : " << exepcted << ", Real : " << finalizer_histo_list[1][0]->get_histo()[i]
					<< std::endl << "Did you compile with  -DAFF3CT_CORE_LINK_HWLOC=ON ?" << std::endl;
			break;
		}
	}
	if (tests_passed)
		std::cout << "# " << rang::style::bold << rang::fg::green << "Tests passed!" << rang::style::reset
		          << std::endl;
	else
		std::cout << "# " << rang::style::bold << rang::fg::red << "Tests failed :-(" << rang::style::reset
		          << std::endl;

	unsigned int test_results = !tests_passed;

	pipeline_chain->set_n_frames(1);
	pipeline_chain->unbind_adaptors();

	(*pin_mod2)("pin").unbind((*pin_mod1)["pin::val"]);
	(*pin_mod3)("pin").unbind((*pin_mod2)["pin::val"]);
	(*finalizers_histo[0])["finalize_store::in"].unbind((*pin_mod1)["pin::val"]);
	(*finalizers_histo[1])["finalize_store::in"].unbind((*pin_mod2)["pin::val"]);
	(*finalizers_histo[2])["finalize_store::in"].unbind((*pin_mod3)["pin::val"]);

	return test_results;
}
