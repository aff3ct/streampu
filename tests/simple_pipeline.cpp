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

std::ifstream::pos_type filesize(const char* filename)
{
	std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
	return in.tellg();
}

bool compare_files(const std::string& filename1, const std::string& filename2)
{
	std::ifstream file1(filename1, std::ifstream::ate | std::ifstream::binary); //open file at the end
	std::ifstream file2(filename2, std::ifstream::ate | std::ifstream::binary); //open file at the end
	const std::ifstream::pos_type fileSize = file1.tellg();

	if (fileSize != file2.tellg())
		return false; //different file size

	file1.seekg(0); //rewind
	file2.seekg(0); //rewind

	std::istreambuf_iterator<char> begin1(file1);
	std::istreambuf_iterator<char> begin2(file2);

	return std::equal(begin1, std::istreambuf_iterator<char>(), begin2); //Second argument is end-of-range iterator
}

int main(int argc, char** argv)
{
	option longopts[] = {
		{"n-threads", required_argument, NULL, 't'},
		{"n-inter-frames", required_argument, NULL, 'f'},
		{"sleep-time", required_argument, NULL, 's'},
		{"data-length", required_argument, NULL, 'd'},
		{"buffer-size", required_argument, NULL, 'u'},
		{"dot-filepath", required_argument, NULL, 'o'},
		{"in-filepath", required_argument, NULL, 'i'},
		{"out-filepath", required_argument, NULL, 'j'},
		{"copy-mode", no_argument, NULL, 'c'},
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
	size_t data_length =512;
	size_t buffer_size =5;
	std::string dot_filepath;
	std::string in_filepath;
	std::string out_filepath = "file.out";
	bool no_copy_mode = true;
	bool print_stats = false;
	bool step_by_step = false;
	bool debug = false;
	bool force_sequence = false;
	bool active_waiting = false;

	while (1)
	{
		const int opt = getopt_long(argc, argv, "t:f:s:d:u:o:i:j:cpbgqwh", longopts, 0);
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
			case 'u':
				buffer_size = atoi(optarg);
				break;
			case 'o':
				dot_filepath = std::string(optarg);
				break;
			case 'i':
				in_filepath = std::string(optarg);
				break;
			case 'j':
				out_filepath = std::string(optarg);
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
				std::cout << "  -u, --buffer-size     "
				          << "Size of the buffer between the different stages of the pipeline       "
				          << "[" << data_length << "]" << std::endl;
				std::cout << "  -o, --dot-filepath    "
				          << "Path to dot output file                                               "
				          << "[" << (dot_filepath.empty() ? "empty" : "\"" + dot_filepath + "\"") << "]" << std::endl;
				std::cout << "  -i, --in-filepath     "
				          << "Path to the input file (used to generate bits of the chain)           "
				          << "[" << (in_filepath.empty() ? "empty" : "\"" + in_filepath + "\"") << "]" << std::endl;
				std::cout << "  -j, --out-filepath    "
				          << "Path to the output file (written at the end of the chain)             "
				          << "[" << (out_filepath.empty() ? "empty" : "\"" + out_filepath + "\"") << "]" << std::endl;
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

	std::cout << "####################################" << std::endl;
	std::cout << "# Micro-benchmark: Simple pipeline #" << std::endl;
	std::cout << "####################################" << std::endl;
	std::cout << "#" << std::endl;

	std::cout << "# Command line arguments:" << std::endl;
	std::cout << "#   - n_threads      = " << n_threads << std::endl;
	std::cout << "#   - n_inter_frames = " << n_inter_frames << std::endl;
	std::cout << "#   - sleep_time_us  = " << sleep_time_us << std::endl;
	std::cout << "#   - data_length    = " << data_length << std::endl;
	std::cout << "#   - buffer_size    = " << buffer_size << std::endl;
	std::cout << "#   - dot_filepath   = " << (dot_filepath.empty() ? "[empty]" : dot_filepath.c_str()) << std::endl;
	std::cout << "#   - in_filepath    = " << (in_filepath.empty() ? "[empty]" : in_filepath.c_str()) << std::endl;
	std::cout << "#   - out_filepath   = " << (out_filepath.empty() ? "[empty]" : out_filepath.c_str()) << std::endl;
	std::cout << "#   - no_copy_mode   = " << (no_copy_mode ? "true" : "false") << std::endl;
	std::cout << "#   - print_stats    = " << (print_stats ? "true" : "false") << std::endl;
	std::cout << "#   - step_by_step   = " << (step_by_step ? "true" : "false") << std::endl;
	std::cout << "#   - debug          = " << (debug ? "true" : "false") << std::endl;
	std::cout << "#   - force_sequence = " << (force_sequence ? "true" : "false") << std::endl;
	std::cout << "#   - active_waiting = " << (active_waiting ? "true" : "false") << std::endl;
	std::cout << "#" << std::endl;

	if (!force_sequence && !no_copy_mode)
		std::clog << rang::tag::warning << "'no_copy_mode' has no effect with pipeline (it is always enable)" << std::endl;
	if (!force_sequence && step_by_step)
		std::clog << rang::tag::warning << "'step_by_step' is not available with pipeline" << std::endl;

	// modules creation
	const bool auto_reset = false;
	module::Source_user_binary<uint8_t> source(data_length, in_filepath, auto_reset);
	module::Sink_user_binary<uint8_t> sink(data_length, out_filepath);

	std::vector<std::shared_ptr<module::Relayer<uint8_t>>> rlys(5);
	for (size_t s = 0; s < rlys.size(); s++)
	{
		rlys[s].reset(new module::Relayer<uint8_t>(data_length));
		rlys[s]->set_ns(sleep_time_us * 1000);
		rlys[s]->set_custom_name("Relayer" + std::to_string(s));
	}

	// sockets binding
	(*rlys[0])[module::rly::sck::relay::in] = source[module::src::sck::generate::out_data];
	for (size_t s = 0; s < rlys.size() -1; s++)
		(*rlys[s+1])[module::rly::sck::relay::in] = (*rlys[s])[module::rly::sck::relay::out];
	sink[module::snk::sck::send_count::in_data] = (*rlys[rlys.size()-1])[module::rly::sck::relay::out];
	sink[module::snk::sck::send_count::in_count] = source[module::src::sck::generate::out_count];

	std::unique_ptr<runtime::Sequence> sequence_chain;
	std::unique_ptr<runtime::Pipeline> pipeline_chain;
	if (force_sequence)
	{
		sequence_chain.reset(new runtime::Sequence(source[module::src::tsk::generate], n_threads));
		sequence_chain->set_n_frames(n_inter_frames);
		sequence_chain->set_no_copy_mode(no_copy_mode);

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

		auto t_start = std::chrono::steady_clock::now();
		if (!step_by_step)
			sequence_chain->exec();
		else
		{
			do
			{
				try
				{
					for (size_t tid = 0; tid < n_threads; tid++)
						while (sequence_chain->exec_step(tid));
				}
				catch (tools::processing_aborted &) { /* do nothing */ }
			}
			while (!source.is_done());
		}
		std::chrono::nanoseconds duration = std::chrono::steady_clock::now() - t_start;

		auto elapsed_time = duration.count() / 1000.f / 1000.f;
		std::cout << "Sequence elapsed time: " << elapsed_time << " ms" << std::endl;
	}
	else
	{
		pipeline_chain.reset(new runtime::Pipeline(
		                     source[module::src::tsk::generate], // first task of the sequence
		                     { // pipeline stage 0
		                       { { &source[module::src::tsk::generate] },   // first tasks of stage 0
		                         { &source[module::src::tsk::generate] } }, // last  tasks of stage 0
		                       // pipeline stage 1
		                       { { &(*rlys[             0])[module::rly::tsk::relay] },   // first tasks of stage 1
		                         { &(*rlys[rlys.size() -1])[module::rly::tsk::relay] } }, // last  tasks of stage 1
		                       // pipeline stage 2
		                       { { &sink[module::snk::tsk::send_count] },   // first tasks of stage 2
		                         {                                     } }, // last  tasks of stage 2
		                     },
		                     {
		                       1,                         // number of threads in the stage 0
		                       n_threads ? n_threads : 1, // number of threads in the stage 1
		                       1                          // number of threads in the stage 2
		                     },
		                     {
		                       buffer_size, // synchronization buffer size between stages 0 and 1
		                       buffer_size, // synchronization buffer size between stages 1 and 2
		                     },
		                     {
		                       active_waiting, // type of waiting between stages 0 and 1 (true = active, false = passive)
		                       active_waiting, // type of waiting between stages 1 and 2 (true = active, false = passive)
		                     }));
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
		pipeline_chain->exec();
		std::chrono::nanoseconds duration = std::chrono::steady_clock::now() - t_start;

		auto elapsed_time = duration.count() / 1000.f / 1000.f;
		std::cout << "Sequence elapsed time: " << elapsed_time << " ms" << std::endl;
	}

	size_t in_filesize = filesize(in_filepath.c_str());
	size_t n_frames = ((int)std::ceil((float)(in_filesize * 8) / (float)(data_length * n_inter_frames)));
	auto theoretical_time = (n_frames * (rlys.size() * sleep_time_us * 1000) * n_inter_frames) / 1000.f / 1000.f / n_threads;
	std::cout << "Sequence theoretical time: " << theoretical_time << " ms" << std::endl;

	// verification of the sequence execution
	bool tests_passed = compare_files(in_filepath, out_filepath);
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
			tools::Stats::show(sequence_chain->get_modules_per_types(), true, false);
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

	// sockets unbinding
	if (force_sequence)
		sequence_chain->set_n_frames(1);
	else {
		pipeline_chain->set_n_frames(1);
		pipeline_chain->unbind_adaptors();
	}
	(*rlys[0])[module::rly::sck::relay::in].unbind(source[module::src::sck::generate::out_data]);
	for (size_t s = 0; s < rlys.size() -1; s++)
		(*rlys[s+1])[module::rly::sck::relay::in].unbind((*rlys[s])[module::rly::sck::relay::out]);
	sink[module::snk::sck::send_count::in_data].unbind((*rlys[rlys.size()-1])[module::rly::sck::relay::out]);
	sink[module::snk::sck::send_count::in_count].unbind(source[module::src::sck::generate::out_count]);

	return test_results;
}
