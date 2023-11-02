#include <iostream>
#include <cstdlib>
#include <memory>
#include <vector>
#include <string>
#include <atomic>
#include <getopt.h>

#include <aff3ct-core.hpp>
using namespace aff3ct;

int main(int argc, char** argv)
{
	option longopts[] = {
		{"n-threads", required_argument, NULL, 't'},
		{"n-inter-frames", required_argument, NULL, 'f'},
		{"sleep-time", required_argument, NULL, 's'},
		{"data-length", required_argument, NULL, 'd'},
		{"n-exec", required_argument, NULL, 'e'},
		{"n-loop-out", required_argument, NULL, 'i'},
		{"n-loop-in", required_argument, NULL, 'j'},
		{"dot-filepath", required_argument, NULL, 'o'},
		{"copy-mode", no_argument, NULL, 'c'},
		{"print-stats", no_argument, NULL, 'p'},
		{"step-by-step", no_argument, NULL, 'b'},
		{"debug", no_argument, NULL, 'g'},
		{"help", no_argument, NULL, 'h'},
		{0}};

	size_t n_threads = std::thread::hardware_concurrency();
	size_t n_inter_frames = 1;
	size_t sleep_time_us = 5;
	size_t data_length = 2048;
	size_t n_exec = 100000;
	size_t n_loop_out = 5;
	size_t n_loop_in = 2;
	std::string dot_filepath;
	bool no_copy_mode = true;
	bool print_stats = false;
	bool step_by_step = false;
	bool debug = false;
	bool force_sequence = false;

	while (1)
	{
		const int opt = getopt_long(argc, argv, "t:f:s:d:e:i:j:o:cpbgqh", longopts, 0);
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
			case 'i':
				n_loop_out = atoi(optarg);
				break;
			case 'j':
				n_loop_in = atoi(optarg);
				break;
			case 'o':
				dot_filepath = std::string(optarg);
				break;
			case 'c':
				no_copy_mode = false;
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
				std::cout << "  -i, --n-loop-out      "
				          << "Number of iterations to perform in the outer loop                     "
				          << "[" << n_loop_out << "]" << std::endl;
				std::cout << "  -j, --n-loop-in       "
				          << "Number of iterations to perform in the inner loop                     "
				          << "[" << n_loop_in << "]" << std::endl;
				std::cout << "  -o, --dot-filepath    "
				          << "Path to dot output file                                               "
				          << "[" << (dot_filepath.empty() ? "empty" : "\"" + dot_filepath + "\"") << "]" << std::endl;
				std::cout << "  -c, --copy-mode       "
				          << "Enable to copy data in sequence (performance will be reduced)         "
				          << "[" << (no_copy_mode ? "true" : "false") << "]" << std::endl;
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
				std::cout << "  -h, --help            "
				          << "This help                                                             "
				          << "[false]" << std::endl;
				exit(0);
				break;
			default:
				break;
		}
	}

	std::cout << "##########################################" << std::endl;
	std::cout << "# Micro-benchmark: Nested loops pipeline #" << std::endl;
	std::cout << "##########################################" << std::endl;
	std::cout << "#" << std::endl;

	std::cout << "# Command line arguments:" << std::endl;
	std::cout << "#   - n_threads      = " << n_threads << std::endl;
	std::cout << "#   - n_inter_frames = " << n_inter_frames << std::endl;
	std::cout << "#   - sleep_time_us  = " << sleep_time_us << std::endl;
	std::cout << "#   - data_length    = " << data_length << std::endl;
	std::cout << "#   - n_exec         = " << n_exec << std::endl;
	std::cout << "#   - n_loop_out     = " << n_loop_out << std::endl;
	std::cout << "#   - n_loop_in      = " << n_loop_in << std::endl;
	std::cout << "#   - dot_filepath   = " << (dot_filepath.empty() ? "[empty]" : dot_filepath.c_str()) << std::endl;
	std::cout << "#   - no_copy_mode   = " << (no_copy_mode ? "true" : "false") << std::endl;
	std::cout << "#   - print_stats    = " << (print_stats ? "true" : "false") << std::endl;
	std::cout << "#   - step_by_step   = " << (step_by_step ? "true" : "false") << std::endl;
	std::cout << "#   - debug          = " << (debug ? "true" : "false") << std::endl;
	std::cout << "#" << std::endl;

	module::Switcher switcher(2, data_length, typeid(uint8_t));
	switcher.set_custom_name("SwitcherIn");
	module::Iterator iterator(n_loop_in);
	iterator.set_custom_name("IteratorIn");

	module::Initializer<uint8_t> initializer(data_length);
	module::Finalizer  <uint8_t> finalizer  (data_length);

	std::vector<std::shared_ptr<module::Incrementer<uint8_t>>> incs(6);
	for (size_t s = 0; s < incs.size(); s++)
	{
		incs[s].reset(new module::Incrementer<uint8_t>(data_length));
		incs[s]->set_ns(sleep_time_us * 1000);
		incs[s]->set_custom_name("Inc" + std::to_string(s));
	}

	module::Switcher switcher2(2, data_length, typeid(uint8_t));
	switcher2.set_custom_name("SwitcherOut");
	module::Iterator iterator2(n_loop_out);
	iterator2.set_custom_name("IteratorOut");
	module::Relayer<uint8_t> relayer1(data_length);
	relayer1.set_custom_name("RelayerStart");
	module::Relayer<uint8_t> relayer2(data_length);
	relayer2.set_custom_name("RelayerEnd");

	relayer1  [module::rly::sck::relay::in]     = initializer[module::ini::sck::initialize::out];
	switcher2 [module::swi::tsk::select ][1]    = relayer1   [module::rly::sck::relay::out];
	iterator2 [module::ite::tsk::iterate]       = switcher2  [module::swi::tsk::select][3];
	switcher2 [module::swi::tsk::commute][0]    = switcher2  [module::swi::tsk::select][2];
	switcher2 [module::swi::tsk::commute][1]    = iterator2  [module::ite::sck::iterate::out];
	switcher  [module::swi::tsk::select ][1]    = switcher2  [module::swi::tsk::commute][2];
	iterator  [module::ite::tsk::iterate]       = switcher   [module::swi::tsk::select ][3];
	switcher  [module::swi::tsk::commute][0]    = switcher   [module::swi::tsk::select ][2];
	switcher  [module::swi::tsk::commute][1]    = iterator   [module::ite::sck::iterate::out];
	(*incs[0])[module::inc::sck::increment::in] = switcher   [module::swi::tsk::commute][2];
	for (size_t s = 0; s < incs.size() -1; s++)
		(*incs[s+1])[module::inc::sck::increment::in] = (*incs[s])[module::inc::sck::increment::out];
	switcher  [module::swi::tsk::select][0]     = (*incs[incs.size()-1])[module::inc::sck::increment::out];
	switcher2 [module::swi::tsk::select][0]     = switcher   [module::swi::tsk::commute][3];
	relayer2  [module::rly::sck::relay::in]     = switcher2  [module::swi::tsk::commute][3];
	finalizer [module::fin::sck::finalize::in]  = relayer2   [module::rly::sck::relay::out];

	std::unique_ptr<runtime::Sequence> sequence_nested_loops;
	std::unique_ptr<runtime::Pipeline> pipeline_chain;

	if (force_sequence)
	{
		sequence_nested_loops.reset(new runtime::Sequence(initializer[module::ini::tsk::initialize], n_threads));
		sequence_nested_loops->set_n_frames(n_inter_frames);
		sequence_nested_loops->set_no_copy_mode(no_copy_mode);

		for (auto cur_module : sequence_nested_loops->get_modules<tools::Interface_reset>())
			cur_module->reset();

		auto tid = 0;
		for (auto cur_initializer : sequence_nested_loops->get_cloned_modules<module::Initializer<uint8_t>>(initializer))
		{
			std::vector<std::vector<uint8_t>> init_data(n_inter_frames, std::vector<uint8_t>(data_length, 0));
			for (size_t f = 0; f < n_inter_frames; f++)
				std::fill(init_data[f].begin(), init_data[f].end(), tid * n_inter_frames +f);
			cur_initializer->set_init_data(init_data);
			tid++;
		}

		if (!dot_filepath.empty())
		{
			std::ofstream file(dot_filepath);
			sequence_nested_loops->export_dot(file);
		}

		// configuration of the sequence tasksexport_dot
		for (auto& mod : sequence_nested_loops->get_modules<module::Module>(false)) for (auto& tsk : mod->tasks)
		{
			tsk->reset          (           );
			tsk->set_debug      (debug      ); // disable the debug mode
			tsk->set_debug_limit(16         ); // display only the 16 first bits if the debug mode is enabled
			tsk->set_stats      (print_stats); // enable the statistics
			tsk->set_fast       (true       ); // enable the fast mode (= disable the useless verifs in the tasks)
		}

		auto t_start = std::chrono::steady_clock::now();
		std::atomic<unsigned int> counter(0);
		if (!step_by_step)
		{
			// execute the sequence (multi-threaded)
			sequence_nested_loops->exec([&counter, n_exec]() { return ++counter >= n_exec; });
		}
		else
		{
			do
				for (size_t tid = 0; tid < n_threads; tid++)
					while (sequence_nested_loops->exec_step(tid));
			while (++counter < (n_exec / n_threads));
		}
		std::chrono::nanoseconds duration = std::chrono::steady_clock::now() - t_start;

		auto elapsed_time = duration.count() / 1000.f / 1000.f;
		std::cout << "Sequence elapsed time: " << elapsed_time << " ms" << std::endl;
	}
	else
	{
		pipeline_chain.reset(new runtime::Pipeline(
		                     initializer[module::ini::tsk::initialize], // first task of the sequence
		                     { // pipeline stage 0
		                       { { &initializer[module::ini::tsk::initialize] },   // first tasks of stage 0
		                         { &initializer[module::ini::tsk::initialize] } }, // last  tasks of stage 0
		                       // pipeline stage 1
		                       { { &relayer1[module::rly::tsk::relay] },   // first tasks of stage 1
		                         { &relayer2[module::rly::tsk::relay] } }, // last  tasks of stage 1
		                       // pipeline stage 2
		                       { { &finalizer[module::fin::tsk::finalize] },   // first tasks of stage 2
		                         {                                     } }, // last  tasks of stage 2
		                     },
		                     {
		                       1,                         // number of threads in the stage 0
		                       n_threads ? n_threads : 1, // number of threads in the stage 1
		                       1                          // number of threads in the stage 2
		                     },
		                     {
		                       2, // synchronization buffer size between stages 0 and 1
		                       2, // synchronization buffer size between stages 1 and 2
		                     },
		                     {
		                       false, // type of waiting between stages 0 and 1 (true = active, false = passive)
		                       false, // type of waiting between stages 1 and 2 (true = active, false = passive)
		                     }));
		pipeline_chain->set_n_frames(n_inter_frames);
		auto tid = 0;
		for (auto cur_initializer : pipeline_chain->get_stages()[0]->get_cloned_modules<module::Initializer<uint8_t>>(initializer))
		{
			std::vector<std::vector<uint8_t>> init_data(n_inter_frames, std::vector<uint8_t>(data_length, 0));
			for (size_t f = 0; f < n_inter_frames; f++)
				std::fill(init_data[f].begin(), init_data[f].end(), tid * n_inter_frames +f);
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
		std::atomic<unsigned int> counter(0);
		auto t_start = std::chrono::steady_clock::now();
		pipeline_chain->exec([&counter, n_exec]() { return ++counter >= n_exec; });
		std::chrono::nanoseconds duration = std::chrono::steady_clock::now() - t_start;

		auto elapsed_time = duration.count() / 1000.f / 1000.f;
		std::cout << "Sequence elapsed time: " << elapsed_time << " ms" << std::endl;
	}

	// verification of the sequence execution
	auto tests_passed = true;
	auto tid = 0;
	if(force_sequence) {
		for (auto cur_finalizer : sequence_nested_loops->get_cloned_modules<module::Finalizer<uint8_t>>(finalizer))
		{
			for (size_t f = 0; f < n_inter_frames; f++)
			{
				const auto &final_data = cur_finalizer->get_final_data()[f];
				for (size_t d = 0; d < final_data.size(); d++)
				{
					auto expected = (int)(incs.size() * iterator.get_limit() * iterator2.get_limit() + (tid * n_inter_frames +f));
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
	}
	else {
		for (auto cur_finalizer : pipeline_chain->get_stages()[2]->get_cloned_modules<module::Finalizer<uint8_t>>(finalizer))
		{
			for (size_t f = 0; f < n_inter_frames; f++)
			{
				const auto &final_data = cur_finalizer->get_final_data()[f];
				for (size_t d = 0; d < final_data.size(); d++)
				{
					auto expected = (int)(incs.size() * iterator.get_limit() * iterator2.get_limit() + (tid * n_inter_frames +f));
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
	}

	if (tests_passed)
		std::cout << "# " << rang::style::bold << rang::fg::green << "Tests passed!" << rang::style::reset << std::endl;
	else
		std::cout << "# " << rang::style::bold << rang::fg::red << "Tests failed :-(" << rang::style::reset << std::endl;

	unsigned int test_results = !tests_passed;

	// display the statistics of the tasks (if enabled)
	if (print_stats)
	{
		std::cout << "#" << std::endl;
		if (force_sequence)
			tools::Stats::show(sequence_nested_loops->get_modules_per_types(), true, false);
		else
		{
			auto stages = pipeline_chain->get_stages();
			for (size_t s = 0; s < stages.size(); s++)
			{
				const int n_threads = stages[s]->get_n_threads();
				std::cout << "#" << std::endl << "# Pipeline stage " << s << " (" << n_threads << " thread(s)): " << std::endl;
				tools::Stats::show(stages[s]->get_tasks_per_types(), true, false);
			}
		}
	}

	if (force_sequence)
		sequence_nested_loops->set_n_frames(1);
	else {
		pipeline_chain->set_n_frames(1);
		pipeline_chain->unbind_adaptors();
	}
	switcher2 [module::swi::tsk::select ][1]   .unbind(relayer1   [module::rly::sck::relay::out]);
	relayer1  [module::rly::sck::relay::in]    .unbind(initializer[module::ini::sck::initialize::out]);
	iterator2 [module::ite::tsk::iterate]      .unbind(switcher2  [module::swi::tsk::select][3]);
	switcher2 [module::swi::tsk::commute][0]   .unbind(switcher2  [module::swi::tsk::select][2]);
	switcher2 [module::swi::tsk::commute][1]   .unbind(iterator2  [module::ite::sck::iterate::out]);
	switcher  [module::swi::tsk::select ][1]   .unbind(switcher2  [module::swi::tsk::commute][2]);
	iterator  [module::ite::tsk::iterate]      .unbind(switcher   [module::swi::tsk::select ][3]);
	switcher  [module::swi::tsk::commute][0]   .unbind(switcher   [module::swi::tsk::select ][2]);
	switcher  [module::swi::tsk::commute][1]   .unbind(iterator   [module::ite::sck::iterate::out]);
	(*incs[0])[module::inc::sck::increment::in].unbind(switcher   [module::swi::tsk::commute][2]);
	for (size_t s = 0; s < incs.size() -1; s++)
		(*incs[s+1])[module::inc::sck::increment::in].unbind((*incs[s])[module::inc::sck::increment::out]);
	switcher  [module::swi::tsk::select][0]    .unbind((*incs[incs.size()-1])[module::inc::sck::increment::out]);
	switcher2 [module::swi::tsk::select][0]    .unbind(switcher   [module::swi::tsk::commute][3]);
	relayer2  [module::rly::sck::relay::in]    .unbind(switcher2  [module::swi::tsk::commute][3]);
	finalizer [module::fin::sck::finalize::in] .unbind(relayer2   [module::rly::sck::relay::out]);

	return test_results;
}
