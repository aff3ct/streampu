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
	tools::Signal_handler::init();

	option longopts[] = {
		{"n-threads", required_argument, NULL, 't'},
		{"n-inter-frames", required_argument, NULL, 'f'},
		{"sleep-time", required_argument, NULL, 's'},
		{"data-length", required_argument, NULL, 'd'},
		{"buffer-size", required_argument, NULL, 'u'},
		{"dot-filepath", required_argument, NULL, 'o'},
		{"in-filepath", required_argument, NULL, 'i'},
		{"out-filepath", required_argument, NULL, 'j'},
		{"probes-filepath", required_argument, NULL, 'a'},
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
	size_t data_length = 2048;
	size_t buffer_size = 16;
	std::string dot_filepath;
	std::string in_filepath;
	std::string out_filepath = "file.out";
	std::string probes_filepath = "probes.out";
	bool no_copy_mode = true;
	bool print_stats = false;
	bool step_by_step = false;
	bool debug = false;
	bool force_sequence = false;
	bool active_waiting = false;

	while (1)
	{
		const int opt = getopt_long(argc, argv, "t:f:s:d:u:o:i:j:a:cpbgqwh", longopts, 0);
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
			case 'a':
				probes_filepath = std::string(optarg);
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
				std::cout << "  -a, --probes-filepath "
				          << "Path to the output probe file (written during the execution)          "
				          << "[" << (out_filepath.empty() ? "empty" : "\"" + out_filepath + "\"") << "]" << std::endl;
				std::cout << "  -c, --copy-mode       "
				          << "Enable to copy data in sequence (performance will be reduced)         "
				          << "[" << (no_copy_mode ? "false" : "true") << "]" << std::endl;
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

	std::cout << "###################################" << std::endl;
	std::cout << "# Micro-benchmark: Pipeline Probe #" << std::endl;
	std::cout << "###################################" << std::endl;
	std::cout << "#" << std::endl;

	std::cout << "# Command line arguments:" << std::endl;
	std::cout << "#   - n_threads       = " << n_threads << std::endl;
	std::cout << "#   - n_inter_frames  = " << n_inter_frames << std::endl;
	std::cout << "#   - sleep_time_us   = " << sleep_time_us << std::endl;
	std::cout << "#   - data_length     = " << data_length << std::endl;
	std::cout << "#   - buffer_size     = " << buffer_size << std::endl;
	std::cout << "#   - dot_filepath    = " << (dot_filepath.empty() ? "[empty]" : dot_filepath.c_str()) << std::endl;
	std::cout << "#   - in_filepath     = " << (in_filepath.empty() ? "[empty]" : in_filepath.c_str()) << std::endl;
	std::cout << "#   - out_filepath    = " << (out_filepath.empty() ? "[empty]" : out_filepath.c_str()) << std::endl;
	std::cout << "#   - probes_filepath = " << (probes_filepath.empty() ? "[empty]" : probes_filepath.c_str()) << std::endl;
	std::cout << "#   - no_copy_mode    = " << (no_copy_mode ? "true" : "false") << std::endl;
	std::cout << "#   - print_stats     = " << (print_stats ? "true" : "false") << std::endl;
	std::cout << "#   - step_by_step    = " << (step_by_step ? "true" : "false") << std::endl;
	std::cout << "#   - debug           = " << (debug ? "true" : "false") << std::endl;
	std::cout << "#   - force_sequence  = " << (force_sequence ? "true" : "false") << std::endl;
	std::cout << "#   - active_waiting  = " << (active_waiting ? "true" : "false") << std::endl;
	std::cout << "#" << std::endl;

	if (!force_sequence && !no_copy_mode)
		std::clog << rang::tag::warning << "'no_copy_mode' has no effect with pipeline (it is always enable)"
		                                << std::endl;
	if (!force_sequence && step_by_step)
		std::clog << rang::tag::warning << "'step_by_step' is not available with pipeline" << std::endl;
	if (force_sequence && n_threads > 1)
		std::clog << rang::tag::warning << "Sequence mode only supports a single thread (User-Source/Sinks are not "
		                                << "clonable)" << std::endl;

	// create reporters and probes for the real-time probes file
	tools::Reporter_probe rep_fra_stats("Counters");
	module::Probe_occurrence prb_fra_fid ("FRAME_ID" );
	module::Probe_stream     prb_fra_sid ("STREAM_ID");
	rep_fra_stats.register_probes({ &prb_fra_fid, &prb_fra_sid });
	prb_fra_sid.set_col_size(11);
	prb_fra_fid.set_custom_name("Probe<FID>");
	prb_fra_sid.set_custom_name("Probe<SID>");
	rep_fra_stats.set_cols_buff_size(200);

	tools::Reporter_probe rep_thr_stats("Throughput, latency", "and time");
	module::Probe_throughput prb_thr_thr ("FPS" ); // only valid for sequence, invalid for pipeline
	module::Probe_latency    prb_thr_lat ("LAT" ); // only valid for sequence, invalid for pipeline
	module::Probe_time       prb_thr_time("TIME"); // only valid for sequence, invalid for pipeline
	rep_thr_stats.register_probes({ &prb_thr_thr, &prb_thr_lat, &prb_thr_time});
	prb_thr_thr.set_col_size(12);
	prb_thr_time.set_col_prec(6);
	prb_thr_time.set_col_size(12);
	rep_thr_stats.set_cols_buff_size(200);

	tools::Reporter_probe rep_timestamp_stats("Timestamps", "(in microseconds) [SX = stage X, B = begin, E = end]");
	const uint64_t mod = 6000000ul * 60ul * 10; // limit to 60 minutes timestamp
	module::Probe_timestamp       prb_ts_s1b(mod, "S1_B"); // timestamp stage 1 begin
	module::Probe_timestamp       prb_ts_s1e(mod, "S1_E"); // timestamp stage 1 end
	module::Probe_value<uint64_t> prb_ts_s2b(1,   "S2_B"); // timestamp stage 2 begin
	module::Probe_value<uint64_t> prb_ts_s2e(1,   "S2_E"); // timestamp stage 2 end
	module::Probe_timestamp       prb_ts_s3b(mod, "S3_B"); // timestamp stage 3 begin
	module::Probe_timestamp       prb_ts_s3e(mod, "S3_E"); // timestamp stage 3 end
	rep_timestamp_stats.register_probes({ &prb_ts_s1b, &prb_ts_s1e, &prb_ts_s2b, &prb_ts_s2e, &prb_ts_s3b,
	                                      &prb_ts_s3e });
	rep_timestamp_stats.set_cols_buff_size(200); // size of the buffer used by the probes to record values
	rep_timestamp_stats.set_cols_unit("(us)");
	rep_timestamp_stats.set_cols_size(12);

	tools::Reporter_probe rep_bitvals("Read bits", "(Unpacked)");
	module::Probe_value<uint32_t> prb_bitvals_cnt (1,           "Count");
	module::Probe_value<uint8_t>  prb_bitvals_data(data_length, "Bits" );
	rep_bitvals.register_probes({ &prb_bitvals_cnt, &prb_bitvals_data });
	rep_bitvals.set_cols_buff_size(200);
	prb_bitvals_cnt.set_col_size(7);
	prb_bitvals_data.set_col_size(data_length * 4 + 2);
	prb_bitvals_data.set_col_unit("(low-order bit on the left)");

	const std::vector<tools::Reporter*>& reporters = { &rep_fra_stats, &rep_thr_stats, &rep_timestamp_stats,
	                                                   &rep_bitvals };
	tools::Terminal_dump terminal_probes(reporters);

	std::ofstream probes_file;
	if (!probes_filepath.empty())
	{
		probes_file.open(probes_filepath);
		terminal_probes.legend(probes_file);
	}

	// create on-the-fly stateless modules to collect timestamps in the stage 2 (parallel) of the pipeline
	std::unique_ptr<aff3ct::module::Stateless> ts_s2b(new aff3ct::module::Stateless());
	ts_s2b->set_name("Timestamper");
	ts_s2b->set_short_name("Timestamper");
	auto &tsk = ts_s2b->create_task("exec");
	auto ts_out_val = ts_s2b->create_socket_out<uint64_t>(tsk, "out", 1);
	ts_s2b->create_codelet(tsk, [ts_out_val]
		(aff3ct::module::Module &m, aff3ct::runtime::Task &t, const size_t frame_id) -> int
	{
		std::chrono::microseconds us = std::chrono::duration_cast<std::chrono::microseconds>(
			std::chrono::steady_clock::now().time_since_epoch());
		(*t[ts_out_val].get_dataptr<uint64_t>()) = mod ? (uint64_t)us.count() % mod : (uint64_t)us.count();
		return aff3ct::runtime::status_t::SUCCESS;
	});
	std::unique_ptr<aff3ct::module::Stateless> ts_s2e(ts_s2b->clone());
	ts_s2b->set_custom_name("Tsta<S2_B>");
	ts_s2e->set_custom_name("Tsta<S2_E>");

	// modules creation
	const bool auto_reset = false;
	module::Source_user_binary<uint8_t> source(data_length, in_filepath, auto_reset);
	module::Sink_user_binary<uint8_t> sink(data_length, out_filepath);

	std::vector<std::shared_ptr<module::Relayer<uint8_t>>> rlys(6);
	for (size_t s = 0; s < rlys.size(); s++)
	{
		rlys[s].reset(new module::Relayer<uint8_t>(data_length));
		rlys[s]->set_ns(sleep_time_us * 1000);
		rlys[s]->set_custom_name("Relayer" + std::to_string(s));
	}

	// sockets binding
	// stage 0 -------------------------------------------------------------------------------
	prb_fra_fid     (     "probe"          ) = prb_ts_s1b            (     "probe"           );
	prb_fra_sid     (     "probe"          ) = prb_fra_fid           (     "probe"           );
	source          (  "generate"          ) = prb_fra_sid           (     "probe"           );
	prb_ts_s1e      (     "probe"          ) = source                (  "generate"           );
	// stage 1 -------------------------------------------------------------------------------
	(*ts_s2b)       (      "exec"          ) = prb_ts_s1e            (     "probe"           );
	(*rlys[0])      (     "relay"          ) = (*ts_s2b)             (      "exec"           );
	(*rlys[0])      [     "relay::in"      ] = source                [  "generate::out_data" ];
	for (size_t s = 0; s < rlys.size() -1; s++)
		(*rlys[s+1])[     "relay::in"      ] = (*rlys[s])            [     "relay::out"      ];
	(*ts_s2e)       (      "exec"          ) = (*rlys[rlys.size()-1])(     "relay"           );
	// stage 2 -------------------------------------------------------------------------------
	prb_ts_s3b      (     "probe"          ) = (*ts_s2e)             (      "exec"           );
	sink            ("send_count"          ) = prb_ts_s3b            (     "probe"           );
	sink            ["send_count::in_data" ] = (*rlys[rlys.size()-1])[     "relay::out"      ];
	sink            ["send_count::in_count"] = source                [  "generate::out_count"];
	prb_bitvals_cnt (     "probe"          ) = sink                  ("send_count"           );
	prb_bitvals_data(     "probe"          ) = sink                  ("send_count"           );
	prb_bitvals_cnt [     "probe::in"      ] = source                [  "generate::out_count"];
	prb_bitvals_data[     "probe::in"      ] = (*rlys[rlys.size()-1])[     "relay::out"      ];
	prb_thr_thr     (     "probe"          ) = sink                  ("send_count"           );
	prb_thr_lat     (     "probe"          ) = prb_thr_thr           (     "probe"           );
	prb_thr_time    (     "probe"          ) = prb_thr_lat           (     "probe"           );
	prb_ts_s2b      (     "probe"          ) = prb_thr_time          (     "probe"           );
	prb_ts_s2b      [     "probe::in"      ] = (*ts_s2b)             [      "exec::out"      ];
	prb_ts_s2e      (     "probe"          ) = prb_ts_s2b            (     "probe"           );
	prb_ts_s2e      [     "probe::in"      ] = (*ts_s2e)             [      "exec::out"      ];
	prb_ts_s3e      (     "probe"          ) = prb_ts_s2e            (     "probe"           );

	// stop condition that write the probes data into a file
	std::function<bool()> stop_condition = [&terminal_probes, &probes_file] ()
	{
		if (probes_file.is_open())
			terminal_probes.temp_report(probes_file);
		return false;
	};

	std::unique_ptr<runtime::Sequence> sequence_chain;
	std::unique_ptr<runtime::Pipeline> pipeline_chain;
	if (force_sequence)
	{
		sequence_chain.reset(new runtime::Sequence(prb_ts_s1b("probe"), n_threads));
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

		// reset the probes (to initialize the timers)
		for (auto& prb : sequence_chain->get_modules<module::AProbe>(false))
			prb->reset();

		auto t_start = std::chrono::steady_clock::now();
		if (!step_by_step)
			sequence_chain->exec(stop_condition);
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
			while (!stop_condition() && !source.is_done());
		}
		std::chrono::nanoseconds duration = std::chrono::steady_clock::now() - t_start;

		auto elapsed_time = duration.count() / 1000.f / 1000.f;
		std::cout << "Sequence elapsed time: " << elapsed_time << " ms" << std::endl;
	}
	else
	{
		pipeline_chain.reset(new runtime::Pipeline(
		                     { &prb_ts_s1b("probe") }, // first task of the sequence
		                     { // pipeline stage 0: first & last tasks
		                       { { &prb_ts_s1b("probe"), &prb_ts_s1e("probe") },
		                         { &source("generate"), &prb_ts_s1e("probe") },
		                         { }, },
		                       // pipeline stage 1: first & last tasks
		                       { { &(*ts_s2b)("exec"), &(*rlys[0])("relay"), },
		                         { &(*ts_s2e)("exec") } ,
		                         { &prb_ts_s3b("probe"), &sink("send_count"), &prb_ts_s2b("probe"),
		                           &prb_ts_s2e("probe") }, },
		                       // pipeline stage 2: first task (no need to specify the last taks)
		                       { { &prb_ts_s3b("probe"), &sink("send_count"), &prb_bitvals_cnt("probe"),
		                           &prb_bitvals_data("probe"), &prb_ts_s2b("probe"), &prb_ts_s2e("probe") },
		                         { },
		                         { }, },
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

		// reset the probes (to initialize the timers)
		for (auto& prb : pipeline_chain->get_modules<module::AProbe>(false))
			prb->reset();

		auto t_start = std::chrono::steady_clock::now();
		pipeline_chain->exec(stop_condition);
		std::chrono::nanoseconds duration = std::chrono::steady_clock::now() - t_start;

		auto elapsed_time = duration.count() / 1000.f / 1000.f;
		std::cout << "Sequence elapsed time: " << elapsed_time << " ms" << std::endl;
	}

	if (probes_file.is_open())
		terminal_probes.final_report(probes_file);

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
	else
	{
		pipeline_chain->set_n_frames(1);
		pipeline_chain->unbind_adaptors();
	}

	// stage 0 ---------------------------------------------------------------------------------------------------------------------
	prb_fra_fid     [module::prb::tsk::probe               ].unbind(prb_ts_s1b            [module::prb::tsk::probe                ]);
	prb_fra_sid     [module::prb::tsk::probe               ].unbind(prb_fra_fid           [module::prb::tsk::probe                ]);
	source          [module::src::tsk::generate            ].unbind(prb_fra_sid           [module::prb::tsk::probe                ]);
	prb_ts_s1e      [module::prb::tsk::probe               ].unbind(source                [module::src::tsk::generate             ]);
	// stage 1 ---------------------------------------------------------------------------------------------------------------------
	(*ts_s2b)       (                 "exec"               ).unbind(prb_ts_s1e            [module::prb::tsk::probe                ]);
	(*rlys[0])      [module::rly::tsk::relay               ].unbind((*ts_s2b)             (                 "exec"                ));
	(*rlys[0])      [module::rly::sck::relay     ::in      ].unbind(source                [module::src::sck::generate  ::out_data ]);
	for (size_t s = 0; s < rlys.size() -1; s++)
		(*rlys[s+1])[module::rly::sck::relay     ::in      ].unbind((*rlys[s])            [module::rly::sck::relay     ::out      ]);
	(*ts_s2e)       (                 "exec"               ).unbind((*rlys[rlys.size()-1])[module::rly::tsk::relay                ]);
	// stage 2 ---------------------------------------------------------------------------------------------------------------------
	prb_ts_s3b      [module::prb::tsk::probe               ].unbind((*ts_s2e)             (                 "exec"                ));
	sink            [module::snk::tsk::send_count          ].unbind(prb_ts_s3b            [module::prb::tsk::probe                ]);
	sink            [module::snk::sck::send_count::in_data ].unbind((*rlys[rlys.size()-1])[module::rly::sck::relay     ::out      ]);
	sink            [module::snk::sck::send_count::in_count].unbind(source                [module::src::sck::generate  ::out_count]);
	prb_bitvals_cnt [module::prb::tsk::probe               ].unbind(sink                  [module::snk::tsk::send_count           ]);
	prb_bitvals_data[module::prb::tsk::probe               ].unbind(sink                  [module::snk::tsk::send_count           ]);
	prb_bitvals_cnt [module::prb::sck::probe     ::in      ].unbind(source                [module::src::sck::generate  ::out_count]);
	prb_bitvals_data[module::prb::sck::probe     ::in      ].unbind((*rlys[rlys.size()-1])[module::rly::sck::relay     ::out      ]);
	prb_thr_thr     [module::prb::tsk::probe               ].unbind(sink                  [module::snk::tsk::send_count           ]);
	prb_thr_lat     [module::prb::tsk::probe               ].unbind(prb_thr_thr           [module::prb::tsk::probe                ]);
	prb_thr_time    [module::prb::tsk::probe               ].unbind(prb_thr_lat           [module::prb::tsk::probe                ]);
	prb_ts_s2b      [module::prb::tsk::probe               ].unbind(prb_thr_time          [module::prb::tsk::probe                ]);
	prb_ts_s2b      [module::prb::sck::probe     ::in      ].unbind((*ts_s2b)             [                 "exec      ::out"     ]);
	prb_ts_s2e      [module::prb::tsk::probe               ].unbind(prb_ts_s2b            [module::prb::tsk::probe                ]);
	prb_ts_s2e      [module::prb::sck::probe     ::in      ].unbind((*ts_s2e)             [                 "exec      ::out"     ]);
	prb_ts_s3e      [module::prb::tsk::probe               ].unbind(prb_ts_s2e            [module::prb::tsk::probe                ]);

	return test_results;
}
