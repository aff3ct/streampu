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
	std::ifstream file1(filename1); //This is the source
	std::ifstream file2(filename2); //This is the sink
	std::istreambuf_iterator<char> begin1(file1);
	std::istreambuf_iterator<char> begin2(file2);
	int char_count=1;
	char expected, actual;
	while(!(begin1 == std::istreambuf_iterator<char>()) && !(begin2 == std::istreambuf_iterator<char>())) {
		actual = *begin2;
		if(std::isalpha(*begin1)) {
			expected = std::isupper(*begin1) ? std::tolower(*begin1) : std::toupper(*begin1);
			if(actual != expected){
				std::cout << "Expected : '" << expected << "' but got '" << actual << "' at char " << char_count << std::endl; 
				return false;
			}
		}

		begin2++;
		begin1++;
		char_count++;
	}

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
	size_t data_length = 8; //TODO: change pack, upcase and lowcase to support more than a single char at a time
	size_t buffer_size = 16;
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
		const int opt = getopt_long(argc, argv, "t:f:s:u:o:i:j:cpbgqwh", longopts, 0);
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
	if(force_sequence && n_threads > 1)
		std::clog << rang::tag::warning << "Sequence mode only supports a single thread (User-Source/Sinks are not clonable" << std::endl;

	std::function<uint8_t(uint8_t*)> pack = [](uint8_t* unpacked)
	{
		uint8_t res = 0;
		for (size_t i=0; i < 8; i++)
			res |= unpacked[i] << i;
		return res;
	};
	std::function<uint8_t*(uint8_t)> unpack = [](uint8_t packed)
	{
		uint8_t* res = new uint8_t[8]();
		for (size_t i=0; i < 8; i++)
			res[7-i] |= ((packed & (1 << (7 - i))) >> (7 - i)) ;
		return res;
	};

	// modules creation
	const bool auto_reset = false;
	module::Source_user_binary<uint8_t> source(data_length, in_filepath, auto_reset);
	module::Sink_user_binary<uint8_t> sink(data_length, "file.out");

	module::Relayer<uint8_t> relayer_com(data_length), relayer_sel(data_length);
	module::Switcher switcher(2, data_length, typeid(uint8_t));
	module::Stateless alternator, uppercaser, lowercaser;
	relayer_com.set_custom_name("Relayer Com");
	relayer_sel.set_custom_name("Relayer Sel");
	relayer_com.set_ns(sleep_time_us * 1000);
	relayer_sel.set_ns(sleep_time_us * 1000);

	// Stateless tasks
	alternator.create_task("alternate");
	auto s_in_alt = alternator.create_socket_in<uint8_t>(alternator("alternate"), "in", data_length);
	auto s_path = alternator.create_socket_out<int8_t>(alternator("alternate"), "path", 1);
	alternator.create_codelet(alternator("alternate"),
		[s_in_alt, s_path, &pack](module::Module &m, runtime::Task &t, const size_t frame_id)
	{
		uint8_t packed = pack((uint8_t*)(t[s_in_alt].get_dataptr()));
		if(packed >= 97 && packed <= 122) {
			*(uint8_t*)(t[s_path].get_dataptr()) = 0;
		}
		else {
			*(uint8_t*)(t[s_path].get_dataptr()) = 1;
		}
		return 0;
	});

	uppercaser.create_task("upcase");
	auto s_in_up = uppercaser.create_socket_in<uint8_t>(uppercaser("upcase"), "in", data_length);
	auto s_out_up = uppercaser.create_socket_out<uint8_t>(uppercaser("upcase"), "out", data_length);
	uppercaser.create_codelet(uppercaser("upcase"),
		[s_in_up, s_out_up, &pack, &unpack](module::Module &m, runtime::Task &t, const size_t frame_id)
	{
		*(long*)(t[s_out_up].get_dataptr()) = *(long*)(t[s_in_up].get_dataptr());
		uint8_t packed = pack((uint8_t*)(t[s_in_up].get_dataptr()));
		if(packed >= 97 && packed <= 122) {
			uint8_t* unpacked = unpack(packed-32);
			for(size_t i=0; i < 8; i++) {
				((uint8_t*)(t[s_out_up].get_dataptr()))[i] = unsigned(unpacked[i]); 	
			}
		}
		return 0;
	});

	lowercaser.create_task("lowcase");
	auto s_in_low = lowercaser.create_socket_in<uint8_t>(lowercaser("lowcase"), "in", data_length);
	auto s_out_low = lowercaser.create_socket_out<uint8_t>(lowercaser("lowcase"), "out", data_length);
	lowercaser.create_codelet(lowercaser("lowcase"),
		[s_in_low, s_out_low, &pack, &unpack](module::Module &m, runtime::Task &t, const size_t frame_id)
	{
		*(long*)(t[s_out_low].get_dataptr()) = *(long*)(t[s_in_low].get_dataptr());
		uint8_t packed = pack((uint8_t*)(t[s_in_low].get_dataptr()));
		if(packed >= 65 && packed <= 90) {	
			uint8_t* unpacked = unpack(packed+32);
			for(size_t i=0; i < 8; i++) {
				((uint8_t*)(t[s_out_low].get_dataptr()))[i] = unsigned(unpacked[i]); 	
			}
		} 
		return 0;
	});

	// sockets binding
	relayer_com[module::rly::sck::relay::in] = source[module::src::sck::generate::out_data];
	alternator("alternate")[s_in_alt] = source[module::src::sck::generate::out_data];

	switcher[module::swi::tsk::commute][0] = relayer_com[module::rly::sck::relay::out];
	switcher[module::swi::tsk::commute][1] = alternator("alternate")[s_path];
	uppercaser("upcase")[s_in_up] = switcher[module::swi::tsk::commute][2];
	lowercaser("lowcase")[s_in_low] = switcher[module::swi::tsk::commute][3];
	switcher[module::swi::tsk::select][0] = uppercaser("upcase")[s_out_up];
	switcher[module::swi::tsk::select][1] = lowercaser("lowcase")[s_out_low];

	relayer_sel[module::rly::sck::relay::in] = switcher[module::swi::tsk::select][2];
	sink[module::snk::sck::send_count::in_data] = relayer_sel[module::rly::sck::relay::out];
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
		                         { &relayer_com[module::rly::tsk::relay], &alternator("alternate") } }, // last  tasks of stage 0
		                       // pipeline stage 1
		                       { { &switcher[module::swi::tsk::commute] },   // first tasks of stage 1
		                         { &relayer_sel[module::rly::tsk::relay] } }, // last  tasks of stage 1
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
	auto theoretical_time = ((n_frames * 2 * sleep_time_us * 1000) * n_inter_frames) / 1000.f / 1000.f / n_threads;
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
	else 
	{
		pipeline_chain->set_n_frames(1);
		pipeline_chain->unbind_adaptors();
	}

	relayer_com[module::rly::sck::relay::in].unbind(source[module::src::sck::generate::out_data]);
	alternator("alternate")[s_in_alt].unbind(source[module::src::sck::generate::out_data]);

	switcher[module::swi::tsk::commute][0].unbind(relayer_com[module::rly::sck::relay::out]);
	switcher[module::swi::tsk::commute][1].unbind(alternator("alternate")[s_path]);
	uppercaser("upcase")[s_in_up].unbind(switcher[module::swi::tsk::commute][2]);
	lowercaser("lowcase")[s_in_low].unbind(switcher[module::swi::tsk::commute][3]);
	switcher[module::swi::tsk::select][0].unbind(uppercaser("upcase")[s_out_up]);
	switcher[module::swi::tsk::select][1].unbind(lowercaser("lowcase")[s_out_low]);

	relayer_sel[module::rly::sck::relay::in].unbind(switcher[module::swi::tsk::select][2]);
	sink[module::snk::sck::send_count::in_data].unbind(relayer_sel[module::rly::sck::relay::out]);
	sink[module::snk::sck::send_count::in_count].unbind(source[module::src::sck::generate::out_count]);
	return test_results;
}
