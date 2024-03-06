#include <iostream>
#include <cstdlib>
#include <memory>
#include <vector>
#include <string>
#include <atomic>
#include <fstream>
#include <algorithm>
#include <getopt.h>

#include <aff3ct-core.hpp>
using namespace aff3ct;
using namespace aff3ct::runtime;

int main(int argc, char** argv)
{
	tools::setup_signal_handler();

	option longopts[] = {
		{"n-threads", required_argument, NULL, 't'},
		{"n-inter-frames", required_argument, NULL, 'f'},
		{"sleep-time", required_argument, NULL, 's'},
		{"data-length", required_argument, NULL, 'd'},
		{"n-exec", required_argument, NULL, 'e'},
		{"buffer-size", required_argument, NULL, 'u'},
		{"dot-filepath", required_argument, NULL, 'o'},
		{"print-stats", no_argument, NULL, 'p'},
		{"step-by-step", no_argument, NULL, 'b'},
		{"debug", no_argument, NULL, 'g'},
		{"force-sequence", no_argument, NULL, 'q'},
		{"active-waiting", no_argument, NULL, 'w'},
		{"help", no_argument, NULL, 'h'},
		{0}};

	size_t n_threads = std::thread::hardware_concurrency();
	size_t n_inter_frames = 1;
	size_t sleep_time_us = 5;
	size_t data_length = 2048;
	size_t n_exec = 100000;
	size_t buffer_size = 16;
	std::string dot_filepath;
	bool print_stats = false;
	bool step_by_step = false;
	bool debug = false;
	bool force_sequence = false;
	bool active_waiting = false;

	while (1)
	{
		const int opt = getopt_long(argc, argv, "t:f:s:d:e:u:o:pbgqwh", longopts, 0);
		if (opt == -1)
			break;
		switch (opt)
		{
			case 't':
				n_threads = atoi(optarg);
				break;
			case 'f':
				n_inter_frames = atoi(optarg);
				break;
			case 's':
				sleep_time_us = atoi(optarg);
				break;
			case 'd':
				data_length = atoi(optarg);
				break;
			case 'e':
				n_exec = atoi(optarg);
				break;
			case 'u':
				buffer_size = atoi(optarg);
				break;
			case 'o':
				dot_filepath = std::string(optarg);
				break;
			case 'p':
				print_stats = true;
				break;
			case 'b':
				step_by_step = true;
				break;
			case 'g':
				debug = true;
				break;
			case 'w':
				active_waiting = true;
				break;
			case 'q':
				force_sequence = true;
				break;
			case 'h':
				std::cout << "usage: " << argv[0] << " [options]" << std::endl;
				std::cout << std::endl;
				std::cout << "  -t, --n-threads       "
				          << "Number of threads to run in parallel                                  "
				          << "[" << n_threads << "]" << std::endl;
				std::cout << "  -f, --n-inter-frames  "
				          << "Number of frames to process in one task                               "
				          << "[" << n_inter_frames << "]" << std::endl;
				std::cout << "  -s, --sleep-time      "
				          << "Sleep time duration in one task (microseconds)                        "
				          << "[" << sleep_time_us << "]" << std::endl;
				std::cout << "  -d, --data-length     "
				          << "Size of data to process in one task (in bytes)                        "
				          << "[" << data_length << "]" << std::endl;
				std::cout << "  -e, --n-exec          "
				          << "Number of sequence executions                                         "
				          << "[" << n_exec << "]" << std::endl;
				std::cout << "  -u, --buffer-size     "
				          << "Size of the buffer between the different stages of the pipeline       "
				          << "[" << data_length << "]" << std::endl;
				std::cout << "  -o, --dot-filepath    "
				          << "Path to dot output file                                               "
				          << "[" << (dot_filepath.empty() ? "empty" : "\"" + dot_filepath + "\"") << "]" << std::endl;
				std::cout << "  -b, --step-by-step    "
				          << "Enable step-by-step sequence execution (performance will be reduced)  "
				          << "[" << (step_by_step ? "true" : "false") << "]" << std::endl;
				std::cout << "  -p, --print-stats     "
				          << "Enable to print per task statistics (performance will be reduced)     "
				          << "[" << (print_stats ? "true" : "false") << "]" << std::endl;
				std::cout << "  -g, --debug           "
				          << "Enable task debug mode (print socket data)                            "
				          << "[" << (debug ? "true" : "false") << "]" << std::endl;
				std::cout << "  -q, --force-sequence  "
				          << "Force sequence instead of pipeline                                    "
				          << "[" << (force_sequence ? "true" : "false") << "]" << std::endl;
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

	std::cout << "#########################################################" << std::endl;
	std::cout << "# Micro-benchmark: Mixed pipeline increment and compare #" << std::endl;
	std::cout << "#########################################################" << std::endl;
	std::cout << "#" << std::endl;

	std::cout << "# Command line arguments:" << std::endl;
	std::cout << "#   - n_threads      = " << n_threads << std::endl;
	std::cout << "#   - n_inter_frames = " << n_inter_frames << std::endl;
	std::cout << "#   - sleep_time_us  = " << sleep_time_us << std::endl;
	std::cout << "#   - data_length    = " << data_length << std::endl;
	std::cout << "#   - n_exec         = " << n_exec << std::endl;
	std::cout << "#   - buffer_size    = " << buffer_size << std::endl;
	std::cout << "#   - dot_filepath   = " << (dot_filepath.empty() ? "[empty]" : dot_filepath.c_str()) << std::endl;
	std::cout << "#   - print_stats    = " << (print_stats ? "true" : "false") << std::endl;
	std::cout << "#   - step_by_step   = " << (step_by_step ? "true" : "false") << std::endl;
	std::cout << "#   - debug          = " << (debug ? "true" : "false") << std::endl;
	std::cout << "#   - force_sequence = " << (force_sequence ? "true" : "false") << std::endl;
	std::cout << "#   - active_waiting = " << (active_waiting ? "true" : "false") << std::endl;
	std::cout << "#" << std::endl;

	if (!force_sequence && step_by_step)
		std::clog << rang::tag::warning << "'step_by_step' is not available with pipeline" << std::endl;

	module::Initializer<uint8_t> initializer(data_length);
	module::Finalizer  <uint8_t> finalizer1 (data_length);
	module::Finalizer  <uint8_t> finalizer2 (data_length);

	// Incrementers construction
	std::vector<std::shared_ptr<module::Incrementer<uint8_t>>> incs(2);
	for (size_t s = 0; s < incs.size(); s++)
	{
		incs[s].reset(new module::Incrementer<uint8_t>(data_length));
		incs[s]->set_ns(sleep_time_us * 1000);
		incs[s]->set_custom_name("Inc" + std::to_string(s));
	}

	// Relayers construction
	std::vector<std::shared_ptr<module::Relayer<uint8_t>>> rlys(2);
	for (size_t s = 0; s < rlys.size(); s++)
	{
		rlys[s].reset(new module::Relayer<uint8_t>(data_length));
		rlys[s]->set_ns(sleep_time_us * 1000);
		rlys[s]->set_custom_name("Relayer" + std::to_string(s));
	}

	// Stateless module used only for the test
	module::Stateless comp;
	comp.set_name("comparator");
	auto& task_comp = comp.create_task("compare");
	auto sock_0 = comp.create_socket_fwd<uint8_t>(task_comp, "fwd_0", data_length);
	auto sock_1 = comp.create_socket_fwd<uint8_t>(task_comp, "fwd_1", data_length);
	auto sock_2 = comp.create_socket_fwd<uint8_t>(task_comp, "fwd_2", data_length);

	comp.create_codelet(task_comp,
		[sock_0,sock_1,sock_2,data_length,incs](module::Module &m, runtime::Task &t, const size_t frame_id) -> int
	{
		auto tab_0 = t[sock_0].get_dataptr<const uint8_t>();
		auto tab_1 = t[sock_1].get_dataptr<const uint8_t>();
		auto tab_2 = t[sock_2].get_dataptr<const uint8_t>();
		for (size_t i = 0; i < data_length; i++)
		{
			if (tab_0[i] != tab_1[i] || tab_0[i] != tab_2[i])
			{
				std::stringstream message;
				message << "Found different values => " << " Tab_0 : " << +tab_0[i] <<  ", Tab_1 : "
				        << +tab_1[i] << ", Tab_2 : " << +tab_2[i] << std::endl;
				throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
			}
		}
		return runtime::status_t::SUCCESS;
	});

	// sockets binding
	(*rlys[0])[module::rly::sck::relayf::fwd] = initializer[module::ini::sck::initialize::out];
	(*rlys[0])[module::rly::sck::relay::in] = initializer[module::ini::sck::initialize::out];
	(*incs[0])[module::inc::sck::incrementf::fwd] = (*rlys[0])[module::rly::sck::relayf::fwd];
	(*incs[1])[module::inc::sck::incrementf::fwd] = (*rlys[0])[module::rly::sck::relayf::fwd];
	(*incs[1])[module::inc::tsk::incrementf] = (*rlys[0])[module::rly::sck::relay::status];
	(*rlys[1])[module::rly::sck::relayf::fwd] = (*incs[0])[module::inc::sck::incrementf::fwd];
	comp["compare::fwd_0"] = (*rlys[1])[module::rly::sck::relayf::fwd];
	comp["compare::fwd_1"] = (*incs[1])[module::inc::sck::incrementf::fwd];
	comp["compare::fwd_2"] = initializer[module::ini::sck::initialize::out];
	finalizer1[module::fin::sck::finalize::in] = comp["compare::fwd_2"];
	finalizer2[module::fin::sck::finalize::in] = (*rlys[0])[module::rly::sck::relay::out];
	finalizer2[module::fin::tsk::finalize] = finalizer1[module::fin::sck::finalize::status];

	std::unique_ptr<runtime::Sequence> sequence_chain;
	std::unique_ptr<runtime::Pipeline> pipeline_chain;
	std::vector<module::Finalizer<uint8_t>*> finalizer1_list, finalizer2_list;
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////// SEQUENCE EXEC //
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	if (force_sequence)
	{
		sequence_chain.reset(new runtime::Sequence(initializer[module::ini::tsk::initialize], n_threads));
		sequence_chain->set_n_frames(n_inter_frames);

		// initialize input data
		auto tid = 0;
		for (auto cur_initializer : sequence_chain.get()->get_cloned_modules<module::Initializer<uint8_t>>(initializer))
		{
			std::vector<std::vector<uint8_t>> init_data(n_inter_frames, std::vector<uint8_t>(data_length));
			for (size_t f = 0; f < n_inter_frames; f++)
				std::fill(init_data[f].begin(), init_data[f].end(), 40 + n_inter_frames * tid + f);
			cur_initializer->set_init_data(init_data);
			tid++;
		}

		if (!dot_filepath.empty())
		{
			std::ofstream file(dot_filepath);
			sequence_chain->export_dot(file);
		}

		// configuration of the sequence tasks
		for (auto& mod : sequence_chain->get_modules<module::Module>(false)) for (auto& tsk : mod->tasks)
		{
			tsk->reset          (           );
			tsk->set_debug      (debug      ); // disable the debug mode
			tsk->set_debug_limit(16         ); // display only the 16 first bits if the debug mode is enabled
			tsk->set_stats      (print_stats); // enable the statistics
			tsk->set_fast       (true       ); // enable the fast mode (= disable the useless verifs in the tasks)
		}

		std::atomic<unsigned int> counter(0);
		auto t_start = std::chrono::steady_clock::now();
		if (!step_by_step)
			sequence_chain->exec([&counter, n_exec]() { return ++counter >= n_exec; });
		else
		{
			do
			{
				for (size_t tid = 0; tid < n_threads; tid++)
					while (sequence_chain->exec_step(tid));
			}
			while (++counter < (n_exec / n_threads));
		}
		std::chrono::nanoseconds duration = std::chrono::steady_clock::now() - t_start;

		auto elapsed_time = duration.count() / 1000.f / 1000.f;
		std::cout << "Sequence elapsed time: " << elapsed_time << " ms" << std::endl;

		finalizer1_list = sequence_chain.get()->get_cloned_modules<module::Finalizer<uint8_t>>(finalizer1);
		finalizer2_list = sequence_chain.get()->get_cloned_modules<module::Finalizer<uint8_t>>(finalizer2);
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////// PIPELINE EXEC //
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	else
	{
		pipeline_chain.reset(new runtime::Pipeline(
			{ &initializer[module::ini::tsk::initialize] }, // first task of the sequence
			{  // pipeline stage 0
			   { { &initializer[module::ini::tsk::initialize] },    // first tasks of stage 0
			     { &(*rlys[0])[module::rly::tsk::relayf],     },    // last  tasks of stage 0
			     { &(*rlys[0])[module::rly::tsk::relay]       }, }, // excep tasks of stage 0
			   // pipeline stage 1
			   { { &(*incs[0])[module::inc::tsk::incrementf],       // first tasks of stage 1
			       &(*rlys[0])[module::rly::tsk::relay],      },    // last  tasks of stage 1
			     { &(*incs[0])[module::inc::tsk::incrementf]  },
			     {                                            }, }, // excep tasks of stage 1
			   // pipeline stage 2
			   { { &(*rlys[1])[module::rly::tsk::relayf]      },    // first tasks of stage 2
			     { &(*rlys[1])[module::rly::tsk::relayf]      },    // last  tasks of stage 2
			     {                                            }, }, // excep tasks of stage 2
			   // pipeline stage 3
			   { { &(*incs[1])[module::inc::tsk::incrementf]  },    // first tasks of stage 3
			     { &(*incs[1])[module::inc::tsk::incrementf]  },    // last  tasks of stage 3
			     {                                            } },  // excep tasks of stage 3
			   // pipeline stage 4
			   { { &task_comp,                                      // first tasks of stage 4
			       &finalizer2[module::fin::tsk::finalize]    },
			     {                                            },    // last  tasks of stage 4
			     {                                            }, }, // excep tasks of stage 4
			},
			{
			   1,                         // number of threads in the stage 0
			   n_threads ? n_threads : 1, // number of threads in the stage 1
			   1,                         // number of threads in the stage 2
			   n_threads ? n_threads : 1, // number of threads in the stage 3
			   1                          // number of threads in the stage 4
			},
			{
			   buffer_size, // synchronization buffer size between stages 0 and 1
			   buffer_size, // synchronization buffer size between stages 1 and 2
			   buffer_size, // synchronization buffer size between stages 2 and 3
			   buffer_size, // synchronization buffer size between stages 4 and 4
			},
			{
			   active_waiting, // type of waiting between stages 0 and 1 (true = active, false = passive)
			   active_waiting, // type of waiting between stages 1 and 2 (true = active, false = passive)
			   active_waiting, // type of waiting between stages 2 and 3 (true = active, false = passive)
			   active_waiting, // type of waiting between stages 3 and 4 (true = active, false = passive)
			}));
		pipeline_chain->set_n_frames(n_inter_frames);

		// initialize input data
		auto tid = 0;
		for (auto cur_initializer :
			pipeline_chain.get()->get_stages()[0]->get_cloned_modules<module::Initializer<uint8_t>>(initializer))
		{
			std::vector<std::vector<uint8_t>> init_data(n_inter_frames, std::vector<uint8_t>(data_length));
			for (size_t f = 0; f < n_inter_frames; f++)
				std::fill(init_data[f].begin(), init_data[f].end(), 40 + n_inter_frames * tid + f);
			cur_initializer->set_init_data(init_data);
			tid++;
		}

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

		finalizer1_list = pipeline_chain.get()->get_stages()[pipeline_chain.get()->get_stages().size()-1]
			->get_cloned_modules<module::Finalizer<uint8_t>>(finalizer1);
		finalizer2_list = pipeline_chain.get()->get_stages()[pipeline_chain.get()->get_stages().size()-1]
			->get_cloned_modules<module::Finalizer<uint8_t>>(finalizer2);
	}

	// verification of the sequence execution
	bool tests_passed = true;
	auto tid = 0;

	for (auto cur_finalizer : finalizer1_list)
	{
		for (size_t f = 0; f < n_inter_frames; f++)
		{
			const auto &final_data = cur_finalizer->get_final_data()[f];
			for (size_t d = 0; d < final_data.size(); d++)
			{
				auto expected = (int)(42 + n_inter_frames * tid + f);
				expected = expected % 256;
				if (final_data[d] != expected)
				{
					std::cout << "# expected = " << +expected << " - obtained = "
					          << +final_data[d] << " (d = " << d << ", tid = " << tid << ")" << std::endl;
					tests_passed = false;
				}
			}
		}
		tid++;
	}

	tid = 0;
	for (auto cur_finalizer : finalizer2_list)
	{
		for (size_t f = 0; f < n_inter_frames; f++)
		{
			const auto &final_data = cur_finalizer->get_final_data()[f];
			for (size_t d = 0; d < final_data.size(); d++)
			{
				auto expected = (int)(41 + n_inter_frames * tid + f);
				expected = expected % 256;
				if (final_data[d] != expected)
				{
					std::cout << "# expected = " << +expected << " - obtained = "
					          << +final_data[d] << " (d = " << d << ", tid = " << tid << ")" << std::endl;
					tests_passed = false;
				}
			}
		}
		tid++;
	}

	if (tests_passed)
		std::cout << "# " << rang::style::bold << rang::fg::green << "Tests passed!" << rang::style::reset
		          << std::endl;
	else
		std::cout << "# " << rang::style::bold << rang::fg::red << "Tests failed :-(" << rang::style::reset
		          << std::endl;

	unsigned int test_results = !tests_passed;

	if (force_sequence)
		sequence_chain->set_n_frames(1);
	else
	{
		pipeline_chain->set_n_frames(1);
		pipeline_chain->unbind_adaptors();
	}

	(*rlys[0])[module::rly::sck::relayf::fwd].unbind(initializer[module::ini::sck::initialize::out]);
	(*rlys[0])[module::rly::sck::relay::in].unbind(initializer[module::ini::sck::initialize::out]);
	(*incs[0])[module::inc::sck::incrementf::fwd].unbind((*rlys[0])[module::rly::sck::relayf::fwd]);
	(*incs[1])[module::inc::sck::incrementf::fwd].unbind((*rlys[0])[module::rly::sck::relayf::fwd]);
	(*incs[1])[module::inc::tsk::incrementf].unbind((*rlys[0])[module::rly::sck::relay::status]);
	(*rlys[1])[module::rly::sck::relayf::fwd].unbind((*incs[0])[module::inc::sck::incrementf::fwd]);
	comp["compare::fwd_1"].unbind((*incs[1])[module::inc::sck::incrementf::fwd]);
	comp["compare::fwd_0"].unbind((*rlys[1])[module::rly::sck::relayf::fwd]);
	comp["compare::fwd_2"].unbind(initializer[module::ini::sck::initialize::out]);
    finalizer1[module::fin::sck::finalize::in].unbind(comp["compare::fwd_2"]);
	finalizer2[module::fin::sck::finalize::in].unbind((*rlys[0])[module::rly::sck::relay::out]);
	finalizer2[module::fin::tsk::finalize].unbind(finalizer1[module::fin::sck::finalize::status]);

	return test_results;
}
